/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

#include <stdio.h>
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"

#include "azx_log.h"

#include "azx_tasks.h"

/**
* @brief On task complete return function
* 
* If user onComplete function is registered, this function is will call it when a task user callback function exits. 
* @param[output] taskID the numeric ID of the task that whose callback function completed
* @param[output] type   the input "type" parameter that was passed to the task CB
* @param[output] res    the result value of the execution
*
* refer to azx_tasks_init_with_compl()
* @ingroup taskUsage
*/
static INT32 azx_tasks_onTaskCompl(INT32 taskID, INT32 type, INT32 res);

/* Global variables =============================================================================*/
_AZX_TASKS_PARAMS m2mb_tasks;

INT32 azx_tasks_init(void)
{
  INT32 ret;
  int i;
  for(i = 0; i < AZX_TASKS_MAX_TASKS; i++ )
  {
    m2mb_tasks.task_slots[i].Task_H = M2MB_OS_TASK_INVALID;
    m2mb_tasks.task_slots[i].Task_Queue_H = M2MB_OS_Q_INVALID;
    m2mb_tasks.task_slots[i].Task_NameBuf[0] = '\0';
    m2mb_tasks.task_slots[i].SlotInUse = 0;
  }

  m2mb_tasks.M2MMain_Handle = m2mb_os_taskGetId(); //store
  m2mb_tasks.complCB = NULL;
  m2mb_tasks.isInit = 1;

  ret = M2MB_OS_SUCCESS;

  return ret;
}

INT32 azx_tasks_initWithComplCB(azx_tasks_onTaskComplCB cb)
{
  INT32 ret = azx_tasks_init();
  m2mb_tasks.complCB = cb;
  return ret;
}

static INT32 find_free_task_slot(void)
{
  int i;

  if (! m2mb_tasks.isInit)
  {
    return -1;
  }

  for (i=0; i< AZX_TASKS_MAX_TASKS; i++)
  {
    if(m2mb_tasks.task_slots[i].SlotInUse == 0)
    {
      m2mb_tasks.task_slots[i].SlotInUse = 1;
      return i;
    }
  }
  return -1;
}

static INT32 find_slot_by_handle(M2MB_OS_TASK_HANDLE h)
{
  int i;
  for (i=0; i< AZX_TASKS_MAX_TASKS; i++)
  {
    if(m2mb_tasks.task_slots[i].SlotInUse == 1)
    {
    AZX_LOG_TRACE("comparing task handle %p with current slot %d handle %p\r\n", h, i, m2mb_tasks.task_slots[i].Task_H );
      if (h == m2mb_tasks.task_slots[i].Task_H)
      {
        return i;
      }
    }
  }
  return -1;

}

//char* name must be big enough to contain task names.
char * azx_tasks_getCurrentTaskName(char *name)
{
  MEM_W out;
  M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();
  if (taskHandle == m2mb_tasks.M2MMain_Handle)
  {
    strcpy(name, "M2M_MAIN");
  }
  else
  {
    if (M2MB_OS_SUCCESS != m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &out, NULL ))
    {
      return NULL;
    }
    else
    {
      strcpy(name, (CHAR*)out);
    }
  }
  return name;
}

INT32 azx_tasks_getCurrentTaskId(void)
{
  M2MB_OS_TASK_HANDLE h = m2mb_os_taskGetId();
  return find_slot_by_handle(h) + 1;
}


