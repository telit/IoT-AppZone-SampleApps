/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
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

/* Include files ================================================================================*/
#ifdef __cplusplus
extern "C"
{
#endif
#ifndef _ARMABI
#ifndef M2M_M2MB_FS_STDIO_H
#include <unistd.h>
#endif
#endif


#undef S_ISUID
#undef S_ISGID
#undef S_ISVTX
#ifndef _ARMABI
#include <sys/types.h>
#endif

#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_fs_posix.h"


/* Global defines ================================================================================*/

#define read m2mb_fs_read
#define write m2mb_fs_write
#define close m2mb_fs_close

#undef chown
#define chown azx_gnu_chown

#ifndef M2M_M2MB_FS_STDIO_H
#undef unlink
#define unlink m2mb_fs_unlink
#endif

#undef lseek
#define lseek m2mb_fs_lseek

#undef sleep
#define sleep azx_gnu_sleep

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

unsigned azx_gnu_sleep(unsigned int seconds);

#ifdef __cplusplus
}
#endif

#endif /* HDR_AZX_GNU_UNISTD_H_ */
