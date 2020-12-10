/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showing how to check if USB cable is plugged in or not. Debug prints on AUX UART
  @version 
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    19/08/2020
 */

/* Include files ================================================================================*/
#include "m2mb_types.h"
#include "m2mb_usb.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
BOOLEAN is_USB_connected;
INT32 g_USBfd = -1;
BOOLEAN state = FALSE;

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/


static void USB_Cb( INT32 fd, M2MB_USB_IND_E usb_event, UINT16 resp_size, void *resp_struct, void *userdata )
{
  (void)fd;
  (void)resp_size;
  (void)userdata;

  if( usb_event == M2MB_USB_CABLE_CONNECTED_EVENT )
  {
    is_USB_connected = *((BOOLEAN*)(resp_struct));
    AZX_LOG_INFO("Usb cable check event, USB status: %d\r\n", is_USB_connected);
  }

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

  M2MB_USB_CFG_T cfg;
  INT32 retVal;

  azx_sleep_ms(2000);
  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("\r\nStarting USB cable check demo app. This is v%s built on %s %s.\r\n\r\n",
      VERSION, __DATE__, __TIME__);

  //open usb port

  g_USBfd = m2mb_usb_open("/dev/USB0", 0);
  if ( g_USBfd != -1 )
  {
    AZX_LOG_INFO( "m2mb_usb_open succeeded\r\n");
  }
  else
  {
    AZX_LOG_ERROR( "m2mb_usb_open failed\r\n");
    return;
  }

  retVal = m2mb_usb_ioctl(g_USBfd, M2MB_USB_IOCTL_GET_CFG, &cfg);
  if ( retVal != -1 )
  {
    retVal = m2mb_usb_ioctl(g_USBfd, M2MB_USB_IOCTL_SET_CB, &USB_Cb);
    if ( retVal != -1 )
    {
      AZX_LOG_INFO( "m2mb_usb_ioctl: set usb callback\r\n");
    }
  }

  retVal = m2mb_usb_ioctl(g_USBfd, M2MB_USB_IOCTL_GET_CABLE_STATE, &state);
  if ( retVal != -1 )
  {
    AZX_LOG_INFO( "m2mb_usb_ioctl: got cable status\r\n");
    if (state == 0)
    {
      AZX_LOG_INFO( "USB cable DISCONNECTED, status: %d\r\n", state);
    }
    else
    {
      AZX_LOG_INFO( "USB cable CONNECTED, status: %d\r\n", state);
    }
  }

  AZX_LOG_INFO( "\r\nWaiting for USB cable to be plugged/unplugged...\r\n");


}

