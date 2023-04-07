/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 	 azx_gnu_fcntl.h

 @brief
 	 gnu definition

 @details
 	 Porting from gnu to azx

 @note
 	 Dependencies:
 	 	 m2mb_types.h
 	 	 m2mb_fs_posix.h

 @author Moreno Floris
 @author Norman Argiolas

 @date
 	 11/02/2020
 */

#ifndef HDR_AZX_GNU_FCNTL_H_
#define HDR_AZX_GNU_FCNTL_H_

/* Include files ================================================================================*/

#include "m2mb_types.h"
#include "m2mb_fs_posix.h"

/* Global defines ================================================================================*/
#undef O_RDONLY
#define O_RDONLY M2MB_O_RDONLY
#undef O_WRONLY
#define O_WRONLY M2MB_O_WRONLY
#undef O_RDWR
#define O_RDWR M2MB_O_RDWR
#undef O_CREAT
#define O_CREAT M2MB_O_CREAT
#undef O_EXCL
#define O_EXCL M2MB_O_EXCL
#undef O_NOCTTY
#define O_NOCTTY M2MB_O_NOCTTY
#undef O_TRUNC
#define O_TRUNC M2MB_O_TRUNC
#undef O_APPEND
#define O_APPEND M2MB_O_APPEND

#define open m2mb_fs_open

#endif /* HDR_AZX_GNU_FCNTL_H_ */
