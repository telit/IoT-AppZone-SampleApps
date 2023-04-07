/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    azx_pduEnc.h

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

#ifndef HDR_PDU_PDU_ENC_H_
#define HDR_PDU_PDU_ENC_H_

/* Global declarations ==========================================================================*/
/* Global typedefs ==============================================================================*/
/* Global functions =============================================================================*/
/** \defgroup pduEncUsage Usage of pdu encoding functionalities
*    Functions usable to create a PDU message from a string text
*/
#include "azx_pduCommon.h"
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts an encoded dataset (created with PDUEncode()) into a binary bytearray

  @details
    Converts a dataset, in zero padded hex format, into a binary bytearray
    
  @param[in] dataset
    buffer containing the text message in pdu format
  @param[in] bytearray
    Pointer to the bytearray buffer that will be filled with the encoded data
  @param[in] len
    Length of the pdu dataset

  @return
    New length of the output bytearray
  
  <b>Refer to</b>
    azx_pdu_Encoder()
      
  @ingroup pduEncUsage
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_pdu_convertZeroPaddedHexIntoByte(UINT8 *dataset, UINT8 *bytearray, INT32 len);


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Encode a PDU from the input parameters

  @details
    This function encodes the PDU (in binary stream format) using the encoding, number and message provided
    
  @param[in] number
    string containig the sender number
  @param[in] str
    string containing the text message
  @param[in] pdu
    pointer to the pdu buffer that will be filled with the encoded data
  @param[in] dcs
    data coding scheme, can bee GSM Standard (PDU_DCS_7), 8bits (PDU_DCS_8) or UCS2 (PDU_DCS_UCS2)
  
  @return
    The length of the encoded pdu
   
  <b>Refer to</b>
    azx_pdu_Decoder()
      
  @ingroup pduEncUsage
*/
/*-----------------------------------------------------------------------------------------------*/
int azx_pdu_encode(CHAR *number, CHAR *str, UINT8 *pdu, INT32 dcs);
/** \example{lineno} azx_pduEnc_example.c
  * This is a example PDU encoding functions usage.
  */


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Encode a PDU from the input parameters, allowing to select the number address type

  @details
    This function encodes the PDU (in binary stream format) using the encoding, number and message provided

  @param[in] type
    Address number type (national/international), refer to  AZX_SMS_ADDR_NUM_TYPE_E enum.
  @param[in] number
    string containig the sender number
  @param[in] str
    string containing the text message
  @param[in] pdu
    pointer to the pdu buffer that will be filled with the encoded data
  @param[in] dcs
    data coding scheme, can bee GSM Standard (PDU_DCS_7), 8bits (PDU_DCS_8) or UCS2 (PDU_DCS_UCS2)
  
  @return
    The length of the encoded pdu

  <b>Refer to</b>
    azx_pdu_Decoder()

  @ingroup pduEncUsage
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_pdu_encode_generic(AZX_SMS_ADDR_NUM_TYPE_E type, CHAR *number, CHAR *str, UINT8 *pdu, INT32 dcs);

#endif /* HDR_PDU_PDU_ENC_H_ */
