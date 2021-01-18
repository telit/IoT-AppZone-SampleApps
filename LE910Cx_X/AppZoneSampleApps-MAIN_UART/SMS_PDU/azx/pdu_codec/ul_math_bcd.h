/*
 * ul_bcd.h
 *
 *  Created on: Sep 11, 2011
 *      Author: qwer1
 */

#ifndef UL_BCD_H_
#define UL_BCD_H_

#include "ul_other.h"

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


#endif /* UL_BCD_H_ */
