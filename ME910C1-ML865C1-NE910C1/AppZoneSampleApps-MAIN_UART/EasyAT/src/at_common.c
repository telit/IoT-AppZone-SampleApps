/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    at_common.c

  @brief
    The file contains the common functions for custom commands

  @author


  @date
    13/03/2020
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os.h"
#include "m2mb_os_sem.h"

#include "m2mb_pdp.h"

#include "m2mb_atp.h"

#include "azx_log.h"


#include "azx_easy_at.h"

#include "at_common.h"

#include "at_hash_MYCMD.h"
#include "at_hash_MYINPUT.h"



static AZX_EASY_AT_MODULE_T *g_at_module = NULL;




void set_at_module( AZX_EASY_AT_MODULE_T *module )
{
  g_at_module = module;
}

AZX_EASY_AT_MODULE_T *get_at_module( void )
{
  return g_at_module;
}



INT32 my_cmds_at_init( void )
{
  AZX_EASY_AT_MODULE_T *module;
  AZX_EASY_AT_ATCOMMAND_T my_commands_List[] =
  {
    { "#MYCMD",   M2MB_ATP_NORML | M2MB_ATP_NOPIN | M2MB_ATP_NOSIM, MYCMD_AT_Callback, NULL, 0},
    { "#MYINPUT",   M2MB_ATP_NORML | M2MB_ATP_NOPIN | M2MB_ATP_NOSIM, MYINPUT_AT_Callback, MYINPUT_INPUT_AT_Callback, 0}
  };
  module = azx_easy_at_init( ( CHAR * ) CMD_MODULE_NAME, my_commands_List,
                             AZX_EASY_AT_ARRAY_SIZE( my_commands_List ) );

  if( ! module )
  {
    return -1;
  }

  set_at_module( module );
  return 0;
}
