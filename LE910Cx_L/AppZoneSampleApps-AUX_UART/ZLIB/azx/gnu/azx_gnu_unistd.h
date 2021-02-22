/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 azx_gnu_unistd.h

 @brief
 gnu definition

 @details
 Porting from gnu to azx

 @note

 @author Fabio Pintus
 @author Norman Argiolas

 @date
 11/02/2020
 */

#ifndef HDR_AZX_GNU_UNISTD_H_
#define HDR_AZX_GNU_UNISTD_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Include files ================================================================================*/
#ifndef _ARMABI
#include <unistd.h>
#endif

#include "m2mb_types.h"
#include "m2mb_fs_posix.h"

/* Global defines ================================================================================*/

#define read m2mb_fs_read
#define write m2mb_fs_write
#define close m2mb_fs_close

#undef chown
#define chown azx_gnu_chown

#undef unlink
#define unlink m2mb_fs_unlink

#undef lseek
#define lseek m2mb_fs_lseek

/* Function prototypes ====================================================================*/

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    chown

  @details
    chown not supported

  @param[in]

  @return

  @note
    chown not supported

  @b
    Example
  @code
    <C code example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
int azx_gnu_chown(const char *__path, uid_t __owner, gid_t __group);

#ifdef __cplusplus
}
#endif

#endif /* HDR_AZX_GNU_UNISTD_H_ */
