/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

#ifndef HDR_APP_CFG_H_
#define HDR_APP_CFG_H_
/**
 * @file app_cfg.h
 * @version 1.0.0
 * @date 10/02/2019
 *
 * @brief Application configuration settings conveniently located here.
 *
 * This file contains macros that a programmer can alter to easily modify the
 * behaviour of the application an **compile** time.
 */

/** @cond DEV*/
#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)
/** @endcond*/

/*Local basepath for samples that need local files usage*/
#define LOCALPATH "/data/azc/mod"

#endif /* HDR_APP_CFG_H_ */
