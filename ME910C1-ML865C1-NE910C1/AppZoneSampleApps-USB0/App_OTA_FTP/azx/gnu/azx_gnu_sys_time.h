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

#ifndef HDR_AZX_GNU_SYS_TIME_H_
#define HDR_AZX_GNU_SYS_TIME_H_

/* Global defines ================================================================================*/

#define	timeval 		M2MB_SOCKET_BSD_TIMEVAL
#define	tv_sec 			m_tv_sec
#define	tv_usec 		m_tv_usec

/**\name timerisset
 * \brief Convenience macros for operations on timevals
 * \note `timercmp' does not work for >= or <=
 *    @{
 **/
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timerclear(tvp)		((tvp)->tv_sec = (tvp)->tv_usec = 0)

#define	timercmp(a, b, CMP) 			\
	(((a)->tv_sec == (b)->tv_sec) ?  	\
	((a)->tv_usec CMP (b)->tv_usec) : 	\
	((a)->tv_sec CMP (b)->tv_sec))

#define gettimeofday(a,b) m2m_gettimeofday(a,b)

/**   @} */

/* Function prototypes ====================================================================*/

int m2m_gettimeofday(struct timeval *tv, struct timezone *tz);

#endif /* HDR_AZX_GNU_SYS_TIME_H_ */
