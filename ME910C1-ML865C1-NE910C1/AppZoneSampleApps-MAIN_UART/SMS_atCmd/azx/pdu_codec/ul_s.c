/*
 * ul_s.c
 *
 *  Created on: Sep 21, 2014
 *      Author: qwer
 */

#include "ul_s.h"
#include "ul_math_bcd.h"

u8 utf8_to_ucs2(u8 *in, u16 *out, u8 be){
	u8 n = 0; u16 o;
    if (in[0] < 0x80){ n = 1; o = in[0]; }
    else if ((in[0] & 0xE0) == 0xE0){
        if ((in[1] != 0) && (in[2] != 0)){
        	n = 3;
        	o = ((u16)(in[0] & 0x0F) << 12) | ((u16)(in[1] & 0x3F) << 6) | (in[2] & 0x3F);
        }
    } else if ((in[0] & 0xC0) == 0xC0){
        if (in[1] != 0){
        	n = 2;
        	o = ((u16)(in[0] & 0x1F) << 6) | ((u16)(in[1] & 0x3F));
        }
    }
    if (n){
    	if (be){
    		*((u8 *)out)     = o >> 8;
    		*((u8 *)out + 1) = o;
    	} else {
    		*((u8 *)out)     = o;
    		*((u8 *)out + 1) = o >> 8;
    	}
    }
    return n;
}
u16 s_utf8_to_ucs2(u8 *in, u16 in_len, u16 *out, u8 be){
	if (in_len == 0) in_len = s_len(in,0);
	u16 out_len = 0;
	while (in_len){
		u8 n = utf8_to_ucs2(in, out, be);
		if (n == 0) in_len = 0;
		else {
			in += n;
			out++; out_len++;
			in_len = (in_len >= n) ? (in_len - n) : 0;
		}
	}
	return out_len;
}


u8 ucs2_to_utf8(u32 in, u8 *out){
	u8 n = 0;
    if (in < 0x80){
		out[0] = in;
		out[1] = 0;
		n = 1;
    }
    else if ((in >= 0x80) && (in < 0x800)){
		out[0] = (in >> 6) | 0xC0;
		out[1] = (in & 0x3F) | 0x80;
		out[2] = 0;
		n = 2;
    }
    else if ((in >= 0x800) && (in < 0xFFFF)){
		if (!((in >= 0xD800) && (in <= 0xDFFF))){
		    out[0] = (in >> 12) | 0xE0;
		    out[1] = ((in >> 6 ) & 0x3F) | 0x80;
		    out[2] = (in & 0x3F) | 0x80;
		    out[3] = 0;
		    n = 3;
		}
    }
    else if ((in >= 0x10000) && (in < 0x10FFFF)){
		out[0] = 0xF0 | (in >> 18);
		out[1] = 0x80 | ((in >> 12) & 0x3F);
		out[2] = 0x80 | ((in >> 6) & 0x3F);
		out[3] = 0x80 | ((in & 0x3F));
		out[4] = 0;
		return 4;
    }
    return n;
}
u16 s_ucs2_to_utf8(u16 *in, u16 in_len, u8 *out, u8 be){
	u16 out_len = 0;
	while (in_len){
		u8 n = ucs2_to_utf8(
			(be) ? ((u16)(*(u8 *)in) << 8) | *(((u8 *)in) + 1) : ((u16)(*(u8 *)in)) | ((u16)(*(((u8 *)in) + 1)) << 8), out);
		if (n == 0) in_len = 0;
		else {
			in++;     in_len--;
			out += n; out_len += n;
		}
	}
	return out_len;
}

u16 s_len(u8 *s, u8 eol){
	if (s == NULL) return 0;
	u16 l = 0;
	while (*s++ != eol) l++;
	return l;
}
u8 s_lines(u8 *s, u16 len, u8 eol){
	if (len == 0) len = s_len(s, 0);
	u8 n = 0;
	while (len--) if (*s++ == eol) n++;
	return n;
}
u8 *s_line_start(u8 *s, u16 len, u8 line, u8 eol){
	if (len == 0) len = s_len(s, 0);
	u8 l = 0;
	while (len-- && (l < line))
		if (*s++ == eol) l++;
	return (len == 0) ? NULL : s;
}
u8 *s_skip_crlf(u8 *s){
	while ((*s == '\r')||(*s == '\n')) s++;
	return s;
}

