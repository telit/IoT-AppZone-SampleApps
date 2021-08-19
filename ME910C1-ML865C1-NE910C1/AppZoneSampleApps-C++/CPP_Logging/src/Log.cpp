/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    Log.cpp

  @brief
    The file contains the Log class implementation

  @details
    

  @note
    

  @author
    

  @date
    11/09/2019
*/
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "m2mb_usb.h"
#include "m2mb_uart.h"

#include "Log.h"

namespace M2MLog
{

/* Local defines ================================================================================*/
#define LOG_BUFFER_SIZE 2048

#define NO_COLOUR "\033[0m"
#define BOLD      "\033[1m"
#define DARK      "\033[2m"
#define UNDERLINE "\033[4m"
#define BLINK     "\033[5m"
#define REVERSE   "\033[7m"
#define CONCEALED "\033[8m"

#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define ON_BLACK   "\033[40m"
#define ON_RED     "\033[41m"
#define ON_GREEN   "\033[42m"
#define ON_YELLOW  "\033[43m"
#define ON_BLUE    "\033[44m"
#define ON_MAGENTA "\033[45m"
#define ON_CYAN    "\033[46m"
#define ON_WHITE   "\033[47m"

#define LOG_TRACE_COLOR    CYAN
#define LOG_DEBUG_COLOR    YELLOW
#define LOG_INFO_COLOR     BOLD WHITE
#define LOG_WARN_COLOR     BOLD YELLOW
#define LOG_ERROR_COLOR    BOLD RED
#define LOG_CRITICAL_COLOR BOLD RED ON_WHITE


#define LOG_PREFIX(tag) \
  case LOG_LEVEL_##tag: \
    offset = snprintf(log_buffer, LOG_BUFFER_SIZE, \
      (colouredLogs)? g_prefix_fmt_colour : g_prefix_fmt_no_colour, \
      (colouredLogs)? LOG_##tag##_COLOR: "", #tag, (colouredLogs)? NO_COLOUR :"", \
      now / 1000.0, \
      get_fileName(file), line, \
      function, \
      get_taskName(task_name) \
      ); \
    break

/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
static CHAR log_buffer[LOG_BUFFER_SIZE] = { 0 };
static CHAR task_name[64];

static const CHAR* g_prefix_fmt_colour = "[%s%-5s%s] %3.2f  " CYAN "%s" NO_COLOUR
        ":" BOLD CYAN "%d" NO_COLOUR
        " - %s{" BOLD WHITE "%s" NO_COLOUR "}$ ";
static const CHAR* g_prefix_fmt_no_colour = "[%s%-5s%s] %3.2f  %s:%d - %s{%s}$ ";

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/*Private methods*/

/*-----------------------------------------------------------------------------------------------*/
/**
  \brief returns the system uptime in milliseconds

  \return the system uptime

 */
/*-----------------------------------------------------------------------------------------------*/
UINT32 Logger::Uptime(void)
{

  UINT32 sysTicks = m2mb_os_getSysTicks();

  FLOAT32 ms_per_tick = m2mb_os_getSysTickDuration_ms();

  return (UINT32) (sysTicks * ms_per_tick); //milliseconds
}


/*-----------------------------------------------------------------------------------------------*/
/**
  \brief Prints on the requested log channel (USB, UART, AUX)

  \param [in] message:    message to be printed on output
  \return : amount of printed bytes, negative value in case of error

 */
/*-----------------------------------------------------------------------------------------------*/
INT32 Logger::out(const CHAR* message)
{
  INT32 result = -1;

  if ( ! isInit)
  {
    return NOT_INIT_ERROR;
  }

  switch(int_channel)
  {
  case LOG_TO_MAIN_UART:
    result = out_toM_UART(message);
    break;
  case LOG_TO_AUX_UART:
    result = out_toA_UART(message);
    break;

  case LOG_TO_USB0:
    result = out_toUSB(0, message);
    break;
  case LOG_TO_USB1:
    result = out_toUSB(1, message);
    break;
  default:
    //TODO return some error
    break;
  }

  return result;
}


/*-----------------------------------------------------------------------------------------------*/
/**
  \brief Print directly on the main UART

  \param [in] message: the string to print
  \return sent bytes

 */
/*-----------------------------------------------------------------------------------------------*/
INT32 Logger::out_toM_UART(const CHAR *message)
{
  INT32 sent = 0;


  /* Get a UART handle first */
  if(USIF0_fd == -1)
  {
    USIF0_fd = m2mb_uart_open( "/dev/tty0", 0 );
  }

  if ( -1 != USIF0_fd)
  {
    sent = m2mb_uart_write(USIF0_fd, (char*) message, strlen(message));
  }

  return sent;
}

/*-----------------------------------------------------------------------------------------------*/
/**
  \brief Print directly on the auxiliary UART

  \param [in] message: the string to print
  \return sent bytes

 */
/*-----------------------------------------------------------------------------------------------*/
INT32 Logger::out_toA_UART(const CHAR *message)
{
  INT32 sent = 0;


  /* Get an AUX UART handle first */
  if(USIF1_fd == -1)
  {
    USIF1_fd = m2mb_uart_open( "/dev/tty1", 0 );
  }

  if ( -1 != USIF1_fd)
  {
    sent = m2mb_uart_write(USIF1_fd, (char*) message, strlen(message));

  }

  return sent;
}

/*-----------------------------------------------------------------------------------------------*/
/**
  \brief Prints as out_toM_UART but using a specified USB channel

  \param [in] channel:  USB resource path where to print (e.g. 0 for USB0, 1 for USB1)
  \param [in] message : Message to print
  \return sent bytes, negative in case of error

 */
/*-----------------------------------------------------------------------------------------------*/
INT32 Logger::out_toUSB(UINT8 channel, const CHAR *message)
{
  char path[16];
  INT32 sent = 0;
  INT32 *p_usb_fd = NULL;


  /*get the requested channel from the path, so the related semaphore can be retrieved */
  switch(channel)
  {
  case 0:
    memset(path,0,sizeof(path));
    sprintf(path, "/dev/USB0");
    p_usb_fd = &USB0fd;
    break;
  case 1:
    memset(path,0,sizeof(path));
    sprintf(path, "/dev/USB1");
    p_usb_fd = &USB1fd;
    break;
  default:
    return CANNOT_OPEN_USB_CH;
  }
  /* Get a USB handle first */
  if(*p_usb_fd == -1)
  {
    *p_usb_fd = m2mb_usb_open(path, 0);
  }
  if ( *p_usb_fd == -1 )
  {
    return CANNOT_OPEN_USB_CH;
  }
  sent = m2mb_usb_write ( *p_usb_fd, (const void*) message, strlen(message));
  /* in case of concurrency using m2m_hw_usb... comment the next API to avoid the closing */
  //(void)m2mb_usb_close(*p_usb_fd);
  return sent;
}



/*-----------------------------------------------------------------------------------------------*/
/**
  \brief Removes the file path from the provided path, leaving only filename

  \param [in] path: the file path
  \return the filename(+ extension) extracted from the path

 */
/*-----------------------------------------------------------------------------------------------*/
const char* Logger::get_fileName(const CHAR* path)
{
  const CHAR* p = path;

  while (*p) {
    if (*p == '/' || *p == '\\') {
      return p + 1;
    }

    p++;
  }
  return path;
}


/*-----------------------------------------------------------------------------------------------*/
/**
  \brief Returns the current task name

  \param [in] name: the buffer where the task name will be saved
  \return a reference to name variable

 */
/*-----------------------------------------------------------------------------------------------*/
char *Logger::get_taskName(CHAR *name)
{
  MEM_W out;
  M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();

  if (M2MB_OS_SUCCESS != m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &out, NULL ))
  {
    return NULL;
  }
  else
  {
    strcpy(name, (CHAR*)out);
    return name;
  }
}

