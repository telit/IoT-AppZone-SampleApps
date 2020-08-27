/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    Log.h

  @brief
    C++ Logging utilities to print on available output channels.

  @details
    This library code will give user the possibility to print debug messages on their application.
    A namespace M2MLog is defined in this header file.

  @note
    Dependencies:
    m2mb_types.h

  @author
    Fabio Pintus

  @date
    11/09/2019

  @version
    1.0.2
 */


#ifndef HDR_LOG_H_
#define HDR_LOG_H_

namespace M2MLog
{

/* Global declarations ==========================================================================*/
#ifndef LOG_NO_LOGS
  #ifndef LOG_NO_COLOURS
  #define LOG_COLOURED_LOGS
  #endif
#endif


/* Global typedefs ==============================================================================*/

/** \defgroup logConf Configuration of logging functionalities
 * Configuration functions, macros and enumerators can be used to set the logging channel (Main UART, Auxiliary UART, USB0/ USB1) and the logging mask (which allows to enable each log level independently)
 */

/** \defgroup logUsage Usage of logging functionalities
 *  Macros that can be used to actually print log messages
 */

/**
 * \brief Logging levels
 * \ingroup logConf
 */
typedef enum
{
  NOT_INIT_ERROR,          /**<Trying to use log functions before m2m_log_init()*/
  USB_CABLE_NOT_PLUG,      /**<USB cable is not plugged when trying to print on USB*/
  NO_MORE_USB_INSTANCE,    /**<No more USB instances are available*/
  CANNOT_OPEN_USB_CH,      /**<Cannot open the specified USB channel*/

  MAX_ERROR_LEN
} M2M_LOG_ERRORS_E;


/**
 * \brief Logging errors
 * \ingroup logUsage
 */
typedef enum
{
  LOG_LEVEL_NONE     = 0, /**< Do not print anything */
  LOG_LEVEL_TRACE    = 1, /**< Prints every message, adds a "TRACE" at the beginning of the message*/
  LOG_LEVEL_DEBUG    = 2, /**< Prints most of the messages, adds a "DEBUG" at the beginning of the message*/
  LOG_LEVEL_INFO     = 3, /**< print the message only, without any additional info */
  LOG_LEVEL_WARN     = 4, /**< like DEBUG, but adds a "WARNING" at the beginning of the message. */
  LOG_LEVEL_ERROR    = 5, /**< like DEBUG, but adds an "ERROR" at the beginning of the message */
  LOG_LEVEL_CRITICAL = 6, /**< like DEBUG, but adds an "CRITICAL" at the beginning of the message */
} M2M_LOG_LEVEL_E;


/**
 * \brief Logging output channels
 * \ingroup logConf
 */
typedef enum
{
  LOG_TO_MAIN_UART, /**<Print on Main UART*/
  LOG_TO_AUX_UART,  /**<Print on AUX UART*/
  LOG_TO_USB0,      /**<Print on USB0 port*/
  LOG_TO_USB1,      /**<Print on USB1 port*/

  LOG_TO_MAX
} M2M_LOG_HANDLE_E;


class Logger
{
public:
  /* Global functions =============================================================================*/
  /*-----------------------------------------------------------------------------------------------*/
  /**
    \brief
      Constructor of the Logger class

    \details
      This method initializes the class with the provided parameters. after init, methods like
      critical()
      error()
      warn()
      info()
      debug()
      trace()
      can be used

    \param[in] level
      log level to be set
    \param[in] channel
      log channel where prints will output

    \return
      A Logger class instance

    <b>Sample usage</b>
    \code
    using namespace M2MLog
      Logger log(LOG_LEVEL_DEBUG, LOG_TO_UART_MAIN);
      log.info("hello world");
    \endcode

    \ingroup logConf
   */
  /*-----------------------------------------------------------------------------------------------*/
  Logger(M2M_LOG_LEVEL_E level, M2M_LOG_HANDLE_E channel);


  /*-----------------------------------------------------------------------------------------------*/
    /**
      \brief
        Constructor overloading of the Logger class

      \details
        This method initializes the class with the provided parameters
      \param[in] level
        log level to be set
      \param[in] channel
        log channel where prints will output
      \param[in] colours
        if true, coloured logs will be printed

      \return
        A Logger class instance

      <b>Sample usage</b>
      \code
      using namespace M2MLog
        Logger log(LOG_LEVEL_DEBUG, LOG_TO_UART_MAIN, true);
        log.info("hello world");
      \endcode

      \ingroup logConf
     */
    /*-----------------------------------------------------------------------------------------------*/
  Logger(M2M_LOG_LEVEL_E level, M2M_LOG_HANDLE_E channel, bool colours);


  /*-----------------------------------------------------------------------------------------------*/
    /**
      \brief
        Destructor of the Logger class

      \details
        This method deinitializes the class
      \return
            None

      \ingroup logConf
     */
    /*-----------------------------------------------------------------------------------------------*/
  ~Logger(void);