INT32 azx_tasks_sendMessageToTask( INT32 TaskProcID, INT32 type, INT32 param1, INT32 param2 )
{
  AZX_TASKS_MESSAGE_T tmpMsg;
  M2MB_OS_RESULT_E osRes;
  M2MB_OS_Q_HANDLE Queue_H;

  INT32 slot = TaskProcID - 1;

  if (! m2mb_tasks.isInit)
  {
    AZX_LOG_ERROR("m2m task first init not performed yet\r\n");
    return AZX_TASKS_NOTINIT_ERR;
  }


  if (slot < 0 || slot > AZX_TASKS_MAX_TASKS  -1 )
  {
    AZX_LOG_ERROR("task id %d not valid!\r\n", TaskProcID);
    return AZX_TASKS_INVALID_ID_ERR;
  }

  if(m2mb_tasks.task_slots[slot].SlotInUse == 0)
  {
    AZX_LOG_ERROR("task id %d not existing\r\n", TaskProcID);
    return AZX_TASKS_ID_NOT_DEFINED_ERR;
  }

  Queue_H = m2mb_tasks.task_slots[slot].Task_Queue_H;


  tmpMsg.type = type;
  tmpMsg.param1 = param1;
  tmpMsg.param2 = param2;

  AZX_LOG_TRACE( "message ==> type=%d; par1=%d; par2=%d\r\n", tmpMsg.type, tmpMsg.param1, tmpMsg.param2 );
  osRes = m2mb_os_q_tx( Queue_H, (void*)&tmpMsg, M2MB_OS_NO_WAIT, 0 );

  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_LOG_ERROR( "Send message to Task %d failed; error %d\r\n", TaskProcID, osRes );
    return AZX_TASKS_MSG_SEND_ERR;  // failure
  }
  else
  {
    AZX_LOG_TRACE("Message sent.\r\n");
    return AZX_TASKS_OK;  // success
  }
}


static INT32 azx_tasks_onTaskCompl(INT32 taskID, INT32 type, INT32 res)
{
  /* Task ID  with type parameter returned value %res */
  if(m2mb_tasks.complCB)
  {
    return m2mb_tasks.complCB(taskID, type, res);
  }
  return 0;
}


/*
 *  Task --> EntryFn
 */
void Task_EntryFn( void *arg )
{
  M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();
  AZX_TASKS_MESSAGE_T inPars;

  INT32 slot = (INT32)arg;
  MEM_W  task_name = 0;

  INT32 res = 0;
  if (slot == -1)
  {
    AZX_LOG_ERROR("Cannot find slot for handle %d\r\n", taskHandle);
    return;
  }

  AZX_LOG_TRACE("slot: %d\r\n", slot);


  m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &task_name, NULL );

  AZX_LOG_TRACE( "\r\n==>Task %s started!\r\n", (char*) task_name);

  while( 1 )
  {
     if(M2MB_OS_SUCCESS != m2mb_os_q_rx( m2mb_tasks.task_slots[slot].Task_Queue_H, (void*)&inPars, M2MB_OS_WAIT_FOREVER ))
     {
       break;
     }

     AZX_LOG_TRACE( "%s received a message: \r\n"
             "- type   = %d\r\n"
             "- param1 = %d\r\n"
             "- param2 = %d\r\n\r\n",
             (char*) task_name, inPars.type, inPars.param1, inPars.param2 );
     res = m2mb_tasks.task_slots[slot].Task_UserCB( inPars.type, inPars.param1, inPars.param2 );
     azx_tasks_onTaskCompl(slot + 1, inPars.type, res);
   }

  AZX_LOG_TRACE("exiting entry function. \r\n");
}


