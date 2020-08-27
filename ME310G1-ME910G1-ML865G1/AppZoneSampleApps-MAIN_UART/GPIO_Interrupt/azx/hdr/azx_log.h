/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

#ifndef HDR_AZX_LOG_H_
#define HDR_AZX_LOG_H_
/**
 * @file azx_log.h
 * @version 1.0.5
 * @dependencies 
 * @author Fabio Pintus
 * @author Ioannis Demetriou
 * @date 13/01/2017
 *
 * @brief Logging utilities to print on available output channels
 *
 * This library code will give user the possibility to print debug messages on
 * their application.
 */
#include "m2mb_types.h"
#include "app_cfg.h"

/* Global declarations =======================================================*/


/* Global typedefs ===========================================================*/

/** @defgroup logConf Configuration of logging functionalities
 * Configuration functions, macros and enumerators can be used to set the
 * logging channel (Main UART, Auxiliary UART, USB0/ USB1) and the logging mask
 * (which allows to enable each log level independently)
 */

/** @defgroup logUsage Usage of logging functionalities
 * Macros that can be used to actually print log messages
 */

/**
 * @brief Logging levels
 * \ingroup logConf
 */
typedef enum
{
  AZX_LOG_LEVEL_TRACE    = 1,    /**< Prints every message, adds a "TRACE" at the beginning of the message*/
  AZX_LOG_LEVEL_DEBUG    = 2,    /**< Prints most of the messages, adds a "DEBUG" at the beginning of the message*/
  AZX_LOG_LEVEL_INFO     = 3,    /**< print the message only, without any additional info */
  AZX_LOG_LEVEL_WARN     = 4,    /**< like DEBUG, but adds a "WARNING" at the beginning of the message. */
  AZX_LOG_LEVEL_ERROR    = 5,    /**< like DEBUG, but adds an "ERROR" at the beginning of the message */
  AZX_LOG_LEVEL_CRITICAL = 6,    /**< like DEBUG, but adds an "CRITICAL" at the beginning of the message */
  AZX_LOG_LEVEL_NONE     = 0x7F, /**< Do not print anything */
} AZX_LOG_LEVEL_E;


/**
 * @brief Logging errors
 * \ingroup logUsage
 */
typedef enum
{
  AZX_LOG_NOT_INIT,                  /**<Trying to use log functions before azx_log_init()*/
  AZX_LOG_USB_CABLE_UNPLUGGED,       /**<USB cable is not plugged when trying to print on USB*/
  AZX_LOG_NO_USB_INSTANCE_AVAILABLE, /**<No more USB instances are available*/
  AZX_LOG_CANNOT_OPEN_USB_CHANNEL,   /**<Cannot open the specified USB channel*/

  AZX_LOG_MAX_ERROR_LEN
} AZX_LOG_ERRORS_E;

/** @private */
typedef INT32 (*_azx_log_function)(const char *, void*);


/**
 * @brief Logging output channels
 * \ingroup logConf
 */
typedef enum
{
  AZX_LOG_TO_MAIN_UART, /**<Print on Main UART*/
  AZX_LOG_TO_AUX_UART,  /**<Print on AUX UART*/
  AZX_LOG_TO_USB0,      /**<Print on USB0 port*/
  AZX_LOG_TO_USB1,      /**<Print on USB1 port*/

  AZX_LOG_TO_MAX
} AZX_LOG_HANDLE_E;


/**
 * @brief Logging configuration structure
 *
 * This structure holds the parameters to be passed to azx_log_init()
 */
typedef struct
{
  AZX_LOG_LEVEL_E log_level;    /**<The log level to be enabled (see Macros)*/
  AZX_LOG_HANDLE_E log_channel; /**<The output channel */
  BOOLEAN log_colours;          /**<Defines if output should use colours or not*/
}AZX_LOG_CFG_T;


/* Global functions ==========================================================*/

