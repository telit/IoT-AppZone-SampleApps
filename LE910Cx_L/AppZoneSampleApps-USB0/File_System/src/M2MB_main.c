/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing M2MB File system API usage. Debug prints on USB0
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
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_fs_posix.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

static char file_buffer[] = "Hello from file";
static char file_name[] = LOCALPATH "/my_text_file.txt";

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
  
  INT32 fs_res;
  INT32 fd = -1;
  CHAR recv[20];
  
  azx_sleep_ms(5000);

  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting FileSystem demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);


  AZX_LOG_INFO("Opening %s in write mode..\r\n", file_name);
  fd = m2mb_fs_open(file_name,
            M2MB_O_CREAT | M2MB_O_WRONLY  /*Create file if not existing, open in write only mode*/,
            M2MB_ALLPERMS /*0777*/
            );
  if (fd == -1 )
  {
    AZX_LOG_ERROR("Cannot open file\r\n");
    return  ;
  }

  fs_res = m2mb_fs_write(fd, file_buffer, strlen(file_buffer));
  if (fs_res != (INT32)strlen(file_buffer))
  {
    AZX_LOG_ERROR("Failed writing buffer into file.\r\n");
  }
  else
  {
    AZX_LOG_INFO("Buffer written successfully into file. %d bytes were written.\r\n", fs_res);
  }

  AZX_LOG_INFO("Closing file.\r\n");
  m2mb_fs_close(fd);

  AZX_LOG_INFO("Opening %s in read only mode..\r\n", file_name);
  fd = m2mb_fs_open(file_name, M2MB_O_RDONLY);

  memset(recv,0,sizeof(recv));
  fs_res = m2mb_fs_read(fd,recv,sizeof(recv));


  AZX_LOG_INFO("Received %d bytes from file: \r\n<%.*s> \r\n", fs_res, fs_res, recv);

  AZX_LOG_INFO("Closing file.\r\n");
  m2mb_fs_close(fd);

  AZX_LOG_INFO("Deleting File\r\n");

  if (m2mb_fs_unlink(file_name) == 0)
  {
    AZX_LOG_INFO("File deleted \r\n");
  }

  else
  {
    AZX_LOG_ERROR("CANNOT DELETE FILE\r\n");
  }

  AZX_LOG_INFO("App Completed\r\n");
  azx_log_deinit();


}

