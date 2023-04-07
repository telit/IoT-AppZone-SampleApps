/*
 * azx_common.h
 *
 *  Created on: 17 set 2021
 *      Author: robertaga
 */

#ifndef AZX_PDU_CODEC_AZX_PDUCOMMON_H_
#define AZX_PDU_CODEC_AZX_PDUCOMMON_H_

#include <stdint.h>
#include <stddef.h>

#ifndef offsetof
#define offsetof(s,m) (size_t)&(((s *)0)->m)
#endif
#ifndef countof
#define countof(arg) (sizeof(arg) / sizeof(arg[0]))
#endif

//attributes
#define WEAK            __attribute__ ((weak))
#define NAKED           __attribute__ ((naked))
#define INTERRUPT       __attribute__ ((__interrupt__))
#define USED            __attribute__ ((used))
#define NORETURN        __attribute__ ((noreturn))
#define SECTION(s)      __attribute__ ((section(s)))
#define NOINIT          SECTION(".noinit")
#define ALIAS(a)        __attribute__ ((weak, alias (a)))
#define ALWAYS_INLINE   __attribute__ ((always_inline))
#define OPTIMIZE(level) __attribute__ ((optimize(level)))

//architecture-specific inlines
#ifdef __AVR
#define wdr()   asm volatile("wdr")
#define nop()   asm volatile("nop")
#define sleep() asm volatile("sleep")
#else
#define PROGMEM
#endif

#ifdef __i8051
#define cli()  EI = 0
#define sei()  EI = 1
#endif

#ifdef __STM8
#define cli()  _asm("sim")
#define sei()  _asm("rim")
#define nop()  _asm("nop")
#define wdr()  { IWDG_KR = 0xAA; }
#endif

#if defined( __ARM_ARCH_7M__ ) || defined( __ARM_ARCH_6M__ )
#define cli() asm volatile ( "CPSID i \n" )
#define sei() asm volatile ( "CPSIE i \n" )
#endif

//short named types
typedef uint8_t  u8;
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef uint64_t u64;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64;
#ifdef __float128
typedef __float128 f128;
#endif

//bit mask
#define B(_b)   (1 << (_b))
#define BIT(_b) (1 << (_b))

//Increment/decrement, Add/Subtract with range rollover/limit
#define INC_ROLL(a, min, max) { if ((a) >= (max)) (a) = (min); else (a)++; }
#define INC_LIMIT(a, max)     { if ((a) < (max)) (a)++; }
#define ADD_LIMIT(a, b, max)  { (a) += (b); if ((a) > (max)) (a) = (max); }
#define DEC_ROLL(a, min, max) { if ((a) <= (min)) (a) = (max); else (a)--; }
#define DEC_LIMIT(a, min)     { if ((a) > (min)) (a)--; }
#define SUB_LIMIT(a, b, min)  { (a) -= (b); if ((a) < (min)) (a) = (min); }
#define IN_RANGE(a, min, max) (((min) <= (a))&&((a) <= (max)))
#define TO_UPPER(c) ((IN_RANGE(c,'a','z')) ? ((c) - ('a' - 'A')) : (c))
#define TO_LOWER(c) ((IN_RANGE(c,'A','Z')) ? ((c) + ('a' - 'A')) : (c))
#define TO_HEX(bbb) (((bbb) < 10) ? ((bbb) + '0') : ((bbb) + ('A' - 10)))
#define IS_ALPHA(c) (IN_RANGE(c,'a','z') || IN_RANGE(c,'A','Z'))
#define IS_DIGIT(c) (IN_RANGE(c,'0','9'))
#define MIN(x, y)   ((x) < (y) ? (x) : (y))
#define MAX(x, y)   ((x) > (y) ? (x) : (y))

#ifdef __ARM_ARCH_7M__
//ARM Cortex BitBand access macros
#define BITBAND_SRAM_REF   0x20000000
#define BITBAND_SRAM_BASE  0x22000000
// Convert SRAM address
#define BITBAND_SRAM(addr, bit) ((volatile u8 *)((BITBAND_SRAM_BASE + ((u32)(addr)-BITBAND_SRAM_REF)*32 + (bit*4))))
#define BBM( addr, bit ) (BITBAND_SRAM(addr,bit))

#define BITBAND_PERI_REF   0x40000000
#define BITBAND_PERI_BASE  0x42000000
// Convert PERIPHERAL address
#define BITBAND_PERI(addr, bit) ((volatile u8 *)((BITBAND_PERI_BASE + ((u32)(addr)-BITBAND_PERI_REF)*32 + (bit*4))))
#define BBP( addr, bit ) (BITBAND_PERI(addr,bit))
#endif

