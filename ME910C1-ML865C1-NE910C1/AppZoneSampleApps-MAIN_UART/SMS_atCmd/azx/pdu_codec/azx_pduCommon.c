/*
 * azx_common.c
 *
 *  Created on: 17 set 2021
 *      Author: robertaga
 */

#include "azx_pduCommon.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>


/*
	Setting out_fmt variable, output can be represented in hex raw (if coding is 8-bit or UCS2) or utf8. Default is utf8.
	Variable can be set/read runtime using function pdu_set_output_format/get_set_output_format
 */

static SMS_TXT_OUT_FMT_E out_fmt = UTF8_default;
//SMS_TXT_OUT_FMT_E out_fmt = HEX_raw;

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

void *memset2(void *dest, int val, size_t len) {
  u8 *dest1 = (u8*) dest;
  while (len--) *dest1++ = (u8)val;
  return dest;
}

void *memcpy2(void *dest, const void *src, size_t len) {
  u8 *dest1 = (u8 *) dest;
  u8 *src1 = (u8 *)src;
  while (len--) *dest1++ = *src1++;
  return dest;
}

u8  divmod10_u32_rem;
u32 divmod10_u32(u32 n) {
  u32 quot = n >> 1;
  quot  += quot >> 1;
  quot  += quot >> 4;
  quot  += quot >> 8;
  quot  += quot >> 16;
  u32 qq = quot & ~7ul;
  quot >>= 3;
  divmod10_u32_rem = n - ((quot << 1) + qq);
  if (divmod10_u32_rem > 9) {
    divmod10_u32_rem -= 10;
    quot++;
  }
  return quot;
}

u8 asc_point = '.';
u8 asc_space = ' ';
u8 asc_minus = '-';

u8 bcd2bin_u8( u8 bcd ){ return ( (bcd & 0xF) + (bcd >> 4) * 10 ); }

u8 bin2bcd_u8( u8 bin ) {
  u8 tens = 0;
  while (bin > 9) { bin -= 10; tens++; }
  return ( (tens << 4) + bin );
}

#ifdef __AVR__
u32 bin2bcd_u32_avr(u32 data, u8 result_bytes) __attribute__((naked)) __attribute__((noinline));
u32 bin2bcd_u32_avr(u32 data, u8 result_bytes) {
  u32 result;
  asm volatile (
      "push __tmp_reg__ \n"
      "mov __tmp_reg__, %[result_bytes] \n" // __tmp_reg__ = number of bytes of result

      "bin2bcd_u32_00: \n"      // correct input number by left shifts
      "mov r31, %A[data] \n"
      "mov %A[data], %B[data] \n"
      "mov %B[data], %C[data] \n"
      "mov %C[data], %D[data] \n"
      "mov %D[data], r31 \n"
      "dec __tmp_reg__ \n"
      "brne bin2bcd_u32_00 \n"

      "eor r26, r26 \n"         // result=0
      "eor r27, r27 \n"
      "eor r30, r30 \n"
      "eor r31, r31 \n"
      "mov __tmp_reg__, %[result_bytes] \n"
      "lsl __tmp_reg__ \n"      // __tmp_reg__ = size in bits
      "lsl __tmp_reg__ \n"
      "lsl __tmp_reg__ \n"

      "bin2bcd_u32_01: \n"      // bits shift and correction loop
      "subi r26,-0x33 \n"   // add 0x33
      "sbrs r26, 3 \n"      // if carry to bit 3,
      "subi r26, 3 \n"      // subtract 3
      "sbrs r26, 7 \n"      // if carry to bit 7,
      "subi r26, 0x30 \n"   // subtract 0x30
      "subi r27,-0x33 \n"   // add 0x33
      "sbrs r27, 3 \n"      // if carry to bit 3,
      "subi r27, 3 \n"      // subtract 3
      "sbrs r27, 7 \n"      // if carry to bit 7,
      "subi r27, 0x30 \n"   // subtract 0x30
      "subi r30,-0x33 \n"   // add 0x33
      "sbrs r30, 3 \n"      // if carry to bit 3,
      "subi r30, 3 \n"      // subtract 3
      "sbrs r30, 7 \n"      // if carry to bit 7,
      "subi r30, 0x30 \n"   // subtract 0x30
      "subi r31,-0x33 \n"   // add 0x33
      "sbrs r31, 3 \n"      // if carry to bit 3,
      "subi r31, 3 \n"      // subtract 3
      "sbrs r31, 7 \n"      // if carry to bit 7,
      "subi r31, 0x30 \n"   // subtract 0x30
      "lsl r26 \n"          // shift result number
      "rol r27 \n"
      "rol r30 \n"
      "rol r31 \n"

      "sbrc %D[data], 7 \n" // skip if msbit of input == 0
      "ori  r26, 1 \n"      // set lsb of output
      "lsl %A[data] \n"     // shift input number
      "rol %B[data] \n"
      "rol %C[data] \n"
      "rol %D[data] \n"

      "dec __tmp_reg__ \n"
      "brne bin2bcd_u32_01 \n" // repeat for all bits

      //"mov %A[result], r26 \n"     // move to result
      //"mov %B[result], r27 \n"
      //"mov %C[result], r30 \n"
      //"mov %D[result], r31 \n"

      "mov r22, r26 \n"     // move to result
      "mov r23, r27 \n"
      "mov r24, r30 \n"
      "mov r25, r31 \n"

      "pop __tmp_reg__ \n"
      "ret \n"
      :[result]"=r"(result) // output
       :[data]"r"(data),[result_bytes]"r"(result_bytes) // input
        :"r26","r27","r30","r31" // clobbers
  );
  return result;
}
#endif