  /*-----------------------------------------------------------------------------------------------*/
  /**
    \brief Prints on the defined stream (UART or USB channel)
   *
    \param [in] level:    Logging level. see M2M_LOG_LEVEL_E enum
    \param [in] function:   source function name to add to the output if log is verbose
    \param [in] file:     source file path to add to the output if log is verbose
    \param [in] line:     source file line to add to the output if log is verbose
    \param [in] fmt :     string format with parameters to print
    \param [in] ... :     ...
    \return the number of sent bytes. 0 if logging level is not enabled, negative in case of error
   *
   */
  /*-----------------------------------------------------------------------------------------------*/
  INT32 out_format( M2M_LOG_LEVEL_E level, const char* function, const char* file, int line, const CHAR *fmt, ... );

  /** \addtogroup  logUsage
  @{ */

  /* PUBLIC MACROS */
  /** \name Public log Macros
   * \brief These function-like macros can be used to print different messages with different log levels
   *  @{ */
  #define critical(a...)  out_format(LOG_LEVEL_CRITICAL, __FUNCTION__, __FILE__, __LINE__, a)  /**<Prints a critical error message.*/
  #define error(a...)     out_format(LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, a)     /**<Prints an error message.*/
  #define warn(a...)      out_format(LOG_LEVEL_WARN, __FUNCTION__, __FILE__, __LINE__, a)      /**<Prints a warning message.*/
  #define info(a...)      out_format(LOG_LEVEL_INFO, "", "", 0, a)                             /**<Prints an informative message.*/
  #define debug(a...)     out_format(LOG_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, a)     /**<Prints a debug message.*/
  #define trace(a...)     out_format(LOG_LEVEL_TRACE, __FUNCTION__, __FILE__, __LINE__, a)     /**<Prints a trace level message.*/
  /** @} */
  /** @} */  //close addtogroup

private:
  bool isInit = false;                                  /**<Flag for class initialization*/
  bool colouredLogs = false;                            /**<Flag for colour usage*/
  M2M_LOG_LEVEL_E   int_level = LOG_LEVEL_NONE;         /**<The log level to be used.*/
  M2M_LOG_HANDLE_E  int_channel = LOG_TO_MAX;           /**<The output channel */

  M2MB_OS_SEM_HANDLE CSSemHandle;                       /**<Critical section semaphore handle*/


  INT32 USIF0_fd = -1;                                  /**<Main UART file descriptor*/
  INT32 USIF1_fd = -1;                                  /**<Aux UART file descriptor*/
  INT32 USB0fd = -1;                                    /**<USB0 file descriptor*/
  INT32 USB1fd = -1;                                    /**<USB1 file descriptor*/


  /*-----------------------------------------------------------------------------------------------*/
  /**
    \brief returns the system uptime in milliseconds

    \return the system uptime

   */
  /*-----------------------------------------------------------------------------------------------*/
  UINT32 Uptime(void);

  /*-----------------------------------------------------------------------------------------------*/
  /**
    \brief Prints on the requested log channel (USB, UART, AUX)

    \param [in] message:    message to be printed on output
    \return : amount of printed bytes, negative value in case of error

   */
  /*-----------------------------------------------------------------------------------------------*/
  INT32 out(const CHAR* message);


  /*-----------------------------------------------------------------------------------------------*/
  /**
    \brief Print directly on the main UART

    \param [in] message: the string to print
    \return sent bytes

   */
  /*-----------------------------------------------------------------------------------------------*/
  INT32 out_toM_UART(const CHAR *message);

  /*-----------------------------------------------------------------------------------------------*/
  /**
    \brief Print directly on the auxiliary UART

    \param [in] message: the string to print
    \return sent bytes

   */
  /*-----------------------------------------------------------------------------------------------*/
  INT32 out_toA_UART(const CHAR *message);

  /*-----------------------------------------------------------------------------------------------*/
  /**
    \brief Prints as out_toM_UART but using a specified USB channel

    \param [in] channel:  USB resource path where to print (e.g. 0 for USB0, 1 for USB1)
    \param [in] message : Message to print
    \return sent bytes, negative in case of error

   */
  /*-----------------------------------------------------------------------------------------------*/
  INT32 out_toUSB(UINT8 channel, const CHAR *message);

  /*-----------------------------------------------------------------------------------------------*/
  /**
    \brief Removes the file path from the provided path, leaving only filename

    \param [in] path: the file path
    \return the filename(+ extension) extracted from the path

   */
  /*-----------------------------------------------------------------------------------------------*/
  const char *get_fileName(const CHAR* path);

  /*-----------------------------------------------------------------------------------------------*/
  /**
    \brief Returns the current task name

    \param [in] name: the buffer where the task name will be saved
    \return a reference to name variable

   */
  /*-----------------------------------------------------------------------------------------------*/
  char *get_taskName(CHAR *name);
};

}

#endif /* HDR_LOG_H_ */
