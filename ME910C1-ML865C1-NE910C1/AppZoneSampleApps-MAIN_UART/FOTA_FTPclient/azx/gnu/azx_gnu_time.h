/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 azx_gnu_time.h

 @brief
 gnu definition

 @details
 Porting from gnu to azx

 @note
 Dependencies:
 m2mb_types.h
 m2mb_os_api.h
 time.h

 @author Cristina Desogus

 @date
 11/08/2022
 */

#ifndef HDR_AZX_GNU_TIME_H_
#define HDR_AZX_GNU_TIME_H_


#ifdef __cplusplus
extern "C"
{
#endif

/* Global defines ================================================================================*/

#undef time
#define time(a) 		azx_gnu_time(a)


/* Function prototypes ====================================================================*/

/*-----------------------------------------------------------------------------------------------*/
/**

 @brief
 Returns a timestamp from unix epoch


 @param [in] timer: time_t pointer

 @return
 0 on SUCCESS
 -1 on FAILURE


 */
/*-----------------------------------------------------------------------------------------------*/
time_t azx_gnu_time(time_t *_timer);



#ifdef __cplusplus
}
#endif

#endif /* HDR_AZX_GNU_TIME_H_ */
