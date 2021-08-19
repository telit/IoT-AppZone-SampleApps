/*
 * gsm_pdu.c
 *
 *  Created on: Sep 15, 2014
 *      Author: qwer
 */
#include <stdint.h>
#include <string.h>
#include "ul_gsm_pdu.h"
#include "ul_s.h"
/*******************************************************************************
 *                                                                             *
 *******************************************************************************/
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
void pdu_in_decode_binary(uint8_t *in, uint16_t len, pdu_struct *out)
{
	int8_t tz_sign = 1;
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
