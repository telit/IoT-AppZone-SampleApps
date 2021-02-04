/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    azx_pduDec.c

  @brief
    Decoding functionalities for PDUs
  @details


  @note
    Dependencies:
       m2mb_types.h

  @author
    Fabio Pintus

  @date
     24/10/2018
*/

/* Include files ================================================================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "m2mb_types.h"

#include "ul_gsm_pdu.h"
#include "azx_pduDec.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/

typedef struct
{
  UINT8 in_lsB;
  UINT8 in_msB;
  UINT8 out;
} utf_ascii_conversion_map_elem;

/* Local statics ================================================================================*/

static char g_Buff[512];

/* Local function prototypes ====================================================================*/

/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

INT32 azx_pdu_decode(UINT8 *pdu, UINT32 pdu_len, pdu_struct *packet, CHAR *number, CHAR *msg)
{
  UINT16 len_msg;


  memset(g_Buff, 0, sizeof(g_Buff));

  //...fill buffer with PDU data
  memcpy(g_Buff,  pdu, pdu_len);
  memset(packet, 0, sizeof(pdu_struct));

  //input data to filling pdu_struct
  pdu_in_decode_binary((UINT8*)&g_Buff[0],
      pdu_len,
      packet);

  // check type SMS
  if(packet->sender.type == PDU_TYPE_INTERNATIONAL)
  {
    pdu_phone_unpack(&(packet->sender.data[0]),
        packet->sender.bytes,
        (UINT8*)number,
        packet->sender.type);
  }

  // check type alphadet
  if(packet->sender.type == PDU_TYPE_ALPHADET)
  {
    pdu_in_decode_text(&(packet->sender.data[0]),
        packet->sender.bytes,
        PDU_DCS_7,
        (UINT8*)number);
  }
  // decoding message

  
  //fill result buffer
  pdu_in_decode_text(packet->msg.data,
      packet->msg.len,
      (packet->tp_dcs == 0x00 ? PDU_DCS_7:PDU_DCS_UCS2),
      (UINT8*)msg);

  len_msg = packet->msg.len;

  // correct symbol
  if(packet->tp_dcs == 0x00)
  {
    len_msg = pdu_corr_sym_unpack_dcs7(&msg[0], packet->msg.len);
  }
  else
  {
    len_msg = azx_pdu_s_utf8_to_ascii((UINT8*) &msg[0], packet->msg.len);
  }

  return len_msg;
}


INT32 azx_pdu_decodeString(CHAR *pdu_str, pdu_struct *packet, CHAR *number, CHAR *msg)
{
  UINT16 len_msg;


  memset(g_Buff, 0, sizeof(g_Buff));

  //...fill buffer with PDU data
  memcpy(g_Buff,  pdu_str, strlen( pdu_str));
  memset(packet, 0, sizeof(pdu_struct));




  //input data to filling pdu_struct
  pdu_in_decode((UINT8*)&g_Buff[0],
      strlen(g_Buff),
      packet);


  // check type SMS
  if(packet->sender.type == PDU_TYPE_INTERNATIONAL)
  {
    pdu_phone_unpack(&(packet->sender.data[0]),
        packet->sender.bytes,
        (UINT8*)number,
        packet->sender.type);
  }

  // check type alphadet
  if(packet->sender.type == PDU_TYPE_ALPHADET)
  {
    pdu_in_decode_text(&(packet->sender.data[0]),
        packet->sender.bytes,
        PDU_DCS_7,
        (UINT8*)number);
  }
  // decoding message

  //fill result buffer
  pdu_in_decode_text(packet->msg.data,
      packet->msg.len,
      (packet->tp_dcs == 0x00 ? PDU_DCS_7:PDU_DCS_UCS2),
      (UINT8*)msg);

  len_msg = packet->msg.len;

  // correct symbol
  if(packet->tp_dcs == 0x00)
  {
    len_msg = pdu_corr_sym_unpack_dcs7(&msg[0], packet->msg.len);
  }
  else
  {
    len_msg = azx_pdu_s_utf8_to_ascii((UINT8*) &msg[0], packet->msg.len);
  }

  return len_msg;
}

utf_ascii_conversion_map_elem conversion[] =
{
    {0xc2, 0xa0, ' '} //NO-BREAK SPACE to ascii space
};


/*small conversion for UTF8 spaces*/
UINT16 azx_pdu_s_utf8_to_ascii(UINT8 *in, UINT16 in_len)
{
  UINT16 out_len=0;
  int conv_size = sizeof(conversion)/sizeof(conversion[0]);

  int i = 0,j, c;

  while (in_len)
  {
    for (c = 0; c < conv_size; c++)
    {
      if( in[i] == conversion[c].in_lsB && in[i+1] == conversion[c].in_msB )
      {
        in[i] = conversion[c].out;
        for (j=i+1; j< in_len; j++)
        {
          in[j] = in[j+1];
          i++;
          in_len--;
        }
      }

    }
    i++;
    in_len--;
    out_len++;
  }
  return out_len;
}

