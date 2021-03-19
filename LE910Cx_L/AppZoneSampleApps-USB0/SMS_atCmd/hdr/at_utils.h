/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*
 * at_utils.h
 *
 *  Created on: 18 lug 2019
 *      Author: FabioPi
 */

#ifndef HDR_AT_UTILS_H_
#define HDR_AT_UTILS_H_


/*Async mode (with callback)*/
M2MB_RESULT_E at_cmd_async_init(INT16 instance);
M2MB_RESULT_E at_cmd_async_deinit(INT16 instance);
M2MB_RESULT_E send_async_at_command(INT16 instance, const CHAR *atCmd, CHAR *atRsp, UINT32 atRspMaxLen);

/*Sync mode (without callback)*/
M2MB_RESULT_E at_cmd_sync_init(INT16 instance);
M2MB_RESULT_E at_cmd_sync_deinit(INT16 instance);
M2MB_RESULT_E send_sync_at_command(INT16 instance, const CHAR *atCmd, CHAR *atRsp, UINT32 atRspMaxLen);


#endif /* HDR_AT_UTILS_H_ */