INT32 azx_tasks_createTask( char *task_name, INT32 stack_size, INT32 priority, INT32 msg_q_size, USER_TASK_CB cb)
{
  M2MB_OS_RESULT_E os_res;

  M2MB_OS_TASK_ATTR_HANDLE Task_Attr_H;
  M2MB_OS_Q_ATTR_HANDLE Task_Queue_Attr_H;

  INT32 task_prio;
  UINT32 queue_area_size;
  INT32 slot;



  if (! m2mb_tasks.isInit)
  {
    AZX_LOG_ERROR("m2m task first init not performed yet\r\n");
    return AZX_TASKS_NOTINIT_ERR;
  }

  if (priority < AZX_TASKS_PRIORITY_MAX || priority > AZX_TASKS_PRIORITY_MIN)
  {
    AZX_LOG_ERROR("priority out of bounds\r\n");
    return AZX_TASKS_WRONG_PRIO_ERR;
  }
  else
  {
    task_prio = priority + 200; //201 to 250
  }

  if (stack_size < AZX_TASKS_MIN_STACK_SIZE || stack_size > AZX_TASKS_MAX_STACK_SIZE)  //1 to 64KB
  {
    AZX_LOG_ERROR("stack size out of bounds\r\n");
    return AZX_TASKS_STACK_SIZE_ERR;
  }

  if (msg_q_size < AZX_TASKS_MIN_QUEUE_SIZE || msg_q_size > AZX_TASKS_MAX_QUEUE_SIZE)
  {
    AZX_LOG_ERROR("message queue size out of bounds\r\n");
    return AZX_TASKS_MSG_Q_SIZE_ERR;
  }

  queue_area_size = msg_q_size * BYTES_FOR_MSG(AZX_TASKS_MESSAGE_T);

  //input parameters are valid, now get a free slot.
  slot = find_free_task_slot();

  if (slot == -1)
  {
    AZX_LOG_ERROR("No free slots!\r\n");
    return AZX_TASKS_NO_FREE_SLOTS_ERR;
  }

  memset(m2mb_tasks.task_slots[slot].Task_NameBuf,0, AZX_TASKS_TASK_NAME_SIZE);

  if (task_name)
  {
    strncpy(m2mb_tasks.task_slots[slot].Task_NameBuf, task_name, AZX_TASKS_TASK_NAME_SIZE - 1);
  }
  else
  {
    sprintf(m2mb_tasks.task_slots[slot].Task_NameBuf, "Task%d", slot + 1);
  }

  AZX_LOG_TRACE("task_name_buf: %s\r\n", m2mb_tasks.task_slots[slot].Task_NameBuf);

  m2mb_tasks.task_slots[slot].Task_UserCB = cb;

  AZX_LOG_TRACE( "Create task messages queue\r\n" );
  if ( m2mb_os_q_setAttrItem( &Task_Queue_Attr_H, 1,M2MB_OS_Q_SEL_CMD_CREATE_ATTR,NULL) != M2MB_OS_SUCCESS )
  {
    AZX_LOG_ERROR( "SET QUEUE ATTRIBUTES - FAIL  --> SKIP QUEUE CREATION !!!\r\n" );
    m2mb_tasks.task_slots[slot].SlotInUse = 0;
    return AZX_TASKS_Q_ATTRIB_SET_ERR;
  }
  else
  {
    os_res = m2mb_os_q_setAttrItem( &Task_Queue_Attr_H,
        CMDS_ARGS
        (
            M2MB_OS_Q_SEL_CMD_MSG_SIZE, WORD32_FOR_MSG(AZX_TASKS_MESSAGE_T),
            M2MB_OS_Q_SEL_CMD_QSIZE, queue_area_size
        ));
    if ( M2MB_OS_SUCCESS != os_res )
    {
      AZX_LOG_ERROR( "SET QUEUE ATTRIBUTES - FAIL\r\n" );
      m2mb_tasks.task_slots[slot].SlotInUse = 0;
      return AZX_TASKS_Q_ATTRIB_SET_ERR;
    }
    else
    {
      AZX_LOG_TRACE( "Set queue attributes done\r\n" );

      os_res = m2mb_os_q_init( &(m2mb_tasks.task_slots[slot].Task_Queue_H), &Task_Queue_Attr_H  );
      if ( M2MB_OS_SUCCESS != os_res )
      {
        AZX_LOG_ERROR( "QUEUE CREATE - FAIL\r\n" );
        m2mb_os_q_setAttrItem( &Task_Queue_Attr_H, 1,M2MB_OS_Q_SEL_CMD_DEL_ATTR,NULL);
        m2mb_tasks.task_slots[slot].SlotInUse = 0;
        return AZX_TASKS_Q_INIT_ERR;
      }
      else
      {
        AZX_LOG_TRACE( "QUEUE CREATE - PASS\r\n" );
        os_res = m2mb_os_taskSetAttrItem( &Task_Attr_H,
                          CMDS_ARGS(
                               M2MB_OS_TASK_SEL_CMD_CREATE_ATTR, NULL,
                               M2MB_OS_TASK_SEL_CMD_STACK_SIZE, (void*)stack_size,
                               M2MB_OS_TASK_SEL_CMD_NAME, m2mb_tasks.task_slots[slot].Task_NameBuf,
                               M2MB_OS_TASK_SEL_CMD_PRIORITY, task_prio,      /* Use priority values above 200. max value: 255 (less priority) */
                               M2MB_OS_TASK_SEL_CMD_PREEMPTIONTH, task_prio,
                               M2MB_OS_TASK_SEL_CMD_AUTOSTART, M2MB_OS_TASK_AUTOSTART,
                               M2MB_OS_TASK_SEL_CMD_USRNAME, m2mb_tasks.task_slots[slot].Task_NameBuf
                               )
                        );
        if ( M2MB_OS_SUCCESS != os_res )
        {
          AZX_LOG_ERROR( "SET TASK ATTRIBUTES - FAIL\r\n" );
          m2mb_tasks.task_slots[slot].SlotInUse = 0;
          return AZX_TASKS_ATTRIB_SET_ERR;
        }
        else
        {
          AZX_LOG_TRACE( "SET TASK ATTRIBUTES - PASS\r\n" );
        }

        AZX_LOG_TRACE( "Creating the task\r\n" );
        os_res = m2mb_os_taskCreate(
                  &(m2mb_tasks.task_slots[slot].Task_H),
                  &Task_Attr_H,
                  &Task_EntryFn,
                  (void*)( (INT32) slot )
              );

        if ( M2MB_OS_SUCCESS != os_res )
        {
          AZX_LOG_ERROR( "TASK CREATION FAIL\r\n" );
          m2mb_os_taskSetAttrItem( &Task_Attr_H, 1,M2MB_OS_TASK_SEL_CMD_DEL_ATTR,NULL);
          m2mb_tasks.task_slots[slot].SlotInUse = 0;
          return AZX_TASKS_CREATE_ERR;
        }
        else
        {
          if ( M2MB_OS_TASK_INVALID == m2mb_tasks.task_slots[slot].Task_H )
          {
            AZX_LOG_ERROR( "INVALID TASK HANDLE - FAIL\r\n" );
            m2mb_tasks.task_slots[slot].SlotInUse = 0;
            return -1;
          }
          AZX_LOG_TRACE("Task created successfully with handle %p\r\n", m2mb_tasks.task_slots[slot].Task_H);

          return slot + 1;
        }

      }
    }
  }
}



