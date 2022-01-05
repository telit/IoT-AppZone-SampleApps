/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    azx_pduDec.h

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
#ifndef HDR_AZX_PDUDEC_H_
#define HDR_AZX_PDUDEC_H_

/* Global declarations ==========================================================================*/
/* Global typedefs ==============================================================================*/
/* Global functions =============================================================================*/
/** \defgroup pduDecUsage Usage of pdu decoding functionalities
*    Functions usable to retrieve ASCII message and other info from a PDU message in binary format
*/
#include "azx_pduCommon.h"


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Decodes an incoming PDU in binary format

  @details
    This function decodes the incoming PDU (in binary stream format) to output structure.
    Sender Number and SMS text message are returned as strings
    
  @param[in] pdu
    Pointer to the buffer containing the pdu stream in binary format
  @param[in] pdu_len
    Size in bytes of the pdu
  @param[in] packet
    Pointer to the pdu_struct that will be filled with all the information about the PDU
  @param[in] number
    String buffer that will be filled with the caller number
  @param[in] msg
    String buffer that will be filled with the incoming text message in ascii format
        
  @return
    The length of the decoded text message

  <b>Refer to</b>
    azx_pdu_Encoder()
  @ingroup pduDecUsage        
 
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_pdu_decode(UINT8 *pdu, UINT32 pdu_len, pdu_struct *packet, CHAR *number, CHAR *msg);
/** \example{lineno} azx_pduDec_example.c
  * This is a example PDU decoding functions usage.
  */


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Decodes an incoming PDU in ASCII string format

  @details
    This function decodes the incoming PDU (in ASCII string format) to output structure.
    Sender Number and SMS text message are returned as strings
  @param[in] pdu_str
    Pointer to the buffer containing the pdu string
  @param[in] packet
    Pointer to the pdu_struct that will be filled with all the information about the PDU
  @param[in] number
    String buffer that will be filled with the caller number
  @param[in] msg
    String buffer that will be filled with the incoming text message in ascii format
  
  @return
      The length of the decoded text message
  
  <b>Refer to</b>
    azx_pdu_Encoder()
  @ingroup pduDecUsage           
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_pdu_decodeString(CHAR *pdu_str, pdu_struct *packet, CHAR *number, CHAR *msg);

  
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts UTF8 spaces into ASCII spaces

  @details
    If the decoded PDU has an UTF8 coding, spaces will be returned as 2 bytes (0xc2, 0xa0), this function converts
    them to ASCII spaces (0x20)
    
  @param[inout] in
    pointer to the buffer containing the UTF8 data. It will be converted and in param will be overwritten
  @param[in] in_len
    length of the buffer

  @return
    The length of the converted message
    
  @ingroup pduDecUsage        
*/
/*-----------------------------------------------------------------------------------------------*/
UINT16 azx_pdu_s_utf8_to_ascii(UINT8 *in, UINT16 in_len);

#endif /* HDR_AZX_PDUDEC_H_ */
