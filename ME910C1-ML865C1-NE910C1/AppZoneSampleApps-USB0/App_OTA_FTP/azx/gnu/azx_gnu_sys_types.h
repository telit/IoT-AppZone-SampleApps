/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 	 azx_gnu_sys_types.h

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

#ifndef HDR_AZX_GNU_SYS_TYPES_H_
#define HDR_AZX_GNU_SYS_TYPES_H_

/* Global defines ================================================================================*/

	#undef	fd_set
	#define	fd_set		M2MB_SOCKET_BSD_FD_SET_T
	#undef	fds_bits
	#define	fds_bits	fd_array

#endif /* HDR_AZX_GNU_SYS_TYPES_H_ */
