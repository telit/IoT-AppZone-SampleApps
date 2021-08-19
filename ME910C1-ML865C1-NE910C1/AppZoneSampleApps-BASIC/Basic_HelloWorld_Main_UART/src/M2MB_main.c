/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    The application prints "Hello World!" on Main UART every 2 seconds using 
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

#include "m2mb_types.h"

#include "m2mb_os_api.h" /*For sleep functionality*/

#include "m2mb_uart.h"

#include "app_cfg.h"

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
  
  
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS(5000) );

  /* Main UART is tty0 */
  out_ch_fd = m2mb_uart_open( "/dev/tty0", 0 );


  memset(out_buffer,0,sizeof(out_buffer));
  sprintf(out_buffer, "\r\n Start Hello world Application  [ version: %f ] \r\n", 2.0 );
  m2mb_uart_write(out_ch_fd, (char*) out_buffer, strlen(out_buffer));


  while (1)
  {
    counter++;
    if ( counter > 999999 )
    {
        memset(out_buffer,0,sizeof(out_buffer));
        sprintf(out_buffer, "\r\n Counter zeroed " );
        m2mb_uart_write(out_ch_fd, (char*) out_buffer, strlen(out_buffer));
        counter = 0;
        break;
    }

    memset(out_buffer,0,sizeof(out_buffer));
    sprintf(out_buffer, "\r\n Hello world 2.0 [ %06d ] ", counter );
    m2mb_uart_write(out_ch_fd, (char*) out_buffer, strlen(out_buffer));
    m2mb_os_taskSleep( M2MB_OS_MS2TICKS(2000) );

  }

  m2mb_uart_close(out_ch_fd);
}

