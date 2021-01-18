/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    azx_pduEnc.c

  @brief
    Encoding functionalities for PDUs
  @details


  @note
    Dependencies:
       m2mb_types.h

  @author
    Fabio Pintus

  @date
     19/10/2018
*/

/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"

#include "ul_gsm_pdu.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/
INT32 azx_pdu_convertZeroPaddedHexIntoByte(UINT8 *dataset, UINT8 *bytearray, INT32 len)
{
  int j=0,counter=0;
  UINT8 c[2];
  unsigned int bytes[2];

  for(j=0;j<len;j++)
  {
    if(0 == j%2)
    {
      c[0] = dataset[j];
      c[1] = dataset[j+1];

      sscanf((char*)c, "%02x", &bytes[0]);

      bytearray[counter] = bytes[0];

      counter++;
    }
  }
  return counter;
}



INT32 azx_pdu_encode(CHAR *number,CHAR *str, UINT8 *pdu, INT32 dcs)
{

  pdu_struct packet;
  INT32 len;

  memset(&packet,0,sizeof(pdu_struct));

  len = pdu_out_encode_simple(&packet, pdu, number, str, 0, dcs);
  return len;
}
