/*===================================================================================*/
/*>>> Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved. <<<*/
/*        See LICENSE file in the project root for full license information.         */
/**
  @file
    ul_gsm_pdu.h

  @brief
    PDU defines
  @details
    porting of original code at https://github.com/tardigrade888/c-pdu

  @note
    Dependencies:
       m2mb_types.h

  @author
    
  @date
     24/10/2018
*/
#ifndef HDR_UL_GSM_PDU_H_
#define HDR_UL_GSM_PDU_H_

// data coding scheme
enum 
{
  PDU_DCS_7 = 0, 
  PDU_DCS_8 = 0x4, 
  PDU_DCS_UCS2 = 0x08, 
  PDU_DCS_AUTO = 0xFF
};

// number format
enum 
{
  PDU_TYPE_NATIONAL = 0xA1, 
  PDU_TYPE_INTERNATIONAL = 0x91, 
  PDU_TYPE_ALPHADET = 0xD0,
  PDU_TYPE_NETWORK = 0xB1
};

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
uint16_t pdu_out_encode_simple(pdu_struct *pdu, uint8_t *out, void *sender, void *msg, uint8_t tp_vp, int dsc);

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

#endif /* HDR_UL_GSM_PDU_H_ */
