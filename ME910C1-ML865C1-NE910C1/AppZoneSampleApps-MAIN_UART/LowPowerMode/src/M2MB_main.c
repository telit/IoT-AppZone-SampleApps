/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    The application shows how to set the module in low power modes (by disabling UART and RF). Debug prints on MAIN UART which it is enabled/disabled to reach low power mode, <ins>using AZX log example functions</ins>
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
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_uart.h"
#include "m2mb_sys.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/


/* Set to 1 to also disable the RF part when going in low power.
 * Set to 0 to keep it enabled and registered to the network */
#define DISABLE_RADIO 0


#if DISABLE_RADIO
/*Internal events defines */
#define EV_SYS_OPMODE_SET_RES_BIT    (UINT32)0x00000001
#define EV_SYS_OPMODE_IND_BIT        (UINT32)0x00000002

#define EV_SYS_FAIL_RES_BIT           (UINT32)0x80000000
#endif  /*DISABLE_RADIO*/

/* Local typedefs ===============================================================================*/
#if DISABLE_RADIO
typedef enum
{
  ONLINE = 0,
  LOW_POWER = 1
} RADIO_MODE_E;
#endif  /*DISABLE_RADIO*/

/* Local statics ================================================================================*/
#if DISABLE_RADIO
static M2MB_SYS_HANDLE sys_handle = NULL;
static M2MB_OS_EV_HANDLE sys_evHandle = NULL;
#endif  /*DISABLE_RADIO*/

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/

#if DISABLE_RADIO
INT32 setRadioMode(RADIO_MODE_E mode)
{
  INT32 res = 0;
  if (m2mb_sys_set_operating_mode(sys_handle, (M2MB_SYS_OP_MODE_E)mode, FALSE) != M2MB_RESULT_SUCCESS )
  {
    res = -1;
  }
  else
  {
    M2MB_OS_RESULT_E        osRes;
    UINT32                  curEvBits;
    osRes = m2mb_os_ev_get( sys_evHandle,
        EV_SYS_OPMODE_SET_RES_BIT | EV_SYS_FAIL_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(5000));
    if(osRes != M2MB_OS_SUCCESS)
    {
      AZX_LOG_ERROR("set operating mode timeout!\r\n");
      res = -2;
    }
    else
    {
      if ((curEvBits & EV_SYS_FAIL_RES_BIT) == EV_SYS_FAIL_RES_BIT)
      {
        AZX_LOG_ERROR("Failure event arrived!\r\n");
        res = -3;
      }

      /*else success*/
    }
  }

  return res;
}


void sys_CB(M2MB_SYS_HANDLE h, M2MB_SYS_IND_E sys_event, UINT16 resp_size, void *resp_struct, void *userdata)
{
  (void) h;
  (void) resp_size;
  (void) userdata;

  switch(sys_event)
  {
  case M2MB_SYS_SET_OPERATING_MODE_RESP:
  {
    M2MB_SYS_GENERIC_RESP_T *resp = (M2MB_SYS_GENERIC_RESP_T*)resp_struct;
    if(resp->response == M2MB_RESULT_SUCCESS)
    {
      AZX_LOG_INFO( "Operating mode set result OK\r\n");
      m2mb_os_ev_set(sys_evHandle, EV_SYS_OPMODE_SET_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_ERROR( "Operating mode set result %d\r\n", resp->response );
      m2mb_os_ev_set(sys_evHandle, EV_SYS_FAIL_RES_BIT, M2MB_OS_EV_SET);
    }
    break;

  }
  break;
  case M2MB_SYS_OPERATING_MODE_CHANGED_IND:
  {
    M2MB_SYS_OPERATING_MODE_T *resp = (M2MB_SYS_OPERATING_MODE_T*)resp_struct;
    AZX_LOG_INFO("SET OPERATING MODE is operatingMode %d, persistent %d\r\n", resp->operatingMode, resp->persistent);
  }
  break;

  default:
    break;
  }
}
#endif /*DISABLE_RADIO*/
/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;
  int err;

  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting low power demo. This is v%s built on %s %s. LEVEL: %d\r\n",
      VERSION, __DATE__, __TIME__, azx_log_getLevel());

  AZX_LOG_INFO("Please ensure USB native port is unplugged during low power state\r\n");

#if DISABLE_RADIO
  {
    M2MB_OS_RESULT_E        osRes;
    M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;

    /* Init events handler */
    osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL));
    osRes = m2mb_os_ev_init( &sys_evHandle, &evAttrHandle );

    if ( osRes != M2MB_OS_SUCCESS )
    {
      m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
      AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
      return;
    }
    m2mb_os_ev_set(sys_evHandle, EV_SYS_FAIL_RES_BIT, M2MB_OS_EV_CLEAR);

  }

  /* Initialize sys subsystem */
  if(m2mb_sys_init( &sys_handle, sys_CB, NULL ) != M2MB_RESULT_SUCCESS)
  {
    AZX_LOG_CRITICAL("Cannot initialize sys subsystem\r\n");
    return;
  }
#endif  /*DISABLE_RADIO*/

  AZX_LOG_INFO("Sleeping 20 seconds...\r\n");
  azx_sleep_ms(20000);

#if DISABLE_RADIO
  /*
   Disabling RF
   */
  AZX_LOG_INFO("Disabling RF...\r\n");
  err = setRadioMode(LOW_POWER);
  if( err != 0)
  {
    AZX_LOG_ERROR("Failed with errCode = %d!\r\n", err);
  }
#endif  /*DISABLE_RADIO*/

  /*
   Disabling LOG UART.
   If another UART is used, a brief example code is shown below to reach low power consumption:

   #define UART_INSTANCE  "/dev/tty0"
   Open UART    ->  INT32 uart_fd = m2mb_uart_open( UART_ISTANCE, 0 );
   Disable UART ->  INT32 res = m2mb_uart_ioctl( uart_fd, M2MB_UART_IOCTL_SET_POWER_STATE, FALSE );
   [LOW POWER]
   Enable UART  ->  INT32 res = m2mb_uart_ioctl( uart_fd, M2MB_UART_IOCTL_SET_POWER_STATE, TRUE );
   Close UART   ->  m2mb_uart_close( uart_fd );
   */
  AZX_LOG_INFO("Disable LOG UART to reach low power mode and sleep for 20 seconds!\r\n");
  err = azx_log_disableUart();
  if( err != 0)
  {
    AZX_LOG_ERROR("Failed with errCode = %d!\r\n", err);
  }

  azx_sleep_ms(20000);

  /*
   Enabling LOG UART.
   */
  err = azx_log_enableUart();
  if( err != 0)
  {
    AZX_LOG_ERROR("Failed with errCode = %d!\r\n", err);
  }
  AZX_LOG_INFO("LOG UART is enabled again!\r\n");
#if DISABLE_RADIO
  /*
   Enabling RF
   */
  AZX_LOG_INFO("Enabling RF...\r\n");
  err = setRadioMode(ONLINE);
  if( err != 0)
  {
    AZX_LOG_ERROR("Failed with errCode = %d!\r\n", err);
  }
#endif  /*DISABLE_RADIO*/

  AZX_LOG_INFO("Sleeping 60 seconds...\r\n");
  azx_sleep_ms(60000);

#if DISABLE_RADIO
  m2mb_sys_deinit(sys_handle);
  m2mb_os_ev_deinit(sys_evHandle);
#endif

  AZX_LOG_INFO("Application end\r\n");

  azx_sleep_ms(2000);
}