u32 bin2bcd_u32_shift(u32 data, u8 result_bytes) {
  u32 result = 0; /*result*/
  for (u8 cnt_bytes=(4 - result_bytes); cnt_bytes; cnt_bytes--) /* adjust input bytes */
    data <<= 8;
  for (u8 cnt_bits=(result_bytes << 3); cnt_bits; cnt_bits--) { /* bits shift loop */
    /*result BCD nibbles correction*/
    result += 0x33333333;
    /*result correction loop*/
    for (u8 cnt_bytes=4; cnt_bytes; cnt_bytes--) {
      u8 corr_byte = result >> 24;
      if (!(corr_byte & 0x08)) corr_byte -= 0x03;
      if (!(corr_byte & 0x80)) corr_byte -= 0x30;
      result <<= 8; /*shift result*/
      result += corr_byte; /*set 8 bits of result*/
    }
    /*shift next bit of input to result*/
    result <<= 1;
    if (((u8)(data >> 24)) & 0x80)
      result |= 1;
    data <<= 1;
  }
  return(result);
}

u32 bin2bcd_u32_hwdiv(u32 data, u8 result_bytes) {
  u32 result   = 0;
  result_bytes = 0;
  while (data > 0) {
    result += (data % 10) << result_bytes;
    data /= 10;
    result_bytes += 4;
  }
  return result;
}

u32 bin2bcd_u32_ldiv(u32 value, u8 nbytes) {
  static ldiv_t bin2bcd_ldiv_result;
  bin2bcd_ldiv_result.quot = value;
  u32 result = 0;
  nbytes = 0;
  while (bin2bcd_ldiv_result.quot > 0) {
    bin2bcd_ldiv_result = ldiv(bin2bcd_ldiv_result.quot, 10);
    result += bin2bcd_ldiv_result.rem << nbytes;
    nbytes += 4;
  }
  return result;
}

struct divmod10_t {
  uint32_t quot;
  uint8_t  rem;
} divmodu10_res;
inline static void divmodu10(u32 n) {
  divmodu10_res.quot = n >> 1;
  divmodu10_res.quot += divmodu10_res.quot >> 1;
  divmodu10_res.quot += divmodu10_res.quot >> 4;
  divmodu10_res.quot += divmodu10_res.quot >> 8;
  divmodu10_res.quot += divmodu10_res.quot >> 16;
  u32 qq = divmodu10_res.quot & ~7ul;
  divmodu10_res.quot >>= 3;
  divmodu10_res.rem = n - ((divmodu10_res.quot << 1) + qq);
  if (divmodu10_res.rem > 9) {
    divmodu10_res.rem -= 10;
    divmodu10_res.quot++;
  }
}
u32 bin2bcd_u32_fdiv(u32 value, u8 nbytes) {
  u32 result = 0; nbytes = 0;
  divmodu10_res.quot = value;
  while (divmodu10_res.quot != 0) {
    divmodu10(divmodu10_res.quot);
    result += divmodu10_res.rem << nbytes;
    nbytes += 4;
  }
  return result;
}

