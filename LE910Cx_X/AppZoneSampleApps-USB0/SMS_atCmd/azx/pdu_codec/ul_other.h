/*
 * ul_other.h
 *
 *  Created on: Sep 27, 2011
 *      Author: qwer1
 */

#ifndef UL_OTHER_H_
#define UL_OTHER_H_

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

#endif /* OTHER_H_ */
