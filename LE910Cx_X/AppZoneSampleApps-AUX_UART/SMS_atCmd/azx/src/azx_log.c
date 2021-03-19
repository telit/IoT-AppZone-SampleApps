/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/* Include files =============================================================*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_usb.h"
#include "m2mb_uart.h"

#include "m2mb_fs_posix.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_rtc.h"

#include "app_cfg.h"
#include "azx_log.h"

/* Local defines =============================================================*/
#define USB_CH_MAX 3
#define LOG_BUFFER_SIZE 2048
#define MAX_FILE_LOG_CACHE 10000

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
    case AZX_LOG_LEVEL_##tag: \
    offset = snprintf(log_buffer, LOG_BUFFER_SIZE, \
        (log_cfg.colouredLogs)? prefix_fmt_colour : prefix_fmt_no_colour, \
            (log_cfg.colouredLogs)? LOG_##tag##_COLOR: "", #tag, (log_cfg.colouredLogs)? NO_COLOUR :"", \
                now / 1000.0, \
                get_file_title(file), line, \
                function, \
                get_current_task_name(task_name) \
    ); \
    break

#define LOG_FILE_PREFIX(tag) \
    case AZX_LOG_LEVEL_##tag: \
      offset = snprintf(log_buffer, LOG_BUFFER_SIZE, \
          prefix_fmt_file, get_date_time(), \
          ((now / 10) % 100), #tag, \
          get_file_title(file), line \
      ); \
    break

/* Local typedefs ============================================================*/
/* Local statics =============================================================*/
static struct
{
  BOOLEAN isInit;
  AZX_LOG_LEVEL_E level;
  AZX_LOG_HANDLE_E channel;
  INT32 ch_fd;
  BOOLEAN colouredLogs;
  M2MB_OS_SEM_HANDLE CSSemHandle;
} log_cfg = {/*.isInit*/ FALSE, /*.level*/ AZX_LOG_LEVEL_NONE, /*.channel*/ AZX_LOG_TO_MAX, /*.ch_fd */  -1, /*.colouredLogs*/ FALSE, /*.CSSemHandle */  NULL};



static CHAR log_buffer[LOG_BUFFER_SIZE] = { 0 };
static CHAR task_name[64];
static CHAR dateTime[32] = { 0 };

static struct
{
  M2MB_FILE_T* fd;
  CHAR name[32];
  CHAR current_name[40];
  UINT32 circular_chunks;
  UINT32 max_size_kb;
  AZX_LOG_LEVEL_E min_level;
  UINT32 cache_idx;
  CHAR cache[MAX_FILE_LOG_CACHE];
} logFile = {
  /*.fd */
      0,
  /*.name */
  { 0 },
  /*.current_name */
  { 0 },
  /*.circular_chunks */
  0,
  /*.max_size_kb */
  0,
  /*.min_level */
  AZX_LOG_LEVEL_CRITICAL,
  /*.cache_idx */
  0,
  /*.cache */
  { 0 }
};


static const CHAR* prefix_fmt_colour = "[%s%-5s%s] %3.2f  " CYAN "%s" NO_COLOUR
    ":" BOLD CYAN "%d" NO_COLOUR
    " - %s{" BOLD WHITE "%s" NO_COLOUR "}$ ";
static const CHAR* prefix_fmt_no_colour = "[%s%-5s%s] %3.2f  %s:%d - %s{%s}$ ";
static const CHAR* prefix_fmt_file = "%s.%.2u [%-5s] %s:%d ";


/* Local function prototypes =================================================*/

/*----------------------------------------------------------------------------*/
/*!
  \brief Print directly on the main UART

  \param [in] message: the string to print
  \return sent bytes
 */
/*----------------------------------------------------------------------------*/
static INT32 log_print_to_UART(const CHAR *message);

/*----------------------------------------------------------------------------*/
/*!
  \brief Print directly on the auxiliary UART

  \param [in] message: the string to print
  \return sent bytes

 */
/*----------------------------------------------------------------------------*/
static INT32 log_print_to_AUX_UART(const CHAR *message);

/*----------------------------------------------------------------------------*/
/*!
  \brief Prints as log_printToUart  but using a specified USB channel

  \param [in] path:     USB resource path where to print (e.g. /dev/USB0
  \param [in] message : Message to print
  \return sent bytes, negative in case of error

  \details Using channel:USB_CH_DEFAULT uses channel assigned to instance
  USER_USB_INSTANCE_0
 */
