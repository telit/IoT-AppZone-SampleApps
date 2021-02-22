/*
 * ul_s.h
 *
 *  Created on: Sep 21, 2014
 *      Author: qwer
 */

#ifndef UL_S_H_
#define UL_S_H_

#include "ul_other.h"


// convert UTF8 to UCS2 codepoint, returns number of bytes processed, be=1 for big-endian
u8 utf8_to_ucs2(u8 *in, u16 *out, u8 be);
// convert string in UTF8 to UCS2, returns number of UCS code points, be=1 for big-endian
u16 s_utf8_to_ucs2(u8 *in, u16 in_len, u16 *out, u8 be);

// convert UCS2 codepoint to UTF8, returns number of bytes processed
u8 ucs2_to_utf8(u32 in, u8 *out);
// convert string of UCS2 codepoints to UTF8, returns number of UTF8 bytes, be=1 for big-endian
u16 s_ucs2_to_utf8(u16 *in, u16 in_len, u8 *out, u8 be);


// find length of line, terminated by eol char
u16 s_len(u8 *s, u8 eol);

// number of lines, separated by eol char
u8 s_lines(u8 *s, u16 len, u8 eol);

u8 c_upcase(u8 c);
// uppercase string, in-place
void s_upcase(u8 *s, u16 len);

u8 s_h2b4(u8 hex);
u8 s_b2h4(u8 b);

// find start of line, delimited by eol char
u8 *s_line_start(u8 *s, u16 len, u8 line, u8 eol);

// skip CR/LF chars
u8 *s_skip_crlf(u8 *s);

// compare 2 strings, returns 1 if equal
u8 s_cmp(u8 *s1, u8 *s2, u16 len);
// compare 2 strings, upper-casing each char, returns 1 if equal
u8 s_cmp_upcase(u8 *s1, u8 *s2, u16 len);

// check if s starts with start
u8 s_starts(u8 *s, char *start);
// check if s ends with end
u8 s_ends(u8 *s, char *end);

// finds 1st occurence of char c, returns NULL if not found
u8 *s_cpos(u8 *s, u8 c, u8 eol);

// check if string is 7-bit
u8 s_is_7bit(u8 *s, u8 eol);

// copy in string (terminated by eol char) to out
u8 *s_copy(u8 *in, u8 eol, u8 *out);


// append string out to in, returns pointer after last char
u8 *s_append_s  (u8 *out, char *in);
// append char to string buffer
u8 *s_append_c  (u8 *out, u8 c);
// append Hex char to string buffer
u8 *s_append_h4 (u8 *out, u8 n);
// append Hex byte to string buffer
u8 *s_append_h8 (u8 *out, u8 n);
// append 16-bit Hex to string buffer
u8 *s_append_h16(u8 *out, u16 n);
// append 24-bit Hex to string buffer
u8 *s_append_h24(u8 *out, u32 n);
// append 32-bit Hex to string buffer
u8 *s_append_h32(u8 *out, u32 n);


// parse decimal number
u32 s_parse_dec(u8 *s, u8 len);
// parse hex number
u32 s_parse_hex(u8 *s, u8 len);

#endif /* UL_S_H_ */