#ifdef __AVR__
u32 bcd2bin_u32_avr(u32 data, u8 result_bytes) __attribute__((naked)) __attribute__((noinline));
u32 bcd2bin_u32_avr(u32 data, u8 result_bytes) {
  u32 result;
  asm volatile (
      "push __tmp_reg__ \n"
      "eor r26, r26 \n"           /* result = 0 */
      "eor r27, r27 \n"
      "eor r30, r30 \n"
      "eor r31, r31 \n"
      "mov __tmp_reg__, %A[result_bytes] \n"   /* __tmp_reg__ = size in bits of input parameter  */
      "lsl __tmp_reg__ \n"
      "lsl __tmp_reg__ \n"
      "lsl __tmp_reg__ \n"

      "bcd2bin_u32_00: \n"        /* bits shift and correction loop */
      "lsr r31 \n"            /* shift out buffer */
      "ror r30 \n"
      "ror r27 \n"
      "ror r26 \n"

      "sbrc %A[data], 0 \n"        /* move lowest bit to result */
      "ori  r31, 0x80 \n"

      "lsr %D[data] \n"
      "ror %C[data] \n"
      "ror %B[data] \n"
      "ror %A[data] \n"

      "sbrc %D[data], 7 \n"        /* if carry to bit 7, */
      "subi %D[data], 0x30 \n"     /* subtract 0x30 */
      "sbrc %D[data], 3 \n"        /* if carry to bit 3, */
      "subi %D[data], 3 \n"        /* subtract 3 */
      "sbrc %C[data], 7 \n"        /* if carry to bit 7, */

      "subi %C[data], 0x30 \n"     /* subtract 0x30 */
      "sbrc %C[data], 3 \n"        /* if carry to bit 3, */
      "subi %C[data], 3 \n"        /* subtract 0x30 */
      "sbrc %B[data], 7 \n"        /* if carry to bit 7, */
      "subi %B[data], 0x30 \n"     /* subtract 0x30 */
      "sbrc %B[data], 3 \n"        /* if carry to bit 3, */
      "subi %B[data], 3 \n"        /* subtract 3 */
      "sbrc %A[data], 7 \n"        /* if carry to bit 7, */
      "subi %A[data], 0x30 \n"     /* subtract 0x30 */
      "sbrc %A[data], 3 \n"        /* if carry to bit 3, */
      "subi %A[data], 3 \n"        /* subtract 3 */

      "dec __tmp_reg__ \n"    /* repeat for all bits */
      "brne bcd2bin_u32_00 \n"

      //            "movw %A[result], r26 \n"          /* adjust result */
      //            "movw %C[result], r30 \n"
      "mov r22, r26 \n"     // move to result
      "mov r23, r27 \n"
      "mov r24, r30 \n"
      "mov r25, r31 \n"

      "bcd2bin_u32_01: \n"
      "mov __tmp_reg__,r25 \n"
      "mov r25,r24 \n"
      "mov r24,r23 \n"
      "mov r23,r22 \n"
      "mov r22,__tmp_reg__ \n"
      "dec %[result_bytes] \n"
      "brne bcd2bin_u32_01 \n"

      "pop __tmp_reg__ \n"
      "ret \n"
      :[result]"=r"(result):[data]"r"(data),[result_bytes]"r"(result_bytes):"r0","r26","r27","r30","r31" /*input and output parameters*/
  );
  return result;
}
#endif

u32 bcd2bin_u32_soft(u32 data, u8 input_bytes) {
  u32 result = 0; /*result*/
  for (u8 cnt_bits = (input_bytes << 3); cnt_bits; cnt_bits--) {
    /*shift next bit*/
    result >>= 1;
    if (((u8)(data)) & (u8)0x01) result |= 0x80000000;
    data >>= 1;
    /* result BCD correction */
    for (u8 cnt_bytes = 4; cnt_bytes; cnt_bytes--) {
      u8 tmp_byte = (data >> 24);
      if (tmp_byte & 0x80) tmp_byte -= 0x30;
      if (tmp_byte & 0x08) tmp_byte -= 0x03;
      data <<= 8;
      data |= tmp_byte;
    }
  }
  /*adjust result bytes*/
  for (u8 cnt_bits = (4 - input_bytes); cnt_bits; cnt_bits--)
    result >>= 8;
  return(result);
}

u8 asc2bin_u8(u8 data) {
  if ((data >= '0') && (data <= '9')) data -= '0';
  else if ((data >= 'a') && (data <= 'f')) data -= ('a' - 0xa);
  else if ((data >= 'A') && (data <= 'F')) data -= ('A' - 0xa);
  else data = 0xf0;
  return(data);
}

u8 bin2asc_u8(u8 data) {
  data &= 0xf;
  return (data > 9) ? (data + 'A' - 10) : (data + '0');
}

