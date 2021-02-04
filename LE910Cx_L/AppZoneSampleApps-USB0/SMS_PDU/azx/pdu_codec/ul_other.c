/*
 * ul_other.c
 *
 *  Created on: Sep 27, 2011
 *      Author: qwer1
 */

#include "ul_other.h"

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
