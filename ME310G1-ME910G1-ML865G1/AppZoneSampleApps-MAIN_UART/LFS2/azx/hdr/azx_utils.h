/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

#ifndef HDR_M2M_UTILS_H_
#define HDR_M2M_UTILS_H_
/**
 * @file azx_utils.h
 * @version 1.0.3
 * @dependencies core/azx_log
 * @author Ioannis Demetriou
 * @author Sorin Basca
 * @date 10/02/2019
 *
 * @brief Various helpful utilities
 */
#include "m2mb_types.h"
#include "azx_log.h"


/**
 * @brief Limits a value to a certain interval
 *
 * This is defined as a macro so it works with any types that support ordering.
 *
 * @param val The value to limit
 * @param min The minimum allowed value
 * @param max The maximum allowed value
 *
 * @return The original value if it was inside the limits, or min, or max, if it was less than, or
 * greater than the limits.
 */
#define AZX_LIMIT(val, min, max) (val = (val < (min) ? (min) : (val > (max) ? (max) : val))) 

/**
 * @brief Puts the task to sleep for a specified time.
 *
 * This function returns once the specified number of milliseconds elapses.
 *
 * @param[in] ms The number of milliseconds to wait before returning.
 */
void azx_sleep_ms(UINT32 ms);

/*  @{ */
#define AZX_UTILS_HEX_DUMP_BUFFER_SIZE 250
/**
 * @brief Dumps HEX data to string
 *
 * Useful for logging binary data. If the data gets trimmed, adjust
 * @ref AZX_UTILS_HEX_DUMP_BUFFER_SIZE in the c file.
 *
 * @param[in] data The data to print in hex format
 * @param[in] len The number of bytes
 *
 * @return A string with the hex data. The string is valid until the subsequent
 *     call to azx_hex_dump().
 */
const CHAR* azx_hex_dump(const void* data, UINT32 len);
/** @} */

/**
  @brief Reboots the modem straight away.

  @details
    This utility will ask the module to reboot. It will not be executed immediately, so
    code flow must take care of instructions after this one.

  @return None

*/
/*-----------------------------------------------------------------------------------------------*/
void azx_reboot_now(void);

/**
  @brief Shuts the modem down straight away.

  @details
    This utility will ask the module to shutdown. It will not be executed immediately, so
    code flow must take care of instructions after this one.

  @return None

*/
/*-----------------------------------------------------------------------------------------------*/
void azx_shutdown_now(void);




#endif /* HDR_AZX_UTILS_H_ */