u32 asc2bin_u32(u8 *buf, s8 size) {
  u32 result;
  u8 flag_negative;
  if (size < 0) {
    flag_negative = 1;
    size = -size;
  } else
    flag_negative = 0;
  result = 0;
  while(size) {
    u8 tmp_u8 = asc2bin_u8(*buf);
    if (tmp_u8 <= 0xf) { /*skip non-HEX symbols*/
      result <<= 4;
      result |= tmp_u8;
    }
    size--;
    if (flag_negative) buf--;
    else buf++; /*go to next symbol*/
  }
  return(result);
}

void bin2asc_u32(u32 data, u8 *buf, s8 size) {
  u8 flag_negative;
  if (size < 0) {
    flag_negative = 1;
    size = -size;
  } else
    flag_negative = 0;
  while (size) {
    *buf = bin2asc_u8(data);
    data >>= 4;
    if (flag_negative) buf--;
    else buf++;
    size--;
    if (data == 0) size = 0; /*stop conversion on zero*/
  }
}

void num2asc_s32(s32 data, u8 *buf, u8 flags) {
  u8 cnt_chars = (flags & 7); //number of digits
  //convert negative to positive, set '-' flag
  if (data < 0) {data = -data; flags |= 1;}
  else flags &= ~1;
  //if necessary, convert to BCD
  //if (flags & 0x80) data = bin2bcd_u32_soft(data, (cnt_chars >> 1) + 1); //AlbertoLe: function not present
  cnt_chars++;
  //calculate point position
  u8 point_pos = cnt_chars - ((flags >> 4) & 7);
  if (point_pos) point_pos++;
  flags |= 2;
  while (cnt_chars) { //digits conversion loop
    if ((flags & 8) && (data == 0) && (cnt_chars < point_pos)) {
      if (flags & 1) *buf = asc_minus;
      else if (flags & 2) *buf = '0';
      else *buf = asc_space;
      flags &= ~1;
    } else {
      //show point
      if ((cnt_chars == point_pos) && (flags & 0x70)) {
        *buf-- = asc_point;
        cnt_chars--;
      }
      *buf = bin2asc_u8(data); //show ASCII digit
    }
    cnt_chars--;
    buf--;
    data >>= 4;
    flags &= ~2;
  }
  if (flags & 1) *buf = asc_minus;
}