//memset - simple verion
void *memset2(void *dest, int val, size_t len);
//memcpy - simple verion
void *memcpy2(void *dest, const void *src, size_t len);

extern u8 divmod10_u32_rem;
u32 divmod10_u32(u32 n);


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

extern u8 asc_point;
extern u8 asc_space;
extern u8 asc_minus;

/** convert lower nibble to HEX symbol */
#define to_hex(bbb) (((bbb) & 0xF) > 9 ? ((bbb) & 0xF) + ('a' - 10) : ((bbb) & 0xF) + '0')

/** 8-bit BCD->BIN conversion */
u8 bcd2bin_u8( u8 bcd );
/** 8-bit BIN->BCD conversion */
u8 bin2bcd_u8( u8 bin );

// 32-bit BIN->BCD conversion using shifts and correction
//u32 bin2bcd_u32_soft(u32 data, u8 result_bytes); AlbertoLe: function never called by our test, implementation not present, commented

// 32-bit BIN->BCD conversion using hardware divider
u32 bin2bcd_u32_hwdiv(u32 data, u8 result_bytes);

// 32-bit BIN->BCD conversion using ldiv
u32 bin2bcd_u32_ldiv(u32 data, u8 result_bytes);

// 32-bit BIN->BCD conversion using fast soft div
u32 bin2bcd_u32_fdiv(u32 value, u8 nbytes);

// 32-bit BCD->BIN conversion
u32 bcd2bin_u32_soft(u32 data, u8 input_bytes);

u8 asc2bin_u8(u8 data);
u8 bin2asc_u8(u8 data);
u32 asc2bin_u32(u8 *buf, s8 size);
void bin2asc_u32(u32 data, u8 *buf, s8 size);

// AVR-optimized conversions
#ifdef __AVR__
u32 bin2bcd_u32_avr(u32 data, u8 result_bytes);
u32 bcd2bin_u32_avr(u32 data, u8 input_bytes);
#endif

/** convert 32-bit signed data to ASCII number to buffer (right to left)
 * options & 7 - width-1,
 * (options >> 4) & 7 - point position,
 * options & 0x08 - supress leading zeros,
 * options & 0x80 - convert to BCD.
 */
#define NUM2ASC_BCD     (u8)0x80
#define NUM2ASC_NOZEROS (u8)0x08
#define NUM2ASC_FLAGS( w, p, bcd, nozeros) (u8)( (((u8)((w)-1))&7) + ((u8)((p)<<4)) + ((bcd) * NUM2ASC_BCD) + ((nozeros) * NUM2ASC_NOZEROS) )
#define NUM2ASC_s32( data, buf, w, p, bcd, nozeros ) num2asc_s32( data, buf, NUM2ASC_FLAGS( (w), (p), (bcd), (nozeros) ))
#define NUM2ASC_WIDTH( flags ) ((flags & 7) + 1)
void    num2asc_s32(s32 data, u8 *buf, u8 flags);
inline void num2asc_i32(u8 *buf, s32 n, u8 w, u8 p, u8 bcd, u8 nozeros){ NUM2ASC_s32(n, buf, w,p, bcd,nozeros); }

// calculate number width

inline u8 width_i32(s32 n){
  u8 s = 0;
  if (n < 0){ s = 1; n = -n; }
  return (((u32)n < 10uL) ? 1 : ((u32)n < 100uL) ? 2 : ((u32)n < 1000uL) ? 3 : ((u32)n < 10000uL) ? 4 : ((u32)n < 100000uL) ? 5 :
      ((u32)n < 1000000uL) ? 6 : ((u32)n < 10000000uL) ? 7 : ((u32)n < 100000000uL) ? 8 : ((u32)n < 1000000000uL) ? 9 : 10) + s;
}

#define SMS_PDU_MAX_SIZE 400

// data coding scheme
enum
{
  PDU_DCS_7 = 0,
  PDU_DCS_8 = 0x4,
  PDU_DCS_UCS2 = 0x08,
  PDU_DCS_AUTO = 0xFF
};

//SMS classes
enum
{
  class0 = 0,
  class1,
  class2,
  class3,
  noClass = 0xFF
};

//Output format
typedef enum
{

  HEX_raw = 0,
  UTF8_default
}SMS_TXT_OUT_FMT_E;

// number format
typedef enum
{
  PDU_TYPE_UNKNOWN       = 0x81,
  PDU_TYPE_NATIONAL      = 0xA1,
  PDU_TYPE_INTERNATIONAL = 0x91,
  PDU_TYPE_ALPHADET      = 0xD0,
  PDU_TYPE_NETWORK       = 0xB1
} AZX_SMS_ADDR_NUM_TYPE_E;