/*----------------------------------------------------------------------------*/
static INT32  log_print_to_USB (const CHAR *path, const CHAR *message );

static UINT32 get_uptime(void);
static const char* get_file_title(const CHAR* path);
static char* get_current_task_name(CHAR *name);
static BOOLEAN check_file_size(const CHAR* filename, UINT32 max_size_kb);
static void flush_log_to_file(void);
static void file_log_or_cache(const CHAR* buffer);
static const CHAR* get_next_log_filename(const CHAR* filename,
    UINT32 circular_chunks, UINT32 max_size_kb);
static BOOLEAN rotate_log_files(const CHAR* filename, UINT32 circular_chunks);
static const char* get_date_time(void);

/* Static functions ==========================================================*/

/*----------------------------------------------------------------------------*/
/*!
  \brief Print directly on the main UART

  \param [in] message: the string to print
  \return sent bytes

 */
/*----------------------------------------------------------------------------*/
static INT32 log_print_to_UART(const CHAR *message)
{
  INT32 sent = 0;

  /* Get a UART handle first */
  if(log_cfg.ch_fd == -1)
  {
    log_cfg.ch_fd = m2mb_uart_open( "/dev/tty0", 0 );
  }

  if ( -1 != log_cfg.ch_fd)
  {
    sent = m2mb_uart_write(log_cfg.ch_fd, (char*) message, strlen(message));

  }
  return sent;
}

/*----------------------------------------------------------------------------*/
/*!
  \brief Print directly on the auxiliary UART

  \param [in] message: the string to print
  \return sent bytes

 */
/*----------------------------------------------------------------------------*/
static INT32 log_print_to_AUX_UART(const CHAR *message)
{
  INT32 sent = 0;

  /* Get a UART handle first */
  if(log_cfg.ch_fd == -1)
  {
    log_cfg.ch_fd = m2mb_uart_open( "/dev/tty1", 0 );
  }

  if ( -1 != log_cfg.ch_fd)
  {
    sent = m2mb_uart_write(log_cfg.ch_fd, (char*) message, strlen(message));

    //m2mb_uart_close(g_AUX_fd);
  }
  return sent;
}

/*----------------------------------------------------------------------------*/
/*!
  \brief Prints as log_printToUart  but using a specified USB channel

  \param [in] path:    USB resource path where to print (e.g. /dev/USB0
  \param [in] message: Message to print
  \return sent bytes, negative in case of error

 */
/*-----------------------------------------------------------------------------*/
static INT32 log_print_to_USB (const CHAR *path, const CHAR *message )
{
  INT32 ch;
  INT32 result;
  INT32 sent = 0;

  /* get the requested channel from the path,
   * so the related semaphore can be retrieved */
  result = sscanf(path, "/dev/USB%d", &ch);
  if(!result)
  {
    return AZX_LOG_CANNOT_OPEN_USB_CHANNEL;
  }


  /* Get a USB handle first */
  if(log_cfg.ch_fd == -1)
  {
    log_cfg.ch_fd = m2mb_usb_open(path, 0);
  }
  if ( log_cfg.ch_fd == -1 )
  {
    return AZX_LOG_CANNOT_OPEN_USB_CHANNEL;
  }
  sent = m2mb_usb_write( log_cfg.ch_fd, (const void*) message, strlen(message));

  /* in case of concurrency using m2m_hw_usb...
   * Comment the next API to avoid closing */
  //(void)m2mb_usb_close(log_cfg.ch_fd);

  return sent;
}

/*-----------------------------------------------------------------------------------------------*/
/*!
  \brief returns the system uptime in milliseconds

  \return the system uptime

 */
/*-----------------------------------------------------------------------------------------------*/
static UINT32 get_uptime(void)
{

  UINT32 sysTicks = m2mb_os_getSysTicks();

  FLOAT32 ms_per_tick = m2mb_os_getSysTickDuration_ms();

  return (UINT32) (sysTicks * ms_per_tick); //milliseconds
}


static CHAR fileTitle[12] = "";

/*-----------------------------------------------------------------------------------------------*/
/*!
  \brief Removes the file path from the provided path, leaving only filename

  \param [in] path: the file path
  \return the filename(+ extension) extracted from the path

 */