INT32 azx_tasks_destroyTask(INT32 TaskProcID)
{
  INT32 slot = TaskProcID - 1;
  M2MB_OS_RESULT_E res;
  MEM_W  out = 0;

  AZX_LOG_TRACE("destroy...\r\n");
  if (! m2mb_tasks.isInit)
  {
    AZX_LOG_ERROR("m2m task first init not performed yet\r\n");
    return AZX_TASKS_NOTINIT_ERR;
  }

  if (slot < 0 || slot > AZX_TASKS_MAX_TASKS  -1 )
  {
    AZX_LOG_ERROR("task id %d not valid!\r\n", TaskProcID);
    return AZX_TASKS_INVALID_ID_ERR;
  }

  if(m2mb_tasks.task_slots[slot].SlotInUse == 0)
  {
    AZX_LOG_ERROR("task id %d not existing\r\n", TaskProcID);
    return AZX_TASKS_ID_NOT_DEFINED_ERR;
  }

  AZX_LOG_TRACE("Trying to terminate task...\r\n");
  res = m2mb_os_taskTerminate(m2mb_tasks.task_slots[slot].Task_H);
  if (res != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("Cannot terminate task %d\r\n", TaskProcID);
    return res;
  }

  m2mb_os_taskGetItem( m2mb_tasks.task_slots[slot].Task_H, M2MB_OS_TASK_SEL_CMD_STATE, &out, NULL );
  AZX_LOG_TRACE ( "Task state: %d\r\n", out );

  AZX_LOG_TRACE("Trying to delete task with handle %d...\r\n", m2mb_tasks.task_slots[slot].Task_H);
  res = m2mb_os_taskDelete(m2mb_tasks.task_slots[slot].Task_H);
  if (res != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("Cannot delete task %d\r\n", TaskProcID);
    return res;
  }

  AZX_LOG_TRACE("Trying to clear task queue...\r\n");
  res = m2mb_os_q_clear(m2mb_tasks.task_slots[slot].Task_Queue_H);
  if (res != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("Cannot clear task %d queue \r\n", TaskProcID);
    return res;
  }

  AZX_LOG_TRACE("Trying to deinit task queue...\r\n");
  res = m2mb_os_q_deinit(m2mb_tasks.task_slots[slot].Task_Queue_H);
  if (res != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("Cannot deinit task %d queue \r\n", TaskProcID);
    return res;
  }

  m2mb_tasks.task_slots[slot].Task_H = M2MB_OS_TASK_INVALID;
  m2mb_tasks.task_slots[slot].Task_Queue_H = M2MB_OS_Q_INVALID;
  m2mb_tasks.task_slots[slot].SlotInUse = 0;

  return 0;
}

