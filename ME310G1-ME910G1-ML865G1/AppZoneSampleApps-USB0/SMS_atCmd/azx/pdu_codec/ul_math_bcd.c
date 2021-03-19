#include "ul_math_bcd.h"
#include <stdlib.h>

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

