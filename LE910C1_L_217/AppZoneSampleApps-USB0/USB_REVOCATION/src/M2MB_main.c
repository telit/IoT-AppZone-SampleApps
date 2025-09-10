/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    The application prints "Hello World!" on USB 0 every 2 seconds using 
    m2mb_uart_* apis
  @version 
    1.0.2
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
#include <unistd.h>

#include "m2mb_types.h"

#include "m2mb_os_api.h" /*For sleep functionality*/

#include "m2mb_usb.h"
#include <syslog.h>


#define USBAPP_DEBUG(fmt, args...) syslog(LOG_INFO, fmt, ##args)

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
INT32 out_ch_fd = -1;
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
  char out_buffer[512];
  int counter = 0;
  int ret;
  
  USBAPP_DEBUG("TEST M2MB_USB: APPLICATION STARTED.....");
  
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS(5000) );

  out_ch_fd = m2mb_usb_open( "/dev/USB1", 0 );
  USBAPP_DEBUG("TEST M2MB_USB: usb open fd[u=%u, d=%d]", out_ch_fd, out_ch_fd );
  if( out_ch_fd == -1 )
  {
    USBAPP_DEBUG("TEST M2MB_USB: usb open failed");
    return;
  }
  USBAPP_DEBUG("TEST M2MB_USB: usb open success");


  memset(out_buffer,0,sizeof(out_buffer));
  sprintf(out_buffer, "\r\n Start Hello world Application  [ version: %f ] \r\n", 2.0 );
  ret = m2mb_usb_write(out_ch_fd, (char*) out_buffer, strlen(out_buffer));
  USBAPP_DEBUG("TEST M2MB_USB: usb write[%d] ret[u=%u, d=%d]", strlen(out_buffer), ret, ret );

  while (1)
  {
    counter++;
    if ( counter > 999999 )
    {
        memset(out_buffer,0,sizeof(out_buffer));
        sprintf(out_buffer, "\r\n Counter zeroed " );
        m2mb_usb_write(out_ch_fd, (char*) out_buffer, strlen(out_buffer));
        counter = 0;
        break;
    }

    memset(out_buffer,0,sizeof(out_buffer));
    sprintf(out_buffer, "\r\n Hello world 2.0 [ %06d ] ", counter );
    ret = m2mb_usb_write(out_ch_fd, (char*) out_buffer, strlen(out_buffer));
    USBAPP_DEBUG("TEST M2MB_USB: usb write[%d] ret[u=%u, d=%d][counter=%d]", strlen(out_buffer),ret, ret, counter );
    if( ret > 0 )
    {
      USBAPP_DEBUG("TEST M2MB_USB: usb write success");
    }
    else
    {
      USBAPP_DEBUG("TEST M2MB_USB: usb write failed");
      break;
    }
    sleep(10);

  }
  USBAPP_DEBUG("TEST M2MB_USB: usb going to close ");
  ret = m2mb_usb_close(out_ch_fd);
  USBAPP_DEBUG("TEST M2MB_USB: usb close ret[u=%u, d=%d]", ret, ret );
  USBAPP_DEBUG("TEST M2MB_USB: APPLICATION STOPPED.....");
  
}