INT32 azx_tasks_getEnqueuedCount( INT32 TaskProcID )
{
  INT32 slot = TaskProcID - 1;
  MEM_W  out = 0;
  M2MB_OS_Q_HANDLE queueHandle = m2mb_tasks.task_slots[slot].Task_Queue_H;
  M2MB_OS_RESULT_E result = m2mb_os_q_getItem( queueHandle, M2MB_OS_Q_SEL_CMD_ENQUEUED, &out, NULL );
  if(result != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("Cannot read enqueued message count: %d\r\n", result);
    return -1;
  }

  return (INT32)out;
}


INT32 azx_tasks_suspendTask( INT32 TaskProcID )
{
  M2MB_OS_RESULT_E res;
  INT32 slot = TaskProcID - 1;
  if (! m2mb_tasks.isInit)
  {
    AZX_LOG_ERROR("m2m task first init not performed yet\r\n");
    return AZX_TASKS_NOTINIT_ERR;
  }
  
  res = m2mb_os_taskSuspend(m2mb_tasks.task_slots[slot].Task_H);
  if (res != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("Cannot suspend task %d\r\n", TaskProcID);
    return AZX_TASKS_INTERNAL_ERR;
  }
  
  return AZX_TASKS_OK;  // success
}

INT32 azx_tasks_resumeTask( INT32 TaskProcID )
{
  M2MB_OS_RESULT_E res;
  INT32 slot = TaskProcID - 1;
  if (! m2mb_tasks.isInit)
  {
    AZX_LOG_ERROR("m2m task first init not performed yet\r\n");
    return AZX_TASKS_NOTINIT_ERR;
  }
  
  res = m2mb_os_taskResume(m2mb_tasks.task_slots[slot].Task_H);
  if (res != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("Cannot resume task %d\r\n", TaskProcID);
    return AZX_TASKS_INTERNAL_ERR;
  }
  return AZX_TASKS_OK;  // success
}


M2MB_OS_TASK_HANDLE azx_tasks_getM2MBTaskHandleById(INT32 TaskProcID)
{
  INT32 slot = TaskProcID - 1;
  if (! m2mb_tasks.isInit)
  {
    AZX_LOG_ERROR("m2m task first init not performed yet\r\n");
    return NULL;
  }
  
  if (slot >= 0)
  {
    AZX_LOG_TRACE("Looking for task id: %d\r\n", TaskProcID);

    if (m2mb_tasks.task_slots[slot].SlotInUse == 0)
    {
      return NULL;
    }
    return m2mb_tasks.task_slots[slot].Task_H;
  }
  return M2MB_OS_TASK_INVALID;
}