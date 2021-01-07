/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    at_common.h

  @brief
    The file contains common functions for custom commands

  @author


  @date
    13/03/2020
*/

#ifndef HDR_AT_COMMON_H_
#define HDR_AT_COMMON_H_


#define CMD_MODULE_NAME "MYCMDS"

void set_at_module( AZX_EASY_AT_MODULE_T *module );
AZX_EASY_AT_MODULE_T *get_at_module( void );

INT32 my_cmds_at_init( void );



#endif /* HDR_AT_COMMON_H_ */