/*-----------------------------------------------------------------------------------------------*/
static const char* get_file_title(const CHAR* path)
{
  const CHAR* p = path;
  const CHAR* start = path;
  const CHAR* end = path;

  while (*p) {
    if (*p == '/' || *p == '\\') {
      start = p + 1;
    }

    if(*p == '.') {
      end = p;
    }

    p++;
  }

  if(end <= start) {
    end = p;
  }

  snprintf(fileTitle, sizeof(fileTitle), "%.*s", (INT32)(end - start), start);
  return fileTitle;
}


/*-----------------------------------------------------------------------------------------------*/
/*!
  \brief Returns the current task name

  \param [in] name: the buffer where the task name will be saved
  \return a reference to name variable

 */
/*-----------------------------------------------------------------------------------------------*/
static char* get_current_task_name(CHAR *name)
{
  MEM_W out;
  M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();

  if(M2MB_OS_SUCCESS !=
      m2mb_os_taskGetItem(taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &out, NULL))
  {
    return NULL;
  }
  else
  {
    strcpy(name, (CHAR*)out);
    return name;
  }
}

static const char* get_date_time(void)
{
  INT32 fd = m2mb_rtc_open( "/dev/rtc0", 0 );
  M2MB_RTC_TIME_T ts = { 0 };
  if(fd == -1)
  {
    dateTime[0] = '\0';
    return dateTime;
  }

  if(-1 == m2mb_rtc_ioctl( fd, M2MB_RTC_IOCTL_GET_SYSTEM_TIME, &ts ))
  {
    m2mb_rtc_close( fd );
    dateTime[0] = '\0';
    return dateTime;
  }
  m2mb_rtc_close( fd );
  snprintf(dateTime, sizeof(dateTime), "%02u-%02u %02u:%02u:%02u",
      ts.mon, ts.day,
      ts.hour, ts.min, ts.sec);
  return dateTime;
}

/* Global functions ==========================================================*/


void azx_log_init(AZX_LOG_CFG_T *cfg)
{
  M2MB_OS_SEM_ATTR_HANDLE semAttrHandle;

  if(log_cfg.isInit == TRUE)
  {
    return;
  }

  log_cfg.level = cfg->log_level;
  log_cfg.channel = cfg->log_channel;
  log_cfg.colouredLogs = cfg->log_colours;
  log_cfg.ch_fd = -1;

  if (NULL == log_cfg.CSSemHandle)
  {
    m2mb_os_sem_setAttrItem(&semAttrHandle,
        CMDS_ARGS(M2MB_OS_SEM_SEL_CMD_CREATE_ATTR, NULL,
            M2MB_OS_SEM_SEL_CMD_COUNT, 1 /*CS*/,
            M2MB_OS_SEM_SEL_CMD_TYPE, M2MB_OS_SEM_GEN,
            M2MB_OS_SEM_SEL_CMD_NAME, "CSSem"));
    m2mb_os_sem_init( &log_cfg.CSSemHandle, &semAttrHandle );
  }
  log_cfg.isInit = TRUE;
}


INT32 azx_log_deinit(void)
{
  INT32 rc;
  if ( ! log_cfg.isInit)
  {
    return AZX_LOG_NOT_INIT;
  }

  switch(log_cfg.channel)
  {
  case AZX_LOG_TO_MAIN_UART:
    if(log_cfg.ch_fd != -1)
    {
      rc = m2mb_uart_close(log_cfg.ch_fd);
    }
    break;
  case AZX_LOG_TO_AUX_UART:
    if(log_cfg.ch_fd != -1)
    {
      rc = m2mb_uart_close(log_cfg.ch_fd);
    }
    break;
  case AZX_LOG_TO_USB0:
    if(log_cfg.ch_fd != -1)
    {
      rc = m2mb_usb_close(log_cfg.ch_fd);
    }
    break;
  case AZX_LOG_TO_USB1:
    if(log_cfg.ch_fd != -1)
    {
      rc = m2mb_usb_close(log_cfg.ch_fd);
    }
    break;
  default:
    //TODO return some error
    break;
  }
  log_cfg.ch_fd = -1;

  //destroy lock related to USB that can not be opened
  m2mb_os_sem_deinit(log_cfg.CSSemHandle);
  log_cfg.CSSemHandle = NULL;

  log_cfg.isInit = FALSE;
  return rc;
}