/* Global functions =============================================================================*/

/*Constructor*/
Logger::Logger(M2M_LOG_LEVEL_E level, M2M_LOG_HANDLE_E channel)
{
  int_level = level;
  int_channel = channel;
  CSSemHandle = NULL;

  isInit = true;
}

Logger::Logger(M2M_LOG_LEVEL_E level, M2M_LOG_HANDLE_E channel, bool colours)
{
  int_level = level;
  int_channel = channel;
  CSSemHandle = NULL;

  colouredLogs = colours;
  isInit = true;
}


/*Destructor*/
Logger::~Logger(void)
{
  
  if ( !isInit)
  {
    return;
  }

  switch(int_channel)
  {
  case LOG_TO_MAIN_UART:
    if(USIF0_fd != -1)
    {
      m2mb_uart_close(USIF0_fd);
      USIF0_fd = -1;
    }
    break;
  case LOG_TO_AUX_UART:
    if(USIF1_fd != -1)
    {
      m2mb_uart_close(USIF1_fd);
      USIF1_fd = -1;
    }
    break;
  case LOG_TO_USB0:
    if(USB0fd != -1)
    {
      m2mb_usb_close(USB0fd);
      USB0fd = -1;
    }
    break;
  case LOG_TO_USB1:
    if(USB1fd != -1)
    {
      m2mb_usb_close(USB1fd);
      USB1fd = -1;
    }
    break;
  default:
    //TODO return some error
    break;
  }
  isInit = FALSE;
}


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
INT32 Logger::out_format( M2M_LOG_LEVEL_E level, const char* function, const char* file, int line, const CHAR *fmt, ... )
{
  INT32 sent = 0;
  va_list arg;
  INT32   offset = 0;
  UINT32 now;
  M2MB_OS_SEM_ATTR_HANDLE semAttrHandle;

  /* If the selected log level is set */
  if(level >= int_level)
  {

    if (NULL == CSSemHandle)
    {
      m2mb_os_sem_setAttrItem( &semAttrHandle, CMDS_ARGS( M2MB_OS_SEM_SEL_CMD_CREATE_ATTR,  NULL,M2MB_OS_SEM_SEL_CMD_COUNT, 1 /*CS*/,
          M2MB_OS_SEM_SEL_CMD_TYPE, M2MB_OS_SEM_GEN,M2MB_OS_SEM_SEL_CMD_NAME, "LogCSSem"));
      m2mb_os_sem_init( &CSSemHandle, &semAttrHandle );
    }

    m2mb_os_sem_get(CSSemHandle, M2MB_OS_WAIT_FOREVER );

    now = Uptime();
    /*Prepare buffer*/
    memset(log_buffer,0,LOG_BUFFER_SIZE);

    switch(level)
    {
    LOG_PREFIX(TRACE);
    LOG_PREFIX(DEBUG);
    LOG_PREFIX(WARN);
    LOG_PREFIX(ERROR);
    LOG_PREFIX(CRITICAL);
    default:
      break;
    }

    va_start(arg, fmt);

    vsnprintf((char* )(log_buffer + offset), (size_t) (LOG_BUFFER_SIZE - offset), (const char*) fmt, arg);
    va_end(arg);

    /* Print the message on the selected output stream */
    sent = out(log_buffer);

  }

  m2mb_os_sem_put(CSSemHandle);
  return sent;

}


} /*namespace Log*/
