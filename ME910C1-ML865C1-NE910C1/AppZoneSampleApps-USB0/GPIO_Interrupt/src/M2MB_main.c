/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to use GPIOs and interrupts. Debug prints on USB0
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    02/10/2019
 */
/* Include files ================================================================================*/
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_gpio.h"

#include "azx_log.h"
#include "app_cfg.h"

#define OUTPUT_GPIO_PIN_STRING "4"
#define INPUT_GPIO_PIN_STRING "3"



/*Short GPIO 3 and GPIO 4 with a jumper*/


/* Local defines ================================================================================*/
/* define to disable DEBUG_SIM */
/* #defined( NDEBUG_SIM ) */

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
/* Local function prototypes ====================================================================*/



/* Static functions =============================================================================*/
static void gpio_interr_cb(UINT32 fd,  void *userdata )
{
  M2MB_GPIO_VALUE_E value;
  AZX_LOG_INFO("CALLBACK->Interrupt on GPIO %s! ", (char*) userdata);
  m2mb_gpio_read( fd, &value );
  AZX_LOG_INFO("Value: %d\r\n", value);
}

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
  INT32 ret;
  int count = 0;

  INT32 gpio_out_fd, gpio_in_fd;
  


  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting GPIO interrupt demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);
        
  
  m2mb_os_taskSleep(M2MB_OS_MS2TICKS(2000));

  gpio_out_fd = m2mb_gpio_open( "/dev/GPIO" OUTPUT_GPIO_PIN_STRING, 0 );
  /*gpio as output*/
  if( gpio_out_fd != -1 )
  {
    ret = m2mb_gpio_ioctl( gpio_out_fd, M2MB_GPIO_IOCTL_SET_DIR, M2MB_GPIO_MODE_OUTPUT );
    if ( ret == -1 )
      return;

    ret = m2mb_gpio_ioctl( gpio_out_fd, M2MB_GPIO_IOCTL_SET_PULL, M2MB_GPIO_PULL_UP );
    ret = m2mb_gpio_ioctl( gpio_out_fd, M2MB_GPIO_IOCTL_SET_DRIVE, M2MB_GPIO_MEDIUM_DRIVE );
  }
  else
  {
    AZX_LOG_ERROR("Cannot open GPIO " OUTPUT_GPIO_PIN_STRING ". return...\r\n");
    return;
  }

  gpio_in_fd = m2mb_gpio_open( "/dev/GPIO" INPUT_GPIO_PIN_STRING, 0 );

  /*Activate the interrupts on gpio.*/
  if( gpio_in_fd != -1 )
  {

    AZX_LOG_INFO("Setting gpio " INPUT_GPIO_PIN_STRING " interrupt...\r\n");

    ret = m2mb_gpio_multi_ioctl( gpio_in_fd, CMDS_ARGS( M2MB_GPIO_IOCTL_SET_DIR, M2MB_GPIO_MODE_INPUT,    /*Set gpio in input mode*/
        M2MB_GPIO_IOCTL_SET_PULL, M2MB_GPIO_PULL_DOWN,            /*Set pull configuration as pull down*/
        M2MB_GPIO_IOCTL_SET_DRIVE, M2MB_GPIO_MEDIUM_DRIVE ,        /*Pull drive set to medium*/
        M2MB_GPIO_IOCTL_SET_INTR_TYPE, INTR_CB_SET,            /*Select interrupt type as callback function*/
        M2MB_GPIO_IOCTL_SET_INTR_TRIGGER,  M2MB_GPIO_INTR_ANYEDGE,    /*Select interrupt event on both edges*/
        M2MB_GPIO_IOCTL_SET_INTR_CB,  (UINT32)gpio_interr_cb,        /*Register interrupt callback*/
        M2MB_GPIO_IOCTL_SET_INTR_ARG,  (UINT32)INPUT_GPIO_PIN_STRING,          /*Register interrupt callback parameter*/
        M2MB_GPIO_IOCTL_INIT_INTR,  (UINT32)NULL));             /*enable interrupts*/
    if (ret == -1)
    {
      AZX_LOG_ERROR("setting gpio interrupt failed!\r\n");
    }

  }
  else
  {
    AZX_LOG_ERROR("Cannot open GPIO " INPUT_GPIO_PIN_STRING ". return...\r\n");
    return;
  }

  /*Now GPIO 4 will be moved in order to stimulate the interrupts on GPIO 3*/
  while( count++ < 10000)
  {

    AZX_LOG_INFO("Setting GPIO " OUTPUT_GPIO_PIN_STRING " HIGH\r\n");
    m2mb_gpio_write( gpio_out_fd, M2MB_GPIO_HIGH_VALUE );

    m2mb_os_taskSleep(M2MB_OS_MS2TICKS(1000));

    AZX_LOG_INFO("Setting GPIO " OUTPUT_GPIO_PIN_STRING " LOW\r\n");
    m2mb_gpio_write( gpio_out_fd, M2MB_GPIO_LOW_VALUE );

    m2mb_os_taskSleep(M2MB_OS_MS2TICKS(1000));
  }

  AZX_LOG_INFO("The application has ended...\r\n");


}
