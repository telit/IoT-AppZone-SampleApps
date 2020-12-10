/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to communicate over SPI with m2mb API. Debug prints on MAIN UART
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    02/03/2017
 */
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_spi.h"

#include "m2mb_gpio.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/


/*
 *  Connection scheme
  #####################################################################
                     | CS1467g-A       |  ST
  TX_AUX/MOSI        | PL303/1         | SDI
  RX_AUX/MISO         | PL303/2         | SDO
  SPI_CLK            | PL303/3         | SCK
  GND                | PL303/10        | BLACK
  V3.8               | PL101/9         | RED
  #####################################################################

  Put a Jumper between MOSI and MISO (PL303/1 and PL303/2 pins)
*/

/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
INT32 spi_fd = -1;
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/

/* Global functions =============================================================================*/

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
  
  UINT8 send_buf[32], recv_buf[32];
  
  INT32 spi_fd;
  M2MB_SPI_CFG_T cfg;
  
  azx_sleep_ms(5000);

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting SPI demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);
        
  spi_fd = m2mb_spi_open("/dev/spidev5.0", 0);     
  if (spi_fd  == -1)
  {
    AZX_LOG_CRITICAL("Cannot open SPI channel!\r\n");
    return;
  }
  memset(&cfg, 0, sizeof(cfg));
  memset(send_buf,0,sizeof(send_buf));
  memset(recv_buf,0,sizeof(recv_buf));
  
  
  cfg.spi_mode = M2MB_SPI_MODE_0; //clock idle LOW, data driven on falling edge and sampled on rising edge
  cfg.cs_polarity = M2MB_SPI_CS_ACTIVE_LOW;
  cfg.cs_mode = M2MB_SPI_CS_KEEP_ASSERTED; //M2MB_SPI_CS_DEASSERT;
  cfg.endianness = M2MB_SPI_NATIVE; //M2MB_SPI_LITTLE_ENDIAN; //M2MB_SPI_BIG_ENDIAN;
  cfg.callback_fn = NULL;
  cfg.callback_ctxt = NULL; 
  cfg.clk_freq_Hz = 1000000; /*Frequency in Hz*/
  cfg.bits_per_word = 8;
  cfg.cs_clk_delay_cycles = 3;
  cfg.inter_word_delay_cycles = 0;
  cfg.loopback_mode = FALSE;

  if(-1 == m2mb_spi_ioctl(spi_fd, M2MB_SPI_IOCTL_SET_CFG, (void *)&cfg))
  {
    AZX_LOG_CRITICAL("Cannot set SPI channel configuration!\r\n");
    return;
  }

  sprintf((CHAR*)send_buf,"hello from spi echo");
  if (FALSE == m2mb_spi_write_read(spi_fd, (void*)send_buf, (void*)recv_buf, (SIZE_T)strlen((CHAR*)send_buf)))
  {
    AZX_LOG_ERROR("Failed sending data.\r\n");
  }
  else
  {
    AZX_LOG_INFO("Transfer successful. Received: %s\r\n", recv_buf);
  }
}

