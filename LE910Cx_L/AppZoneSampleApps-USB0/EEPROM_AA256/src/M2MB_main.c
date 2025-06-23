/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to communicate with a MicroChip 24AA256T I2C EEPROM chip using azx eeprom utility APIs. Debug prints on USB0
    
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    03/11/2020
 */


/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

#include "i2c_utils.h"
#include "azx_eeprom_24XX.h"


#define I2C_SDA       (UINT8) 2
#define I2C_SCL       (UINT8) 3


/* Static functions =============================================================================*/
/* Global functions =============================================================================*/
int run_I2C_EEPROM_Demo(void)
{

  INT32 res;
  EEPROM_I2C_T i2c_h = {0};
  AZX_EEPROM_T AA256_h = {{0}};

  UINT8 buffer[257] = {0};
  UINT16 randomAddr = 0x0213;
  char example_byte;
  char example_pages[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZ................................................abcdefghijklmnopqrstuvwxyz..........................................................................ABCDEFGHIJKLMNOPQRSTUVWXYZ..........................................................................abcdefghijklmnopqrstuvwxyz..........................................................................ABCDEFGHIJKLMNOPQRSTUVWXYZ..........................................................................abcdefghijklmnopqrstuvwxyz..........................................................................ABCDEFGHIJKLMNOPQRSTUVWXYZ..........................................................................abcdefghijklmnopqrstuvwxyz..........................................................................ABCDEFGHIJKLMNOPQRSTUVWXYZ..........................................................................abcdefghijklmnopqrstuvwxyz..........................................................................ABCDEFGHIJKLMNOPQRSTUVWXYZ....................STOP";

  UINT8 a2 = 0, a1 = 0, a0 = 0;  /* Use all address chip select bits to 0 (connected to GND) */

  i2c_h.scl_pin = I2C_SCL;
  i2c_h.sda_pin = I2C_SDA;

  res = azx_eeprom_init(&AA256_h, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("Cannot init EEPROM! %d\r\n", res);
    return -1;
  }

  /* Writing '5' and '6' as single bytes*/
  example_byte = 0x35;

  AZX_LOG_INFO("Writing 1 byte at address 0x%04X...\r\n", randomAddr);
  res = azx_eeprom_writeByte(&AA256_h, randomAddr, example_byte);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done.\r\n");
  }

  example_byte = 0x36;

  AZX_LOG_INFO("Writing 1 byte at address 0x%04X...\r\n", randomAddr + 1);
  res = azx_eeprom_writeByte(&AA256_h, randomAddr + 1, example_byte);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done.\r\n");
  }
  

  AZX_LOG_INFO("Reading the 2 bytes from address 0x%04X...\r\n", randomAddr);
  res = azx_eeprom_readPages(&AA256_h, randomAddr, buffer, 2);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done. Data: [0x%02X 0x%02X]\r\n", buffer[0], buffer[1]);
  }

  AZX_LOG_INFO("\r\nWriting %d bytes at address 0x0000..\r\n", strlen(example_pages));
  res = azx_eeprom_writePages(&AA256_h, 0x00, (UINT8*) example_pages, (UINT16) strlen(example_pages));
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done.\r\n");
  }

  AZX_LOG_INFO("\r\nReading %d bytes from address 0x0000...\r\n", sizeof(buffer) -1);
  memset(buffer,0,sizeof(buffer));
  res = azx_eeprom_readPages(&AA256_h, 0x00, buffer, sizeof(buffer) -1);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done. Data: \r\n<<%s>>\r\n", buffer);
  }


  AZX_LOG_INFO("\r\nReading %d bytes sequentially from address 0x0000...\r\n", sizeof(buffer) -1);
  memset(buffer,0,sizeof(buffer));
  res = azx_eeprom_readSequentially(&AA256_h, 0x00, buffer, sizeof(buffer) -1);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done. Data: \r\n<<%s>>\r\n", buffer);
  }




  AZX_LOG_INFO("\r\n\r\nReading 1 byte from address 0x0000...\r\n");
  memset(buffer,0,sizeof(buffer));
  res = azx_eeprom_readByte(&AA256_h, 0x00, buffer);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done. Data: '%c'\r\n", buffer[0]);
  }
  AZX_LOG_INFO("\r\nReading 1 byte from current address (should be 0x0001)...\r\n");
  res = azx_eeprom_readByteFromCurrentAddress(&AA256_h, buffer);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done. Data: '%c'\r\n", buffer[0]);
  }

  AZX_LOG_INFO("\r\n\r\n");
  AZX_LOG_DEBUG("Erasing all the eeprom...\r\n");
  res = azx_eeprom_eraseAll(&AA256_h);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_DEBUG("Done\r\n");
  }


  AZX_LOG_INFO("\r\nReading the 2 bytes from address 0x%04X...\r\n", randomAddr);
  res = azx_eeprom_readPages(&AA256_h, randomAddr, buffer, 2);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done. Data: [0x%02X 0x%02X]\r\n", buffer[0], buffer[1]);
  }


  AZX_LOG_INFO("\r\nDeinit EEPROM...\r\n");
  res = azx_eeprom_deinit(&AA256_h);
  if(AZX_EEPROM_RES_OK != res)
  {
    AZX_LOG_ERROR("res: %d\r\n", res);
  }
  else
  {
    AZX_LOG_INFO("Done\r\n");
  }
  return 0;
}
/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
**************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;


  azx_sleep_ms(2000);

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting I2C EEPROM 24AA256T demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);

  run_I2C_EEPROM_Demo();



}

