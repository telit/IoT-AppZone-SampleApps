/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    The application shows how to create and manage tasks with m2mb APIs. Debug prints on MAIN UART (can be changed in M2MB_Main function)
  @version 
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    14/04/2020
*/

/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h" /*For task and queue functionalities*/

#include "azx_log.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/

#define TASK_NAME "mytask" /* <-this will be shown in verbose log prints */
#define TASK_PRIORITY 201 /* In range 1-255. The bigger the value, the lower the priority. It is suggested to use values > 200 for typical applications. */
#define TASK_STACK_SIZE 8192 /* Size in bytes. if a buffer is not provided, it will be allocated by system in user HEAP */

#define QUEUE_NAME "myqueue"
#define QUEUE_SLOTS 20; /* how many messages can be enqueued for the task*/
#define QUEUE_RX_TIMEOUT M2MB_OS_WAIT_FOREVER

#define MESSAGE_SIZE 32 /*size of the data buffer available for each message */

/* Local typedefs ===============================================================================*/

/*
 The "user" message structure. It can contain anything, depending on application's requirements. 
 It is supposed to be WORD32 aligned in size (so a multiple of 4 Bytes).
 
 ****!!!!! PLEASE NOTE:  the size of a message must be within 4 - 64 bytes range. 
 Please refer to M2MB_OS_Q_SEL_CMD_MSG_SIZE macro in m2mb_os_q.h ****s
*/

typedef struct
{
  unsigned int size;
  char data[MESSAGE_SIZE];
} QUEUE_MESSAGE_T;


/*Demo related result codes*/
typedef enum
{
  SUCCESS = 0,
  QUEUE_BUFFER_CREATE_FAIL,
  QUEUE_ATTR_INIT_FAIL,
  QUEUE_INIT_FAIL,
  TASK_ATTR_INIT_FAIL,
  TASK_INIT_FAIL,
  TASK_RESUME_FAIL,
  MESSAGE_TOO_LARGE,
  QUEUE_FULL,
  QUEUE_FAIL
} RESULT_CODES_E;

/* Local statics ================================================================================*/
/* Task related variables */
M2MB_OS_TASK_HANDLE _task_handle = M2MB_OS_TASK_INVALID;
const char * _task_name = TASK_NAME;
INT32 _stack_size = TASK_STACK_SIZE;


/* Queue related variables */
M2MB_OS_Q_HANDLE _queue_handle = M2MB_OS_Q_INVALID;
const char * _queue_name = QUEUE_NAME;
unsigned int _queue_size = 0;
void * _queue_buffer = 0;





/* Local function prototypes ====================================================================*/
static void task_entry_function(void * arg);

/* Static functions =============================================================================*/

/* This is the task entry function, executed when task is started. */
/* arg is the user data context (if provided during task init) */
static void task_entry_function(void * arg)
{
  (void) arg; /*Unused in this demo. it can be cast to any user data type */
  
  M2MB_OS_RESULT_E os_res;
  QUEUE_MESSAGE_T _message;
  
  UINT32 timeout = QUEUE_RX_TIMEOUT;
  
  /* Endless loop, waiting for messages */
  while ( 1 )
  {
    os_res = m2mb_os_q_rx(_queue_handle, &_message, timeout);
    switch(os_res)
    {
      case M2MB_OS_SUCCESS:
        /* Normal flow*/
        AZX_LOG_DEBUG( "Received a message with a %d bytes payload: <%.*s>\r\n", 
                          _message.size, _message.size, _message.data);
        /*Do something with _message.data buffer */
        
      break;
      
      /* -- ERROR CASES -- */
      case M2MB_OS_WAIT_ABORTED:
        /*Task was put out of sospension by another task: see m2mb_os_taskWaitAbort(...)*/
        AZX_LOG_WARN("Task forcely resumed from waiting message..\r\n");
      break;
      
      case M2MB_OS_QUEUE_EMPTY:
        AZX_LOG_WARN("queue empty within timeout\r\n");
        if(timeout == M2MB_OS_NO_WAIT)
        {
          /* add artificial wait to avoid infinite loop */
          m2mb_os_taskSleep(M2MB_OS_MS2TICKS(100));
        }
      break;
      
      case M2MB_OS_INVALID_ARG:
      case M2MB_OS_DELETED:
      case M2MB_OS_QUEUE_ERROR:
        /*Better to make the task complete */
        AZX_LOG_ERROR("Queue invalid or deleted..\r\n");
        return;
      break;
    
      case M2MB_OS_PTR_ERROR:
        /*Better to make the task complete */
        AZX_LOG_WARN("Invalid destination pointer 0x08X\r\n", &_message);
        return;
      break;
      
      default:
        AZX_LOG_ERROR("unexpected return code %d\r\n", os_res);
        break;
    }
  }
}
/* Global functions =============================================================================*/

