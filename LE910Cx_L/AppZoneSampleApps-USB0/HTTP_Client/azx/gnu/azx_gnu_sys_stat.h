/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 azx_gnu_sys_time.h

 @brief
 gnu definition

 @details
 Porting from gnu to azx

 @note

 @author Moreno Floris
 @author Norman Argiolas

 @date
 11/02/2020
 */

#ifndef HDR_AZX_GNU_SYS_STAT_H_
#define HDR_AZX_GNU_SYS_STAT_H_

#undef S_ISUID
#undef S_ISGID
#undef S_ISVTX

#include "m2mb_fs_posix.h"


/* Global defines ================================================================================*/

#define	stat		m2mb_fs_stat
#define	fstat		m2mb_fs_fstat


/* Function prototypes ====================================================================*/


#endif /* HDR_AZX_GNU_SYS_STAT_H_ */