void azx_log_setLevel(AZX_LOG_LEVEL_E level)
{

  log_cfg.level = level;
}

AZX_LOG_LEVEL_E azx_log_getLevel(void)
{
  if(log_cfg.isInit)
  {
    return log_cfg.level;
  }

  return AZX_LOG_LEVEL_NONE;
}

/*----------------------------------------------------------------------------*/
/*!
  \brief Prints on the requested log channel (USB, UART, AUX)

  \param [in] msg: message to be printed on output
  \return amount of printed bytes, negative value in case of error

 */
/*----------------------------------------------------------------------------*/
static INT32 log_base_function(const char *msg)
{
  INT32 result;


  if ( ! log_cfg.isInit)
  {
    return AZX_LOG_NOT_INIT;
  }

  switch(log_cfg.channel)
  {
  case AZX_LOG_TO_MAIN_UART:
    result = log_print_to_UART(msg);
    break;
  case AZX_LOG_TO_AUX_UART:
    result = log_print_to_AUX_UART(msg);
    break;
  case AZX_LOG_TO_USB0:
    result = log_print_to_USB("/dev/USB0", msg);
    break;
  case AZX_LOG_TO_USB1:
    result = log_print_to_USB("/dev/USB1", msg);
    break;
  default:
    //TODO return some error
    break;
  }

  return result;
}

/*----------------------------------------------------------------------------*/
/*!
  \brief Prints on the defined stream (UART or USB channel)
 *
  \param [in] level:    Logging level. see AZX_LOG_LEVEL_E enum
  \param [in] function: source function name to add to the output if log is verbose
  \param [in] file:     source file path to add to the output if log is verbose
  \param [in] line:     source file line to add to the output if log is verbose
  \param [in] fmt :     string format with parameters to print
  \param [in] ... :     ...
  \return the number of sent bytes.
    0 if logging level is not enabled, negative in case of error

 */
/*----------------------------------------------------------------------------*/
INT32 azx_log_formatted(AZX_LOG_LEVEL_E level,
    const char* function, const char* file, int line, const CHAR *fmt, ... )
{
  INT32  sent = 0;
  va_list arg;
  INT32   offset = 0;
  UINT32 now;

  /* If the selected log level is set */
  if(level >= azx_log_getLevel())
  {
    m2mb_os_sem_get(log_cfg.CSSemHandle, M2MB_OS_WAIT_FOREVER );

    now = get_uptime();
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
    vsnprintf(log_buffer + offset, LOG_BUFFER_SIZE-offset, fmt, arg);
    va_end(arg);

    /* Print the message on the selected output stream */
    sent = log_base_function(log_buffer);

    if(logFile.fd && level >= logFile.min_level)
    {
      if(!check_file_size(logFile.current_name, logFile.max_size_kb))
      {
        /* Log limit reached, so we'll need to open the next file in the rotation. Log in the file
         * that this limit is reached and then get the next filename */
        m2mb_fs_fputs("=== Log file size limit reached\r\n", logFile.fd);
        m2mb_fs_fclose(logFile.fd);
        logFile.fd = 0;

        snprintf(logFile.current_name, sizeof(logFile.current_name), "%s",
            get_next_log_filename(logFile.name, logFile.circular_chunks,
              logFile.max_size_kb));

        if(logFile.current_name[0] == '\0')
        {
          goto end;
        }

        logFile.fd = m2mb_fs_fopen(logFile.current_name, "a");

        if(!logFile.fd)
        {
          goto end;
        }
      }

      switch(level)
      {
        LOG_FILE_PREFIX(TRACE);
        LOG_FILE_PREFIX(DEBUG);
        LOG_FILE_PREFIX(INFO);
        LOG_FILE_PREFIX(WARN);
        LOG_FILE_PREFIX(ERROR);
        LOG_FILE_PREFIX(CRITICAL);
        default:
          break;
      }

      va_start(arg, fmt);
      vsnprintf(log_buffer + offset, LOG_BUFFER_SIZE - offset, fmt, arg);
      va_end(arg);
      file_log_or_cache(log_buffer);
    }

end:
    m2mb_os_sem_put(log_cfg.CSSemHandle);
  }

  return sent;
}