u8 c_upcase(u8 c){ return ((c >= 'a')&&(c <= 'z')) ? (c - ('a' - 'A')) : c; }
void s_upcase(u8 *s, u16 len){
	if (len == 0) len = s_len(s, 0);
	while (len--){
		*s = c_upcase(*s);
		s++;
	}
}

u8 s_h2b4(u8 hex){ return ((hex >= '0')&&(hex <= '9')) ? (hex - '0') : ((hex >= 'a')&&(hex <= 'f')) ? (hex - ('a' - 10)) : ((hex >= 'A')&&(hex <= 'F')) ? (hex - ('A' - 10)) : 0; }
u8 s_b2h4(u8 b){ return (b < 10) ? (b + '0') : (b < 16) ? (b + ('A'-10)) : '0'; }

u8 s_cmp(u8 *s1, u8 *s2, u16 len){
	while (len--) if (*s1++ != *s2++) return 0;
	return 1;
}
u8 s_cmp_upcase(u8 *s1, u8 *s2, u16 len){
	while (len--) if (c_upcase(*s1++) != c_upcase(*s2++)) return 0;
	return 1;
}

u8 s_starts(u8 *s, char *start){
	while (*start != 0) if (*s++ != *start++) return 0;
	return 1;
}
u8 s_ends(u8 *s, char *end){
	u16 sl = s_len(s,0), el = s_len((u8 *)end,0);
	if ((el > 0)&&(sl >= el)){
		s   += sl - 1; end += el - 1;
		while (el--) if (*s-- != *end--) return 0;
		return 1;
	}
	return 0;
}

u8 *s_cpos(u8 *s, u8 c, u8 eol){
	while(*s != eol){
		if (*s == c) return s;
		s++;
	}
	return NULL;
}

u32 s_parse_dec(u8 *s, u8 len){
	u32 n = 0;
	if (len == 0) len = 255;
	while ((*s >= '0')&&(*s <= '9')&&(len--)) n = (n * 10uL) + s_h2b4( *s++ );
	return n;
}
u32 s_parse_hex(u8 *s, u8 len){
	u32 n = 0;
	if (len == 0) len = 255;
	while ((((*s >= '0')&&(*s <= '9'))||((*s >= 'a')&&(*s <= 'f'))||((*s >= 'A')&&(*s <= 'F')))&&(len--))
		n = (n << 4) + s_h2b4( *s++ );
	return n;
}

u8 s_is_7bit(u8 *s, u8 eol){
	while (*s != eol) if (*s++ > 0x7F) return 0;
	return 1;
}

u8 *s_copy(u8 *in, u8 eol, u8 *out){
	if (in == out) return out;
	while (*in != eol) *out++ = *in++;
	*out = eol;
	return out;
}

u8 *s_append_c  (u8 *out, u8 c)  { *out++ = c; *out = 0; return out; }
u8 *s_append_s  (u8 *out, char *in){ while (*in != 0) *out++ = *in++; *out = 0; return out; }
u8 *s_append_h4 (u8 *out, u8 n)  { *out++ = s_b2h4(n & 0xF); *out = 0; return out; }
u8 *s_append_h8 (u8 *out, u8 n)  { out = s_append_h4(out,n >> 4);   out = s_append_h4(out,n);  return out; }
u8 *s_append_h16(u8 *out, u16 n) { out = s_append_h8(out,n >> 8);   out = s_append_h8(out,n);  return out; }
u8 *s_append_h24(u8 *out, u32 n) { out = s_append_h8(out,n >> 16);  out = s_append_h16(out,n); return out; }
u8 *s_append_h32(u8 *out, u32 n) { out = s_append_h16(out,n >> 16); out = s_append_h16(out,n); return out; }

#if 0
u8 *s_append_u32(u8 *out, u32 n) {
	if (n){
		n = bin2bcd_u32(n, 4); u8 digs = 8;
		while(((u8)(n >> 24) & 0xF0) == 0) { n <<= 4; digs--; }
		while(digs--){
			*out++ = s_b2h4(n >> 28);
			n <<= 4;
		}
	} else *out++ = '0';
	*out = 0;
	return out;
}


u8 *s_append_s32(u8 *out, s32 n) {
	if (n < 0){
		out = s_append_c(out, '-');
		n = -n;
	}
	return s_append_u32(out, n);
}
#endif