uint8_t pdu_corr_sym_unpack_dcs7(char *Str, uint8_t len)
{
  uint8_t len_msg = 0;
  uint8_t i = 0;
  uint8_t check = 160;

  for(uint8_t j = 0; j < len; j++)
    if(Str[j] == 0x00)
      Str[j] = '@';

  for(uint8_t j = 0; j < len; j++)
    if(Str[j] == 0x11)
      Str[j] = '_';

  while((i < len) && (check--))
  {
    if(*Str == 0x1B)
    {
      // |
      if(*(uint16_t*)Str == 0x401B)
      {
        char BuffEnd[161];
        char *a = Str + 2;
        strcpy(&BuffEnd[0], a);
        *Str = '|';
        Str++;
        *Str = 0;
        strcat(Str, &BuffEnd[0]);
        i+=2;
        len_msg++;
      }

      // "\"
      if(*(uint16_t*)Str == 0x2F1B)
      {
        char BuffEnd[161];
        char *a = Str + 2;
        strcpy(&BuffEnd[0], a);
        *Str = '\\';
        Str++;
        *Str = 0;
        strcat(Str, &BuffEnd[0]);
        i+=2;
        len_msg++;
      }

      // ^
      if(*(uint16_t*)Str == 0x141B)
      {
        char BuffEnd[161];
        char *a = Str + 2;
        strcpy(&BuffEnd[0], a);
        *Str = '^';
        Str++;
        *Str = 0;
        strcat(Str, &BuffEnd[0]);
        i+=2;
        len_msg++;
      }

      // [
      if(*(uint16_t*)Str == 0x3C1B)
      {
        char BuffEnd[161];
        char *a = Str + 2;
        strcpy(&BuffEnd[0], a);
        *Str = '[';
        Str++;
        *Str = 0;
        strcat(Str, &BuffEnd[0]);
        i+=2;
        len_msg++;
      }

      // ]
      if(*(uint16_t*)Str == 0x3E1B)
      {
        char BuffEnd[161];
        char *a = Str + 2;
        strcpy(&BuffEnd[0], a);
        *Str = ']';
        Str++;
        *Str = 0;
        strcat(Str, &BuffEnd[0]);
        i+=2;
        len_msg++;
      }

      // {
      if(*(uint16_t*)Str == 0x281B)
      {
        char BuffEnd[161];
        char *a = Str + 2;
        strcpy(&BuffEnd[0], a);
        *Str = '{';
        Str++;
        *Str = 0;
        strcat(Str, &BuffEnd[0]);
        i+=2;
        len_msg++;
      }

      // }
      if(*(uint16_t*)Str == 0x291B)
      {
        char BuffEnd[161];
        char *a = Str + 2;
        strcpy(&BuffEnd[0], a);
        *Str = '}';
        Str++;
        *Str = 0;
        strcat(Str, &BuffEnd[0]);
        i+=2;
        len_msg++;
      }

      // ~
      if(*(uint16_t*)Str == 0x3D1B)
      {
        char BuffEnd[161];
        char *a = Str + 2;
        strcpy(&BuffEnd[0], a);
        *Str = '~';
        Str++;
        *Str = 0;
        strcat(Str, &BuffEnd[0]);
        i+=2;
        len_msg++;
      }

      // euro
      if(*(uint16_t*)Str == 0x651B)
      {
        char BuffEnd[161];
        char *a = Str + 2;
        strcpy(&BuffEnd[0], a);
        *Str = '?';
        Str++;
        *Str = 0;
        strcat(Str, &BuffEnd[0]);
        i+=2;
        len_msg++;
      }
    }
    else
    {
      i++;
      len_msg++;
      Str++;
    }
  }

  return len_msg;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t pdu_7to8(uint8_t *a, uint8_t len)
{
  //'123456789abc' -> 31 D9 8C 56 B3 DD 70 B9 B0 78 OC
  //'hellohello'   -> E8 32 9B FD 46 97 D9 EC 37
  //'Hello!'       -> C8 32 9B FD 0E 01
  if (len == 0)
    len = s_len(a,0);

  a[len] = 0;
  len++;

  // Correct symbol @
  for(uint8_t j = 0; j < len; j++)
    if(a[j] == '@')
      a[j] = 0x00;

  // Correct symbol _
  for(uint8_t j = 0; j < len; j++)
    if(a[j] == '_')
      a[j] = 0x11;

  if (len > 0)
  {
    for (uint8_t i1=0; i1<(len-2); i1++)
    {
      for (uint8_t i2=(len-1); i2>i1; i2--)
      {
        a[i2-1] = a[i2-1] | ((a[i2] & 1) ? 0x80 : 0);
        a[i2] >>= 1;
      }
    }
  }

  len--;

  return len - (len >> 3);
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t pdu_8to7(uint8_t *a, uint8_t len)
{
  uint8_t len1 = len + (len >> 3);

  while (len < len1)
    a[len++] = 0;

  if (len > 0)
  {
    for (uint8_t i1=0; i1<(len1-1); i1++)
      for (uint8_t i2=(len1-1); i2>i1; i2--)
        a[i2] = (a[i2] << 1) | !!(a[i2-1] & 0x80);
  }

  for (uint8_t i=0; i<len1; i++)
    a[i] &= 0x7F;

  return len1;
}

//u16 pdu_utf8_to_ucs2(u8 *in, u16 in_len, u8 *out){
//	u16 out_len = 0;
//	while (in_len){
//		u8 n = utf8_to_ucs2(in, out);
//		if (n == 0) in_len = 0;
//		else {
//			in += n;
//			out++; out_len++;
//			in_len = (in_len >= n) ? (in_len - n) : 0;
//		}
//	}
//	return out_len;
//}


/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t  swap8 (uint8_t b)
{
  return (b << 4)|(b >> 4);
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint16_t swap16(uint16_t w)
{
  return (w << 8)|(w >> 8);
}
#include <stdio.h>
#define M2M_LOG_INFO(a...) ({ printf(a); fflush(stdout); })
/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
void pdu_hex2bytes(uint8_t *h, uint16_t len, uint8_t *b)
{
  if (len == 0)
    return;

  len >>= 1;

  while (len--)
  {
    *b++ = (s_h2b4(*h) << 4) | s_h2b4(*(h+1));
    h += 2;
  }
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
void pdu_bytes2hex(uint8_t *b, uint16_t len, uint8_t *h)
{
  if (len == 0)
    return;

  b += len-1; h += ((uint16_t)len << 1) - 1;
  while (len--)
  {
    *h-- = s_b2h4(*b & 0xF);
    *h-- = s_b2h4(*b-- >> 4);
  }
}


uint8_t convert_binary_coded_to_decimal(uint8_t hex)
{
  uint8_t dec = ((hex & 0xf0) >> 4) * 10 + (hex & 0x0f);
  return dec;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
/*DCS decoding function*/
dcs_struct pdu_decode_dcs(uint8_t tp_dcs)
{
  dcs_struct sm_dcs;
  uint8_t alpha_byte, class_byte;

  if ( ( (tp_dcs & 0xc0) == 0)) /* coding group: 00xx */
  {
    if ( tp_dcs & 0x20 )
      sm_dcs.smCompressed = 1;           /* compressed */
    else
      sm_dcs.smCompressed = 0;


    alpha_byte = ( tp_dcs & 0x0c ) >> 2;   /* alphabet */
    if ( alpha_byte == 0 )
      sm_dcs.smAlphabet = PDU_DCS_7;
    else if ( alpha_byte == 1 )
      sm_dcs.smAlphabet = PDU_DCS_8;
    else if ( alpha_byte == 2 )
      sm_dcs.smAlphabet = PDU_DCS_UCS2;
    else
      sm_dcs.smAlphabet = PDU_DCS_AUTO;

    if ( tp_dcs & 0x10 )  /* bit 1 and 0 have class meaning */
    {
      class_byte = ( tp_dcs & 0x03 );
      if ( class_byte == 0 )
        sm_dcs.smClass = class0;
      else if ( class_byte == 1 )
        sm_dcs.smClass = class1;
      else if ( class_byte == 2 )
        sm_dcs.smClass = class2;
      else /* if ( class_byte == 3 ) */
        sm_dcs.smClass = class3;
    }
    else
      sm_dcs.smClass = noClass;

  } else if ( (tp_dcs >> 4) == 0x0f ){                /* coding group: 1111 */
    sm_dcs.smCompressed = 0;

    if ( tp_dcs & 0x04 )
      sm_dcs.smAlphabet = PDU_DCS_8;
    else
      sm_dcs.smAlphabet = PDU_DCS_7;

    class_byte = ( tp_dcs & 0x03 );
    if ( class_byte == 0 )
      sm_dcs.smClass = class0;
    else if ( class_byte == 1 )
      sm_dcs.smClass = class1;
    else if ( class_byte == 2 )
      sm_dcs.smClass = class2;
    else /* if ( class_byte == 3 ) */
      sm_dcs.smClass = class3;
  }	else  {/* reserved codings groups: assume 7 bit, uncompressed, class 2 */

    sm_dcs.smCompressed = 0;
    sm_dcs.smAlphabet = PDU_DCS_7;
    sm_dcs.smClass = class2;
  }
  return sm_dcs;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
void pdu_in_decode_binary(uint8_t *in, uint16_t len, pdu_struct *out)
{
  int8_t tz_sign = 1;
  dcs_struct sm_dcs;  /*Added dcs decoding in alphabet, class and compressed*/

  if (len == 0)
    len = s_len(in, 0);

  len >>= 1;

  out->smsc.len = in[0];
  out->smsc.bytes = out->smsc.len - 1;

  if (in[0] == 7)
  {
    out->smsc.type = in[1];
    for (uint8_t i=0; i<(out->smsc.bytes); i++)
      out->smsc.data[i] = in[i+2];
  }

  in += out->smsc.len + 1;

  out->first        = *in++;
  out->sender.len   = *in++;
  out->sender.bytes = (out->sender.len + (out->sender.len & 1)) >> 1;
  out->sender.type  = *in++;
  for (uint8_t i=0; i<((out->sender.len + (out->sender.len & 1)) >> 1); i++)
    out->sender.data[i] = *in++;

  out->tp_pid       = *in++;
  out->tp_dcs       = *in++;

  out->year         = convert_binary_coded_to_decimal( swap8(  *in++ ));
  out->month        = convert_binary_coded_to_decimal( swap8(  *in++ ));
  out->date         = convert_binary_coded_to_decimal( swap8(  *in++ ));
  out->hour         = convert_binary_coded_to_decimal( swap8(  *in++ ));
  out->min          = convert_binary_coded_to_decimal( swap8(  *in++ ));
  out->sec          = convert_binary_coded_to_decimal( swap8(  *in++ ));
  //*in = 0x4a;
  out->tz           = swap8( *in++ );

  if ((out->tz & 0x80) == 0x80)
  {
    tz_sign = -1;
  }

  out->tz = tz_sign * convert_binary_coded_to_decimal((out->tz & 0x7F));

  out->msg.len = out->msg.bytes = *in++;

  // correction for 7-bit encoding

  /*Now dcs has been correctly decoded*/
  sm_dcs = pdu_decode_dcs(out->tp_dcs);
  out->tp_dcs = sm_dcs.smAlphabet;
  /**/
  if (out->tp_dcs == PDU_DCS_7)
    out->msg.bytes -= (out->msg.bytes >> 3);
  // decode message text (if output buffer is given)
  out->msg.data = in;
}


/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
void pdu_in_decode(uint8_t *in, uint16_t len, pdu_struct *out)
{
  pdu_hex2bytes(in, len, in);
  pdu_in_decode_binary(in, len, out);
}


/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
#if 1
uint16_t pdu_in_decode_text(uint8_t *in, uint16_t in_bytes, uint8_t in_dcs, uint8_t *out)
{
  if (in_dcs == PDU_DCS_UCS2)
  {
    if (pdu_get_output_format() == UTF8_default) {
      in_bytes = s_ucs2_to_utf8( (uint16_t *)in, in_bytes >> 1, out, 1 );
      out += in_bytes;
    } else if(pdu_get_output_format() == HEX_raw) {
      uint16_t i, j;
      uint8_t tmpByte;

      j = 0;

      for ( i = 0; i < in_bytes; i++ )
      {

        tmpByte = ( in[i] & 0x00FF );
        out[j++] = pdu_hex_to_ascii_format( ( tmpByte & 0xF0 ) >> 4 );
        out[j++] = pdu_hex_to_ascii_format( ( tmpByte & 0x0F )  );

      }

      in_bytes = j;
      out += in_bytes;
    }

  }
  else
  {
    if (in_dcs == PDU_DCS_7)
    {
      in_bytes += in_bytes >> 3;
      pdu_8to7(in, in_bytes);
      for (uint8_t i=0; i<in_bytes; i++)
        *out++ = *in++;
    } else if (in_dcs == PDU_DCS_8) {

      if (pdu_get_output_format() == UTF8_default)
      {
        for (uint8_t i=0; i<in_bytes; i++)
          *out++ = *in++;
      } else if(pdu_get_output_format() == HEX_raw){
        uint16_t i, j;
        uint8_t tmpByte;

        j = 0;

        for ( i = 0; i < in_bytes; i++ )
        {
          tmpByte = ( in[i] & 0x00FF );
          out[j++] = pdu_hex_to_ascii_format( ( tmpByte & 0xF0 ) >> 4 );
          out[j++] = pdu_hex_to_ascii_format( ( tmpByte & 0x0F )  );
        }
        //out[j] = '\0';
        in_bytes = j;
        out += in_bytes;
      } else {
        in_bytes = s_ucs2_to_utf8( (uint16_t *)in, in_bytes >> 1, out, 1 );
        out += in_bytes;
      }
    }
  }

  *out = 0;

  return in_bytes;
}


#else //original
uint16_t pdu_in_decode_text(uint8_t *in, uint16_t in_bytes, uint8_t in_dcs, uint8_t *out)
{
  if (in_dcs == PDU_DCS_UCS2)
  {

    in_bytes = s_ucs2_to_utf8( (uint16_t *)in, in_bytes >> 1, out, 1 );
    out += in_bytes;

  }
  else
  {
    if (in_dcs == PDU_DCS_7)
    {
      in_bytes += in_bytes >> 3;
      pdu_8to7(in, in_bytes);
    }

    for (uint8_t i=0; i<in_bytes; i++)
      *out++ = *in++;
  }

  *out = 0;

  return in_bytes;
}
#endif
/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint16_t pdu_out_encode(pdu_struct *in, uint8_t *out)
{
  uint16_t len = 1;
  *out++ = in->smsc.len;
  // fill SMSC data
  if (in->smsc.len)
  {
    *out++ = in->smsc.type;
    for (uint8_t i=0; i<(in->smsc.len-1); i++)
      *out++ = in->smsc.data[i];
    len = in->smsc.len;
  }

  *out++ = in->first;       // 1st octet (0x11)
  *out++ = in->tp_msg_ref;  // TP-Message-Reference. The "00" value here lets the phone set the message  reference number itself

  in->sender.bytes = pdu_phone_bytes(in->sender.data);
  in->sender.len   = pdu_phone_digits(in->sender.data, in->sender.bytes);
  *out++ = in->sender.len;  // address length, digits
  *out++ = in->sender.type; // type-of-address
  for (uint8_t i=0; i<in->sender.bytes; i++)
    *out++ = in->sender.data[i];

  *out++ = in->tp_pid;      // TP-PID. Protocol identifier

  if (in->tp_dcs == PDU_DCS_AUTO)
    in->tp_dcs = s_is_7bit(in->msg.data, 0) ? PDU_DCS_7 : PDU_DCS_UCS2;

  *out++ = in->tp_dcs;      // TP-DCS. Data coding scheme

  *out++ = in->tp_vp;       // TP-Validity-Period. "AA" means 4 days
  len += in->sender.bytes + 8;

  // add message
  in->msg.len = s_len(in->msg.data,0);

  if (in->tp_dcs == PDU_DCS_UCS2)// UCS2 ? Convert from UTF8
  {
    in->msg.bytes = in->msg.len = (s_utf8_to_ucs2(in->msg.data, in->msg.len, (uint16_t *)(out+1), 1) << 1);
    *out++ = in->msg.len;
  }
  else
  {
    *out++ = in->msg.len;
    s_copy(in->msg.data, 0, out);
    if (in->tp_dcs == PDU_DCS_7)
      in->msg.bytes = pdu_7to8(out, in->msg.len);
    else
      in->msg.bytes = in->msg.len;
  }

  out -= len;
  len += in->msg.bytes;
  pdu_bytes2hex(out, len, out);
  in->len_bytes      = len << 1;
  in->len_cmgs       = len - in->smsc.len - 1;
  out[in->len_bytes] = 0;

  return in->len_bytes;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint16_t pdu_out_encode_simple(pdu_struct *pdu, uint8_t *out, void *sender, void *msg, uint8_t tp_vp,int dcs)
{
  pdu->first       = 0x11;
  pdu->tp_msg_ref  = 0;
  pdu->tp_pid      = 0;
  pdu->tp_vp       = (tp_vp == 0) ? 0xAA : tp_vp;

  pdu->smsc.len    = 0;

  pdu->sender.type = PDU_TYPE_INTERNATIONAL;


  if (pdu_phone_is_packed((uint8_t *)sender)) {
    pdu_phone_copy((uint8_t *)sender, pdu->sender.data);
  }
  else{
    pdu_phone_pack((uint8_t *)sender, pdu->sender.data);
  }


  //pdu->tp_dcs      = PDU_DCS_AUTO;
  //pdu->tp_dcs      = PDU_DCS_8;
  pdu->tp_dcs      = dcs;
  pdu->msg.data    = (uint8_t *)msg;

  pdu_out_encode(pdu, out);

  return pdu->len_bytes;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t pdu_phone_pack(uint8_t *in, uint8_t *out)
{
  uint8_t n = 0;

  if (*in == '+')
    in++;

  while ((*in >= '0')&&(*in <= '9'))
  {
    *out = s_h2b4(*in++);
    if (*in)
    {
      *out |= s_h2b4(*in) << 4;
      in++;
      n++;
    }
    else
      *out |= 0xF0;

    n++;
    out++;
  }

  *out = 0xFF;

  return n;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t pdu_phone_unpack(uint8_t *in, uint8_t len, uint8_t *out, uint8_t type)
{
  uint8_t n = 0;

  if (type == PDU_TYPE_INTERNATIONAL)
    *out++ = '+';

  while (len--)
  {
    *out++ = s_b2h4(*in & 0xF);
    n++;

    if ((*in & 0xF0) != 0xF0)
    {
      *out++ = s_b2h4(*in >> 4);
      n++;
    }

    in++;
  }

  *out = 0;

  return n;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t pdu_phone_is_packed(uint8_t *phone)
{
  return *phone != '+';
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t pdu_phone_digits_bytes(uint8_t digits)
{
  return (digits + (digits & 1)) >> 1;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t pdu_phone_bytes(uint8_t *in)
{
  uint8_t len = 0;

  while (*in++ != 0xFF)
    len++;

  return len;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t pdu_phone_digits(uint8_t *in, uint8_t bytes)
{
  uint8_t len = bytes << 1;

  if ((in[bytes-1] & 0xF0) == 0xF0)
    len--;

  return len;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
uint8_t pdu_phone_cmp(uint8_t *in1, uint8_t in1_bytes, uint8_t *in2, uint8_t in2_bytes)
{
  if ((in1_bytes != in2_bytes)||(in1_bytes == 0))
    return 0;

  while (in1_bytes--) if (*in1++ != *in2++)
    return 0;

  return 1;
}

/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
void pdu_phone_copy(uint8_t *in, uint8_t *out)
{
  if (in == out)
    return;

  uint8_t eop = pdu_phone_is_packed(in) ? 0xFF : 0;

  while (*in != eop)
    *out++ = *in++;

  *out = eop;
}

//Functions to convert SMS text in raw hex output (as specified by 3GPP TS 27.005)
void pdu_set_output_format(SMS_TXT_OUT_FMT_E output_format)
{
  out_fmt = output_format;
}

SMS_TXT_OUT_FMT_E pdu_get_output_format(void)
{
  return out_fmt;
}

static const uint8_t ha_form[] =
{
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'
};

uint8_t pdu_hex_to_ascii_format (uint8_t LV_nmb)
{
  /* convert it into ASCII code */
  return (ha_form[(LV_nmb & 0x0F)]);
}

