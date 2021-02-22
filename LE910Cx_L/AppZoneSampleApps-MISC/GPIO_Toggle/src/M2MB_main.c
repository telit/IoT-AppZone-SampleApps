/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing GPIO usage with M2MB API
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
#include "m2mb_gpio.h"


/* Local defines ================================================================================*/

#define GPIO_PIN "1"
/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
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
  
  int count = 0;
  INT32 ret;
  INT32 gpio_fd;
  
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS(2000) );

  /* Open file descriptor for GPIO*/
  gpio_fd = m2mb_gpio_open( "/dev/GPIO" GPIO_PIN, 0 );
  if( gpio_fd != -1 )
  {
    /* SET GPIO as output */
    ret = m2mb_gpio_ioctl( gpio_fd, M2MB_GPIO_IOCTL_SET_DIR, M2MB_GPIO_MODE_OUTPUT );
    if ( ret == -1 )
      return;

    ret = m2mb_gpio_ioctl( gpio_fd, M2MB_GPIO_IOCTL_SET_PULL, M2MB_GPIO_PULL_UP );
    ret = m2mb_gpio_ioctl( gpio_fd, M2MB_GPIO_IOCTL_SET_DRIVE, M2MB_GPIO_MEDIUM_DRIVE );
  }
  else
  {
    return;
  }

  while(count++ < 10)
  {
    /* set gpio 1 as output and set it HIGH */
    m2mb_gpio_write( gpio_fd, M2MB_GPIO_HIGH_VALUE );
    m2mb_os_taskSleep( M2MB_OS_MS2TICKS(200) );

    /* set gpio 1 as output and set it LOW */
    m2mb_gpio_write( gpio_fd, M2MB_GPIO_LOW_VALUE );
    m2mb_os_taskSleep( M2MB_OS_MS2TICKS(200) );
  }

}