void free_resources(void)
{
  M2MB_OS_RESULT_E os_res;
  MEM_W  out = 0;
     
  /* Queue object */
  if ( _queue_handle != M2MB_OS_Q_INVALID )
  {
    AZX_LOG_TRACE("Trying to clear queue...\r\n");
    os_res = m2mb_os_q_clear(_queue_handle);
    if (os_res != M2MB_OS_SUCCESS)
    {
      AZX_LOG_ERROR("Cannot clear queue. Code: %d\r\n", os_res);
      return ;
    }
    
    AZX_LOG_TRACE("Trying to deinit queue...\r\n");
    os_res = m2mb_os_q_deinit(_queue_handle);
    if (os_res != M2MB_OS_SUCCESS)
    {
      AZX_LOG_ERROR("Cannot deinit queue. Code: %d\r\n", os_res);
      return;
    }
    
    _queue_handle = M2MB_OS_Q_INVALID;
    
  }

  /*/ Task object */
  if ( _task_handle != M2MB_OS_TASK_INVALID )
  {
    os_res = m2mb_os_taskTerminate(_task_handle);
    if (os_res != M2MB_OS_SUCCESS)
    {
      AZX_LOG_ERROR("Cannot terminate task\r\n");
    }
    m2mb_os_taskGetItem( _task_handle, M2MB_OS_TASK_SEL_CMD_STATE, &out, NULL );
    AZX_LOG_TRACE ( "Task state: %d (refer to M2MB_OS_TASK_STATE_E enum in m2mb_os.h)\r\n", out );

    AZX_LOG_TRACE("Trying to delete task with handle %d...\r\n", _task_handle);
    os_res = m2mb_os_taskDelete(_task_handle);
    if (os_res != M2MB_OS_SUCCESS)
    {
      AZX_LOG_ERROR("Cannot delete task. Code: %d\r\n", os_res);
      return;
    }
    _task_handle = M2MB_OS_TASK_INVALID;
    
  }

  /* Queue memory buffer */
  if ( _queue_buffer != NULL )
  {
    m2mb_os_free(_queue_buffer);
  }

  /* Re-initialize variables */
  _queue_size = 0;
  _queue_buffer = NULL;
}




/*  
    Sends a message to the already created queue, 
    where the task entry function is waiting for incoming data
*/
int send_message(const void *content, unsigned int content_size)
{
  M2MB_OS_RESULT_E os_res;
  QUEUE_MESSAGE_T _message;
  
  if (_queue_handle == NULL)
  {
    return QUEUE_INIT_FAIL; /*queue is not initialized*/
  }
  if(content_size > MESSAGE_SIZE)
  {
    return MESSAGE_TOO_LARGE;
  }
  
  memset(_message.data,0, MESSAGE_SIZE);
  memcpy(_message.data,content, content_size);
  _message.size = content_size;
  
  os_res = m2mb_os_q_tx(_queue_handle, &_message, M2MB_OS_NO_WAIT, 0);
  
  if ( os_res == M2MB_OS_SUCCESS )
  {
    return SUCCESS;
  }
  else if ( os_res == M2MB_OS_QUEUE_FULL )
  {
    return QUEUE_FULL;
  }
  else
  {
    return QUEUE_FAIL;
  }
  
  
}

