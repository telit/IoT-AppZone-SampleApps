/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    app_cfg.h

  @brief Application configuration settings conveniently located here.

  @note
    Dependencies:
       m2mb_types.h
       azx_log.h

  @author

*/

#ifndef HDR_SAMPLE_APP_CFG_H_
#define HDR_SAMPLE_APP_CFG_H_



/**
 * @brief The current version of the Samples
 */
#define VERSION "1.0.13-C1"

/**
 * @name Log Settings
 * @{
 */
#define LOG_LEVEL AZX_LOG_LEVEL_DEBUG  /**< How detailed should the logs be. See @ref AZX_LOG_LEVEL_E */
#define LOG_CHANNEL AZX_LOG_TO_USB0
#define LOG_WITH_COLOUR 0 /**< Set to 1 to add ANSI colours to the logs*/


/**
 * @brief Call this at your AZ entry point to easily configure logging
 */
#define AZX_LOG_INIT() do {\
  AZX_LOG_CFG_T cfg =\
  {\
    /*.log_level*/   LOG_LEVEL,\
    /*.log_channel*/ LOG_CHANNEL,\
    /*.log_colours*/ LOG_WITH_COLOUR\
  };\
  azx_log_init(&cfg);\
} while(0)


/** \addtogroup  logUsage
@{ */

/* LOGGING MACROS */
/** \name Public log Macros
 * \brief These function-like macros can be used to print different messages with different log levels
 *  @{ */
 

#define AZX_LOG_CRITICAL(a...)  azx_log_formatted(AZX_LOG_LEVEL_CRITICAL, __FUNCTION__, __FILE__, __LINE__, a)
/**<Prints a critical error message.*/

#define AZX_LOG_ERROR(a...)     azx_log_formatted(AZX_LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, a)
/**<Prints an error message.*/

#define AZX_LOG_WARN(a...)      azx_log_formatted(AZX_LOG_LEVEL_WARN, __FUNCTION__, __FILE__, __LINE__, a)
/**<Prints a warning message.*/

#define AZX_LOG_INFO(a...)      azx_log_formatted(AZX_LOG_LEVEL_INFO, "", "", 0, a)
/**<Prints an informative message.*/

#define AZX_LOG_DEBUG(a...)     azx_log_formatted(AZX_LOG_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, a)
/**<Prints a debug message.*/

#define AZX_LOG_TRACE(a...)     azx_log_formatted(AZX_LOG_LEVEL_TRACE, __FUNCTION__, __FILE__, __LINE__, a)
/**<Prints a trace level message.*/

/** @} */
/** @} */  //close addtogroup




#endif /* HDR_SAMPLE_APP_CFG_H_ */
