/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showing how to compute MD5 hashes using m2mb crypto. Debug prints on AUX UART
  @version 
    1.0.3
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    2020/20/05
 */

/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_fs_posix.h"

#include "azx_log.h"
#include "app_cfg.h" /*FOR LOCALPATH define*/

#include "md5_utils.h"

/* Local defines ================================================================================*/
#define FILE LOCALPATH "/file.txt"

#define DATA_STRING "the quick brown fox jumped over the lazy dog."
#define RIGHT_MD5_HASH "bb0fa6eff92c305f166803b6938dd33a"

/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
static INT32 create_test_file(const char *filename)
{
  INT32 fs_res;
  INT32 fd = -1;

  fd = m2mb_fs_open(filename,
              M2MB_O_CREAT | M2MB_O_WRONLY  /*Create file if not existing, open in write only mode*/,
              M2MB_ALLPERMS /*0777*/
              );
    if (fd == -1 )
    {
      AZX_LOG_ERROR("Cannot open file\r\n");
      return -1;
    }

    fs_res = m2mb_fs_write(fd, DATA_STRING, strlen(DATA_STRING));
    if (fs_res != (INT32)strlen(DATA_STRING))
    {
      AZX_LOG_ERROR("Failed writing buffer into file.\r\n");
      return -1;
    }
    else
    {
      AZX_LOG_INFO("Buffer written successfully into file. %d bytes were written.\r\n", fs_res);
    }

    AZX_LOG_TRACE("Closing file.\r\n");
    m2mb_fs_close(fd);
    return 0;
}

static void clear_test_file(const char *filename)
{
  m2mb_fs_unlink(filename);
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

  UINT8 computedHash[16] = {0};
  char hashString[33] = {0};
  INT32 res;

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting MD5 demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);


  create_test_file(FILE);

  AZX_LOG_INFO("\r\nComputing hash from file...\r\n");
  res = md5_computeFromFile(FILE, computedHash);
  if( res != 0 )
  {
    AZX_LOG_ERROR("Cannot compute hash from file! res: %d\r\n", res);
  }

  AZX_LOG_INFO("Computed hash: %s\r\n"
      "Expected hash: %s\r\n",
      md5_hashToString(computedHash, hashString),
      RIGHT_MD5_HASH);

  if (md5_compareHashWithString(computedHash, sizeof(computedHash), (CHAR *) RIGHT_MD5_HASH))
  {
    AZX_LOG_INFO("Hashes are the same!\r\n");
  }
  else
  {
    AZX_LOG_ERROR("Wrong hash!\r\n");
  }

  clear_test_file(FILE);

  AZX_LOG_INFO("\r\nComputing hash from string...\r\n");

  res = md5_computeSum(computedHash, (UINT8*)DATA_STRING, strlen(DATA_STRING));
  if( res != 0 )
  {
    AZX_LOG_ERROR("Cannot compute hash from string! res: %d\r\n", res);
  }

  AZX_LOG_INFO("Computed hash: %s\r\n"
      "Expected hash: %s\r\n",
      md5_hashToString(computedHash, hashString),
      RIGHT_MD5_HASH);
  if (md5_compareHashWithString(computedHash, sizeof(computedHash), (CHAR *) RIGHT_MD5_HASH))
  {
    AZX_LOG_INFO("Hashes are the same!\r\n");
  }
  else
  {
    AZX_LOG_ERROR("Wrong hash!\r\n");
  }
}