/*INTERNAL FUNCTION, used by public macros*/
/**
 * @brief Prints a message to the selected channel up to the specified log level
 * @private
 *
 * This function will try to output the provided message (in the same format as
 * `printf`) to the selected output channel.
 *
 * If the specified log level bit is not greater than the one specified by
 * @ref AZX_LOG_LEVEL_E, the function will return without printing anything.
 * Depending on log level, additional information will be printed.
 *
 * **Example**
 *
 *     azx_log_formatted(AZX_LOG_LEVEL_INFO,__FUNCTION__, __FILE__, __LINE__,
 *         "my message is %s", "hello world!");
 *
 * @warning This is an internal function, and should not be used directly by user code.
 *
 * @param[in] level Log level bit of the specific message
 * @param[in] function Name of the calling function
 * @param[in] file Name of the originating source file
 * @param[in] line Line number in the source file of the calling function
 * @param[in] fmt The message format - the same accepted by `printf`
 *
 * @return Number of bytes written, 0 if the log level is disabled
 *
 * @see azx_log_init()
 * @see AZX_LOG_LEVEL_E
 */
INT32 azx_log_formatted(AZX_LOG_LEVEL_E level,
    const CHAR *function, const CHAR *file, int line, const CHAR *fmt, ... );

/* Public functions ==========================================================*/

/**
 * @brief Initializes the log functionality
 *
 * This function initializes the log functionality with the provided
 * configuration structure. Call this before any `AZX_LOG_*` are used.
 *
 * It is recommended to use the macro @ref AZX_LOG_INIT.
 *
 * @param[in] cfg Pointer to the structure with the configuration parameters
 *
 * @return None
 *
 * **Sample usage**
 *
 *     AZX_LOG_CFG_T cfg = {
 *        .log_level = AZX_LOG_LEVEL_DEBUG,
 *        .log_channel = AZX_LOG_TO_MAIN_UART,
 *        .log_colours = FALSE
 *     };
 *
 *     azx_log_init(&cfg);  //initialize log system
 *
 * \ingroup logConf
 *
 * @see LOGGING_INIT
 */
void azx_log_init(AZX_LOG_CFG_T *cfg);

/**
 * @brief Deinitializes the log functionality
 *
 * This function deinitializes the log functionality. After it is called, the
 * `AZX_LOG_*` macros will not work.
 *
 * @return One of
 *     @ref AZX_LOG_NOT_INIT - The azx_log_init() function was not call previously
 *     0 - Success
 *
 * @see azx_log_init
 *
 * **Example**
 *
 *     azx_log_deinit();
 */
INT32 azx_log_deinit(void);

/**
 * @brief Sets the new log level to be used.
 *
 * This function sets the new log level that will be used by the log system.
 *
 * **Example**
 *
 *     azx_log_setLevel(AZX_LOG_LEVEL_NONE);
 *
 * @param[in] level The new log level to be set
 *
 * @see azx_log_init()
 */
void azx_log_setLevel(AZX_LOG_LEVEL_E level);

/**
 * @brief Returns the current value of the log level.
 *
 * This function gets the current log level value.
 *
 * @return The log level value
 *
 * @see azx_log_init()
 *
 * **Example**
 *
 *     AZX_LOG_LEVEL_E level = azx_log_get_log_level();
 */
AZX_LOG_LEVEL_E azx_log_getLevel(void);

/**
 * @brief Gets the logging component to output to a file.
 *
 * Once this is called all the logs will be sent to a named file. The file will be appended to, so
 * existing logs there will not be removed.
 *
 * Logs will continue to be sent to USB/UART if that is so configured.
 *
 * Only one file can be used at the same time, so calling this again with a new filename means that
 * all logs will go to the new file instead of the old one.
 *
 * The logging can be configured to be done in a circular way by setting circular_chunks to a value
 * greater than 0. Each chunk will have at most max_size_kb KB.
 *
 * @param filename The name of the file to log to. If NULL, this function does nothing.
 * @param circular_chunks The number of chunks to store circularly (apart from the original one).
 * @param min_level The minimum level of the logs to be stored.
 * @param max_size_kb The maximum size in KB of each size of the log file. Once the file reaches
 * that limit, no further logging will be made to it.
 *
 * @return TRUE if the file can be created and opened, FALSE otherwise
 */
BOOLEAN azx_log_send_to_file(const CHAR* filename, UINT32 circular_chunks,
    AZX_LOG_LEVEL_E min_level, UINT32 max_size_kb);

/**
 * @brief Flushes any outstanding logs to the file.
 *
 * Without calling this, there is no guarantee of when the logs will be written to the filesystem.
 * It may take longer due to caching.
 */
void azx_log_flush_to_file(void);

#endif /* HDR_AZX_LOG_H_ */