static BOOLEAN check_file_size(const CHAR* filename, UINT32 max_size_kb)
{
  struct M2MB_STAT stat;
  if(-1 == m2mb_fs_stat(filename, &stat))
  {
    /* Most likely the file doesn't exist, so return true */
    return TRUE;
  }
  return ((stat.st_size >> 10) < max_size_kb);
}

static void flush_log_to_file(void)
{
  logFile.cache[logFile.cache_idx] = '\0';
  m2mb_fs_fwrite(logFile.cache, logFile.cache_idx, 1, logFile.fd);
  logFile.cache_idx = 0;
}

static void file_log_or_cache(const CHAR* buffer)
{
  const UINT32 size = strlen(buffer);

  if(MAX_FILE_LOG_CACHE - 1 - size < logFile.cache_idx)
  {
    flush_log_to_file();
  }

  memcpy(&logFile.cache[logFile.cache_idx], buffer, size);
  logFile.cache_idx += size;
}

static CHAR filenameInUse[40] = "";

static BOOLEAN rotate_log_files(const CHAR* filename, UINT32 circular_chunks)
{
  CHAR from[40] = "";
  CHAR to[40] = "";
  UINT32 i = 0;

  /* This will move log.1 to log.2, log.2 to log.3 and so on until the last one */
  snprintf(from, sizeof(from), "%s.%u", filename, circular_chunks);

  m2mb_fs_remove(from);

  for(i = circular_chunks - 1; i > 0; --i)
  {
    snprintf(from, sizeof(from), "%s.%u", filename, i);
    snprintf(to, sizeof(to), "%s.%u", filename, i+1);
    if(-1 == m2mb_fs_rename(from, to))
    {
      /* Only return FALSE when the final file cannot be renamed - the other renames could be
       * failing if there are no files there */
      if(i == 1)
      {
        return FALSE;
      }
    }
  }
  return TRUE;
}

static const CHAR* get_next_log_filename(const CHAR* filename,
    UINT32 circular_chunks, UINT32 max_size_kb)
{
  filenameInUse[0] = '\0';

  if(!filename)
  {
    goto end;
  }

  /* First try the original file */
  snprintf(filenameInUse, sizeof(filenameInUse), "%s", filename);

  if(check_file_size(filenameInUse, max_size_kb))
  {
    goto end;
  }

  /* If there is no circular logging and the original file is full, just return empty string */
  if(circular_chunks == 0)
  {
    filenameInUse[0] = '\0';
    goto end;
  }

  /* We should log to filename.1, but if that's full we need to rotate logs and then still use
   * filename.1 (since the old one got moved to filename.2) */
  snprintf(filenameInUse, sizeof(filenameInUse), "%s.1", filename);

  if(check_file_size(filenameInUse, max_size_kb))
  {
    goto end;
  }

  if(!rotate_log_files(filename, circular_chunks))
  {
    /* Since we failed to rotate, just wipe filename.1 so it can be reused */
    m2mb_fs_remove(filenameInUse);
    goto end;
  }

end:
  return filenameInUse;
}

BOOLEAN azx_log_send_to_file(const CHAR* filename, UINT32 circular_chunks,
    AZX_LOG_LEVEL_E min_level, UINT32 max_size_kb)

{
  if(!filename)
  {
    return FALSE;
  }

  if(circular_chunks == 0 && !check_file_size(filename, max_size_kb))
  {
    return FALSE;
  }

  if(logFile.fd)
  {
    m2mb_fs_fclose(logFile.fd);
    logFile.fd = 0;
  }

  snprintf(logFile.current_name, sizeof(logFile.current_name), "%s",
      get_next_log_filename(filename, circular_chunks, max_size_kb));

  if(logFile.current_name[0] == '\0')
  {
    return FALSE;
  }

  logFile.fd = m2mb_fs_fopen(logFile.current_name, "a");

  if(!logFile.fd)
  {
    return FALSE;
  }

  snprintf(logFile.name, sizeof(logFile.name), "%s", filename);
  logFile.circular_chunks = circular_chunks;
  logFile.min_level = min_level;
  logFile.max_size_kb = max_size_kb;
  logFile.cache_idx = 0;
  return TRUE;
}

void azx_log_flush_to_file(void)
{
  m2mb_os_sem_get(log_cfg.CSSemHandle, M2MB_OS_WAIT_FOREVER );
  flush_log_to_file();
  m2mb_os_sem_put(log_cfg.CSSemHandle);
}