typedef struct
{
  // SMSC number
  struct{ uint8_t len, bytes, type, data[14]; }smsc;
  // caller/sender number
  struct{ uint8_t len, bytes, type, data[14]; }sender;
  // input/output zero-terminated message (7bit/8bit/UTF8)
  struct{ uint16_t len, bytes; uint8_t *data; }msg;

  uint8_t first;      // 1st octet of the SMS-SUBMIT message (0x11)
  uint8_t tp_msg_ref; // TP-Message-Reference. The "00" value here lets the phone set the message  reference number itself
  uint8_t tp_pid;     // Protocol identifier (0)
  uint8_t tp_dcs;     // Data coding scheme.This message is coded  according to the 7bit default alphabet. Having "02" instead of "00" here, would  indicate that the TP-User-Data field of this message should be interpreted as  8bit rather than 7bit (used in e.g. smart messaging, OTA provisioning etc)
  uint8_t tp_vp;      // TP-Validity-Period. "AA" means 4 days. Note: This  octet is optional, see bits 4 and 3 of the first  octet

  // incoming SMS timestamp
  uint8_t year, month, date; // date
  uint8_t hour, min, sec;    // time
  int8_t tz;                // zone

  // PDU length in bytes and for CMGS command
  uint16_t len_bytes, len_cmgs;
}pdu_struct;

typedef struct
{

  uint8_t smCompressed;
  uint8_t smAlphabet;
  uint8_t smClass;

}dcs_struct;

void pdu_set_output_format(SMS_TXT_OUT_FMT_E output_format);
SMS_TXT_OUT_FMT_E pdu_get_output_format(void);
uint8_t pdu_hex_to_ascii_format (uint8_t LV_nmb);

// Change GSM 7-bit table to ANSI
uint8_t pdu_corr_sym_unpack_dcs7(char *Str, uint8_t len);
// pack 7-bit array to 8-bit
uint8_t pdu_7to8(uint8_t *a, uint8_t len);
// unpack 8-bit array to 7-bit
uint8_t pdu_8to7(uint8_t *a, uint8_t len);

// convert HEX line to bytes, len - length of input buffer
void pdu_hex2bytes(uint8_t *h, uint16_t len, uint8_t *b);
// convert bytes to HEX line, len - length of input buffer
void pdu_bytes2hex(uint8_t *b, uint16_t len, uint8_t *h);

// decode incoming PDU (in binary stream format) to out structure, out->msg.data points to start of incoming text
void pdu_in_decode_binary(uint8_t *in, uint16_t len, pdu_struct *out);

// decode incoming PDU (in ASCII string format) to out structure, out->msg.data points to start of incoming text
void pdu_in_decode(uint8_t *in, uint16_t len, pdu_struct *out);
// decode text PDU, in - start of incoming text, in_bytes - its length in bytes, in_dcs - Data Coding Scheme, out - output buffer, returns number of chars
uint16_t pdu_in_decode_text(uint8_t *in, uint16_t in_bytes, uint8_t in_dcs, uint8_t *out);

// encode outcoming PDU
uint16_t pdu_out_encode(pdu_struct *in, uint8_t *out);
// encode outcoming PDU, simple interface
uint16_t pdu_out_encode_simple(pdu_struct *pdu, uint8_t *out, AZX_SMS_ADDR_NUM_TYPE_E type, void *sender, void *msg, uint8_t tp_vp, int dsc);

// pack text phone number to bytes, returns number of packed digits (11/12), not including terminating F
uint8_t pdu_phone_pack(uint8_t *in, uint8_t *out);
// unpack phone number to string, returns number of unpacked digits
uint8_t pdu_phone_unpack(uint8_t *in, uint8_t len, uint8_t *out, uint8_t type);
// check if phone is packed
uint8_t pdu_phone_is_packed(uint8_t *phone);

// get number of bytes from number of digits
uint8_t pdu_phone_digits_bytes(uint8_t digits);
// get number of bytes of packed phone
uint8_t pdu_phone_bytes(uint8_t *in);
// count number of digits
uint8_t pdu_phone_digits(uint8_t *in, uint8_t bytes);

// compare 2 packed phones
uint8_t pdu_phone_cmp(uint8_t *in1, uint8_t in1_bytes, uint8_t *in2, uint8_t in2_bytes);
// copy packed phone
void pdu_phone_copy(uint8_t *in, uint8_t *out);

#endif /* AZX_PDU_CODEC_AZX_PDUCOMMON_H_ */