/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
**************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;
  
  AZX_LOG_CFG_T log_cfg;
  
  int result_code = SUCCESS;
  M2MB_OS_RESULT_E os_res;
  
  M2MB_OS_Q_ATTR_HANDLE queue_attributes_handle;
  M2MB_OS_TASK_ATTR_HANDLE task_attributes_handle;

  void *userdata = NULL; /* This is an optional user context parameter. If ! NULL, it will be passed to task entry function at its startup */
  


  m2mb_os_taskSleep(M2MB_OS_MS2TICKS(2000)); /*Wait 2 seconds*/

  /*Set log configuration */
  log_cfg.log_channel = AZX_LOG_TO_MAIN_UART; 
  log_cfg.log_level = AZX_LOG_LEVEL_DEBUG; /*Enable debug messages*/
  log_cfg.log_colours = 0; /*Set to 1 to use coloured logs (not all terminals are compatible)*/

  azx_log_init(&log_cfg); /*Log system initialized, now it is possible to send trace prints on selected channel*/
  
  
  AZX_LOG_INFO("Starting Basic Task demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);
  
  /*Define queue size*/
  _queue_size = BYTES_FOR_MSG(QUEUE_MESSAGE_T) * QUEUE_SLOTS;
  
  
  do
  {
    
    /*Create queue area */
    _queue_buffer = m2mb_os_malloc(_queue_size);
    
    if(NULL == _queue_buffer)
    {
      AZX_LOG_ERROR("Cannot allocate queue buffer. return...\r\n");
      result_code = QUEUE_BUFFER_CREATE_FAIL;
      break; /* Exit flow */
    }
    AZX_LOG_INFO("Successfully created a queue area buffer of %d bytes.\r\n", _queue_size);
    os_res = m2mb_os_q_setAttrItem( &queue_attributes_handle,
          CMDS_ARGS( 
            M2MB_OS_Q_SEL_CMD_CREATE_ATTR, NULL, /*Create attribute handle*/
            M2MB_OS_Q_SEL_CMD_QSTART, _queue_buffer,  /*Starting point of queue area*/
            M2MB_OS_Q_SEL_CMD_MSG_SIZE, WORD32_FOR_MSG(QUEUE_MESSAGE_T),
            M2MB_OS_Q_SEL_CMD_QSIZE, _queue_size
          )
    );
    
    if ( M2MB_OS_SUCCESS != os_res )
    {
      AZX_LOG_ERROR( "Failed setting queue attributes! Result: %d\r\n", os_res );
      result_code = QUEUE_ATTR_INIT_FAIL;
      break; /* Exit flow. Any cleanup will be done at the end */
    }
    
    os_res = m2mb_os_q_init( &_queue_handle, &queue_attributes_handle  );
    if ( M2MB_OS_SUCCESS != os_res )
    {
      AZX_LOG_ERROR( "Failed initializing queue! Result: %d\r\n", os_res );
      
      /*Clean queue attributes handle up*/
      m2mb_os_q_setAttrItem(&queue_attributes_handle, 1, M2MB_OS_Q_SEL_CMD_DEL_ATTR, NULL);
      result_code = QUEUE_INIT_FAIL;
      break; /* Exit flow. Any cleanup will be done at the end */
    }
    
    AZX_LOG_INFO( "Queue successfully created.\r\n" );
    
    os_res = m2mb_os_taskSetAttrItem( &task_attributes_handle,
                CMDS_ARGS(
                  M2MB_OS_TASK_SEL_CMD_CREATE_ATTR, NULL,
                  M2MB_OS_TASK_SEL_CMD_STACK_SIZE, (void*)_stack_size,
                  M2MB_OS_TASK_SEL_CMD_NAME, _task_name,
                  M2MB_OS_TASK_SEL_CMD_PRIORITY, TASK_PRIORITY,
                  M2MB_OS_TASK_SEL_CMD_PREEMPTIONTH, TASK_PRIORITY,
                  M2MB_OS_TASK_SEL_CMD_AUTOSTART, M2MB_OS_TASK_NOT_START, /*Task entry function will not be called at init completion*/
                  M2MB_OS_TASK_SEL_CMD_USRNAME, _task_name
             )
      );
    if ( M2MB_OS_SUCCESS != os_res )
    {
      AZX_LOG_ERROR( "Failed setting task attributes! Result: %d\r\n", os_res );
      result_code = TASK_ATTR_INIT_FAIL;
      break; /* Exit flow. Any cleanup will be done at the end */
    }
  
    AZX_LOG_INFO( "Creating the task...\r\n" );
    os_res = m2mb_os_taskCreate(
              &_task_handle,
              &task_attributes_handle,
              &task_entry_function,
              (void*) userdata /*user context pointer (optional)*/
              );

    if ( M2MB_OS_SUCCESS != os_res )
    {
      AZX_LOG_ERROR( "Failed creating the task! Result: %d\r\n", os_res );
      
      /*Clean task attributes handle up*/
      m2mb_os_taskSetAttrItem(&task_attributes_handle, 1, M2MB_OS_TASK_SEL_CMD_DEL_ATTR, NULL);
      
      result_code = TASK_INIT_FAIL;
      break; /* Exit flow. Any cleanup will be done at the end */
    }
    
    /*Start task entry function*/
    os_res = m2mb_os_taskResume(_task_handle);
    if ( M2MB_OS_SUCCESS != os_res )
    {
      AZX_LOG_ERROR( "Failed resuming the task! Result: %d\r\n", os_res );
      
      /*Clean task attributes handle up*/
      m2mb_os_taskSetAttrItem(&task_attributes_handle, 1, M2MB_OS_TASK_SEL_CMD_DEL_ATTR, NULL);
      
      result_code = TASK_RESUME_FAIL;
      break; /* Exit flow. Any cleanup will be done at the end */
    }
    
    
    AZX_LOG_INFO("Task created and ready to receive messages!\r\n");
  
    
  
    AZX_LOG_DEBUG("Sending a message to the task...\r\n");
    result_code = send_message("hello", 5);
    if(SUCCESS != result_code)
    {
      AZX_LOG_ERROR("Failed sending message: %d\r\n", result_code);
      break;
    }
    m2mb_os_taskSleep(M2MB_OS_MS2TICKS(2000)); /*Wait 2 seconds*/

    AZX_LOG_DEBUG("Sending a second message to the task...\r\n");    
    result_code = send_message("world", 5);
    if(SUCCESS != result_code)
    {
      AZX_LOG_ERROR("Failed sending message: %d\r\n", result_code);
      break;
    }
    
  } while(0);
  
  AZX_LOG_DEBUG("Result code at the end: %d\r\n", result_code);
  
  AZX_LOG_INFO("Clearing resources...\r\n");
  free_resources();
  
  AZX_LOG_INFO("Done. App complete\r\n");
} 

