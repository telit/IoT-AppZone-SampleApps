/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to manage AppZone apps from m2mb code. Debug prints on MAIN UART
  @version 
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    07/05/2020
*/

/* Include files ================================================================================*/
#include "m2mb_types.h"

#include "m2mb_appMng.h"
#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

static const char *APP_STATES_STRINGS[] =
{
    "M2MB_APPMNG_STATE_READY",
    "M2MB_APPMNG_STATE_STARTING",
    "M2MB_APPMNG_STATE_RUN",
    "M2MB_APPMNG_STATE_STOPPING",
    "M2MB_APPMNG_STATE_STOP"
};

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone
s
   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;

  INT32 n_apps;
  M2MB_APPMNG_HANDLE appHandle;
  M2MB_APPMNG_HANDLE myappHandle;
  M2MB_APPMNG_RESULT_E appRes;
  MEM_W  state, delay, delay_ini, ram, address;


  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting App Manager demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);



  n_apps = m2mb_appMng_appsNumber( );
  AZX_LOG_INFO("There are %d configured apps.\r\n", n_apps);


  appHandle = m2mb_appMng_getHandleByName( (char *)"notexisting.bin");
  AZX_LOG_INFO("Not existing app handle test (should be 0): %p\r\n", appHandle);


  myappHandle = m2mb_appMng_getMyHandle();
  AZX_LOG_INFO("Manager app handle: %p\r\n", myappHandle);


  appRes = m2mb_appMng_getItem( myappHandle, M2MB_APPMNG_SEL_CMD_DELAY_INI, &delay_ini, NULL );
  if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
  }
  else
  {
    AZX_LOG_INFO( "Manager app delay from nv memory: %d seconds\r\n", delay_ini );
  }


  delay_ini = 5;
  AZX_LOG_INFO("\r\nChanging Manager app delay time (on non volatile configuration) to %d seconds..\r\n", delay_ini);


  appRes = m2mb_appMng_setItem( myappHandle, M2MB_APPMNG_SEL_CMD_DELAY_INI, (void*)delay_ini, NULL );
  if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
  }

  appRes = m2mb_appMng_getItem( myappHandle, M2MB_APPMNG_SEL_CMD_DELAY_INI, &delay_ini, NULL );
  if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
  }
  else
  {
    AZX_LOG_INFO( "Manager app delay from nv memory is now %d seconds\r\n", delay_ini );
  }


  appRes = m2mb_appMng_getItem( myappHandle, M2MB_APPMNG_SEL_CMD_STATE, &state, NULL );
  if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
  }
  else
  {
    AZX_LOG_INFO( "Manager app state is %s\r\n\r\n", APP_STATES_STRINGS[state] );
  }

  AZX_LOG_INFO("Trying to get Second app handle...\r\n");

  do
  {
    appHandle = m2mb_appMng_getHandleByName((char *)"second.bin");
    if(!appHandle)
    {
      AZX_LOG_WARN("Second app not configured... Trying to add it now\r\n");
      appRes = m2mb_appMng_add(&appHandle, (char *)"second.bin", 1);

      if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("Could not add Second app to permanent list, error %d\r\n", appRes);
        break;
      }
    }


    AZX_LOG_INFO("Second app handle is valid\r\n");

    /*Disable auto start on non volatile configuration*/
    appRes = m2mb_appMng_setItem( appHandle, M2MB_APPMNG_SEL_CMD_SET_EXE_AUTO_INI,
        ( void* )M2MB_APPMNG_NOT_AUTOSTART, NULL );
    if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("m2mb_appMng_setItem failure, error %d\r\n", appRes);
    }


    appRes = m2mb_appMng_getItem( appHandle, M2MB_APPMNG_SEL_CMD_DELAY_INI, &delay_ini, NULL );
    if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
    }
    else
    {
      AZX_LOG_INFO( "2nd app delay from nv memory is %d\r\n", delay_ini );
    }

    appRes = m2mb_appMng_getItem( appHandle, M2MB_APPMNG_SEL_CMD_STATE, &state, NULL );
    if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
    }
    else
    {
      AZX_LOG_INFO( "2nd app current state is %s\r\n", APP_STATES_STRINGS[state] );
    }


    if(state == M2MB_APPMNG_STATE_STOP || state == M2MB_APPMNG_STATE_READY)
    {

      appRes = m2mb_appMng_getItem( appHandle, M2MB_APPMNG_SEL_CMD_ADDRESS, &address, NULL );
      if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
        azx_sleep_ms(200);
      }
      else
      {
        AZX_LOG_INFO("Second app set address is 0x%08X\r\n", address);
      }

      delay = 0;
      AZX_LOG_INFO("Setting volatile Second app delay (not stored in nvm) to %d seconds...\r\n", delay);

      appRes = m2mb_appMng_setItem( myappHandle, M2MB_APPMNG_SEL_CMD_DELAY, (void*)delay, NULL );
      if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_appMng_setItem failure, error %d\r\n", appRes);
      }
      else
      {

        AZX_LOG_INFO("Starting Second app on the fly (without reboot)...\r\n");
        appRes = m2mb_appMng_start(appHandle);
        if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
        {
          AZX_LOG_ERROR("m2mb_appMng_start failure, error %d\r\n", appRes);
        }
        else
        {
          AZX_LOG_INFO("Waiting 2 seconds...\r\n");
          azx_sleep_ms(2000);

          while (state != M2MB_APPMNG_STATE_RUN)
          {
            appRes = m2mb_appMng_getItem( appHandle, M2MB_APPMNG_SEL_CMD_STATE, &state, NULL );
            if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
            {
              AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
              azx_sleep_ms(200);
            }
            else
            {
              AZX_LOG_INFO( "2nd app current state is %s\r\n", APP_STATES_STRINGS[state] );

              if(state == M2MB_APPMNG_STATE_STOP)
              {
                break;
              }
              if (state == M2MB_APPMNG_STATE_STARTING)
              {
                AZX_LOG_INFO("App is starting, waiting 2 more seconds...\r\n");
                azx_sleep_ms(2000);
              }
            }
          }

          if (state != M2MB_APPMNG_STATE_RUN)
          {
            AZX_LOG_ERROR("Failed to start the Second app\r\n");
          }
        }

      }
    }

    if (state == M2MB_APPMNG_STATE_RUN)
    {
      AZX_LOG_INFO("Second app is running!\r\n");

      appRes = m2mb_appMng_getItem( appHandle, M2MB_APPMNG_SEL_CMD_RAM, &ram, NULL );
      if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
      }
      else
      {
        AZX_LOG_INFO("Second App is using %d bytes of RAM\r\n", ram);
      }

      azx_sleep_ms(10000);
      AZX_LOG_INFO("Stopping Second app now...\r\n");
      appRes = m2mb_appMng_stop(appHandle);
      if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_appMng_stop failure, error %d\r\n", appRes);
      }
      else
      {
        AZX_LOG_INFO("wait 10 seconds...\r\n");
        azx_sleep_ms(10000);
        appRes = m2mb_appMng_getItem( appHandle, M2MB_APPMNG_SEL_CMD_STATE, &state, NULL );
        if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
        {
          AZX_LOG_ERROR("m2mb_appMng_getItem failure, error %d\r\n", appRes);
        }
        else
        {
          AZX_LOG_INFO( "2nd app current state is %s\r\n", APP_STATES_STRINGS[state] );
        }
      }
    }

    AZX_LOG_INFO("Set permanent run permission for Second app.\r\n");
    appRes = m2mb_appMng_setItem( appHandle, M2MB_APPMNG_SEL_CMD_SET_EXE_AUTO_INI, ( void* )M2MB_APPMNG_AUTOSTART, NULL );
    if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("m2mb_appMng_setItem failure, error %d\r\n", appRes);
    }
    else
    {
      AZX_LOG_INFO( "Done. Second App will also run from next boot-up\r\n" );
    }

  }while(0);
  
}