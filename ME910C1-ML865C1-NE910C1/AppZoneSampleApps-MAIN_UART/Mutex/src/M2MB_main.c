/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showing mutex usage, with ownership and prioritization usage. Debug prints on MAIN UART
  @version 
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    30/06/2020
 */

/* Include files ================================================================================*/
#include <stdio.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "azx_log.h"
#include "azx_tasks.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/

#define BUFSIZE 30

/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
static M2MB_OS_MTX_HANDLE   mtxHandle;

static M2MB_OS_RESULT_E res;
static INT32 shared_buffer[BUFSIZE];
static INT32 buffer_size=0; //last available buffer position
static INT32 MAX_SIZE=BUFSIZE-1;

static INT32 freepos=0; //these are used for hptf test case
static INT32 evaluate[3]={0,0,0};

/*Label tasks for convenience*/
static INT8 PRODUCER, CONSUMER, SUPPORT, SUPPORT2;






/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/


M2MB_RESULT_E mutex_init(void)
{
  /***********************************************************************************
   * Initialize the mutex
   **********************************************************************************/
  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_MTX_ATTR_HANDLE mtxHandleAttr;
  UINT32                  inheritVal = 0;

  osRes = m2mb_os_mtx_setAttrItem( &mtxHandleAttr,
      CMDS_ARGS(
          M2MB_OS_MTX_SEL_CMD_CREATE_ATTR, NULL,
          M2MB_OS_MTX_SEL_CMD_NAME, "Mutex",
          M2MB_OS_MTX_SEL_CMD_INHERIT, inheritVal
      )
  );
  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_os_mtx_setAttrItem failed, code %d\r\n", osRes );
    return M2MB_RESULT_FAIL;
  }

  osRes=m2mb_os_mtx_init( &mtxHandle, &mtxHandleAttr);
  if(osRes==M2MB_OS_SUCCESS)
    AZX_LOG_DEBUG( "[MUTEX] Mutex initialized\r\n" );
  else
  {
    AZX_LOG_ERROR( "Mutex uninitialized\r\n" );
    return M2MB_RESULT_FAIL;
  }

  return M2MB_RESULT_SUCCESS;

}


INT32 msgProducer(INT32 type, INT32 param1, INT32 param2)
{
  (void) param2;
  switch(type)
  {
  case 0:
  {
    /***********************************************************************************
     * This task produces a resource and it puts it in shared_buffer
     **********************************************************************************/

    res=m2mb_os_mtx_get( mtxHandle, M2MB_OS_MS2TICKS( 10000 ));

    switch(res)
    {
    case M2MB_OS_SUCCESS:

      AZX_LOG_DEBUG("Mutex acquired\r\n" );
      AZX_LOG_DEBUG("Now there are %d items\r\n",buffer_size);

      if(buffer_size<MAX_SIZE)
      {
        shared_buffer[buffer_size]=99;
        AZX_LOG_DEBUG("Produced item %d at index %d\r\n",shared_buffer[buffer_size], buffer_size);
        buffer_size++;

      }
      else
      {
        AZX_LOG_DEBUG("Can't produce anything, buffer is full %d\r\n", buffer_size);
      }
      break;

    case M2MB_OS_DELETED:

      AZX_LOG_ERROR( "Mutex was deleted\r\n" );
      break;

    case M2MB_OS_NOT_AVAILABLE:

      AZX_LOG_ERROR( "Mutex is not available\r\n" );
      break;

    case M2MB_OS_WAIT_ABORTED:
      AZX_LOG_ERROR("Mutex timeout\r\n" );
      break;

    case M2MB_OS_MUTEX_ERROR:
      AZX_LOG_ERROR("Mutex error\r\n" );
      break;

    case M2MB_OS_WAIT_ERROR:
      AZX_LOG_ERROR("Mutex wait error\r\n" );
      break;

    case M2MB_OS_CALLER_ERROR:
      AZX_LOG_ERROR("Mutex caller error\r\n" );
      break;
    default:
     AZX_LOG_ERROR("Generic error %d\r\n", res );
      break;
    }

    res= m2mb_os_mtx_hptf( mtxHandle );
    if(res!=M2MB_OS_SUCCESS)
    {
      AZX_LOG_ERROR("m2mb_os_mtx_hptf FAILED w/ code %d\r\n",res );
    }


    res= m2mb_os_mtx_put( mtxHandle );
    switch(res)
    {
    case M2MB_OS_SUCCESS:
      AZX_LOG_DEBUG( "Mutex released\r\n" );
      break;

    case M2MB_OS_DELETED:
      AZX_LOG_ERROR("Mutex was deleted\r\n" );
      break;

    case M2MB_OS_NOT_AVAILABLE:

      AZX_LOG_ERROR("Mutex is not available\r\n" );
      break;

    case M2MB_OS_WAIT_ABORTED:
      AZX_LOG_ERROR("Mutex timeout\r\n" );
      break;

    case M2MB_OS_MUTEX_ERROR:
      AZX_LOG_ERROR("Mutex error\r\n" );
      break;

    case M2MB_OS_WAIT_ERROR:
      AZX_LOG_ERROR("Mutex wait error\r\n" );
      break;

    case M2MB_OS_CALLER_ERROR:
      AZX_LOG_ERROR("Mutex caller error\r\n" );
      break;
    default:
     AZX_LOG_ERROR("Generic error %d\r\n", res );
      break;
    }

    m2mb_os_taskSleep( M2MB_OS_MS2TICKS( param1 ) );
    azx_tasks_sendMessageToTask( PRODUCER, 0, param1, 0 );

    break;
  }

  case 1:
  {  /*
   * TEST HPTF
   */
    res=m2mb_os_mtx_get( mtxHandle, M2MB_OS_MS2TICKS( 100000 ));
    if(freepos >=0 && freepos<4)
    {

      evaluate[freepos]=1;
      AZX_LOG_DEBUG("producer: freepos = %d | evaluate[freepos]= %d \r\n",freepos,evaluate[freepos]);
      freepos++;
    }
    res=m2mb_os_mtx_put(mtxHandle);
    break;
  }
  }
  return(0);
}


INT32 msgConsumer(INT32 type, INT32 param1, INT32 param2)
{
  /***********************************************************************************
   * This task consumes a resource from shared_buffer
   **********************************************************************************/
  (void) param1;
  (void) param2;
  switch(type)
  {
  case 0:
    res=m2mb_os_mtx_get( mtxHandle, M2MB_OS_MS2TICKS( 10000 ));

    switch(res)
    {

    case M2MB_OS_SUCCESS:
      AZX_LOG_DEBUG( "Mutex acquired\r\n" );
      AZX_LOG_DEBUG("Now there are %d items\r\n",buffer_size);
      if(buffer_size>0 )
      {
        if(shared_buffer[buffer_size-1]==-1)
          AZX_LOG_ERROR("Buffer is out of bound\r\n" );
        else
        {
          /*
           * this acts like a FIFO queue
           */
          AZX_LOG_DEBUG("[CONSUMER]I consumed %d from index %d\r\n",shared_buffer[buffer_size-1], buffer_size - 1);

          buffer_size--;
        }
      }
      else
      {
        AZX_LOG_DEBUG("Can't consume anything, buffer size is %d \r\n",buffer_size);
      }

      break;

    case M2MB_OS_DELETED:

      AZX_LOG_ERROR("Mutex was deleted\r\n" );
      break;

    case M2MB_OS_NOT_AVAILABLE:

      AZX_LOG_ERROR("Mutex is not available\r\n" );
      break;

    case M2MB_OS_WAIT_ABORTED:
      AZX_LOG_ERROR("Mutex timeout\r\n" );
      break;

    case M2MB_OS_MUTEX_ERROR:
      AZX_LOG_ERROR("Mutex error\r\n" );
      break;

    case M2MB_OS_WAIT_ERROR:
      AZX_LOG_ERROR("Mutex wait error\r\n" );
      break;

    case M2MB_OS_CALLER_ERROR:
      AZX_LOG_ERROR("Mutex caller error\r\n" );
      break;
    default:
      AZX_LOG_ERROR("Generic error %d\r\n", res );
      break;
    }


    res= m2mb_os_mtx_put( mtxHandle );
    switch(res)
    {
    case M2MB_OS_SUCCESS:
      AZX_LOG_DEBUG( "Mutex released\r\n" );
      break;

    case M2MB_OS_DELETED:

      AZX_LOG_ERROR("Mutex was deleted\r\n" );
      break;

    case M2MB_OS_NOT_AVAILABLE:

      AZX_LOG_ERROR("Mutex is not available\r\n" );
      break;

    case M2MB_OS_WAIT_ABORTED:
      AZX_LOG_ERROR("Mutex timeout\r\n" );
      break;

    case M2MB_OS_MUTEX_ERROR:
      AZX_LOG_ERROR("Mutex error\r\n" );
      break;

    case M2MB_OS_WAIT_ERROR:
      AZX_LOG_ERROR("Mutex wait error\r\n" );
      break;

    case M2MB_OS_CALLER_ERROR:
      AZX_LOG_ERROR("Mutex caller error\r\n" );
      break;
    default:
      AZX_LOG_ERROR("Generic error %d\r\n", res );
      break;
    }


    m2mb_os_taskSleep( M2MB_OS_MS2TICKS( param1 ) );

    azx_tasks_sendMessageToTask( CONSUMER, 0, param1, 0 );
    break;


    case 1:

      /***********************************************************************************
       * HPTF TEST CASE
       **********************************************************************************/
      res=m2mb_os_mtx_get( mtxHandle, M2MB_OS_MS2TICKS( 100000 ));
      if(freepos >=0 && freepos<4)
      {

        evaluate[freepos]=2;
        AZX_LOG_DEBUG("consumer: freepos = %d | evaluate[freepos]= %d \r\n",freepos,evaluate[freepos]);
        freepos++;
      }
      res=m2mb_os_mtx_put(mtxHandle);
      break;
  }
  return(0);
}


INT32 msgSupport(INT32 type, INT32 param1, INT32 param2)
{
  (void) param1;
  (void) param2;
  
  switch(type)
  {
  case 0:
    break;

  case 1:
    /***********************************************************************************
     * HPTF TEST CASE
     **********************************************************************************/
    res = m2mb_os_mtx_get(mtxHandle,M2MB_OS_WAIT_FOREVER);
    if(freepos >=0 && freepos<4)
    {

      evaluate[freepos]=3;
      AZX_LOG_DEBUG("freepos = %d | evaluate[freepos]= %d \r\n",freepos,evaluate[freepos]);
      freepos++;
    }
    res=m2mb_os_mtx_put(mtxHandle);
    break;
  }
  return(0);
}

INT32 msgSupport2(INT32 type, INT32 param1, INT32 param2)
{
  (void) param1;
  (void) param2;
  
  switch(type)
  {
  case 0:
    break;

  case 1:

    /***********************************************************************************
     * HPTF TEST CASE
     **********************************************************************************/
    res=m2mb_os_mtx_get( mtxHandle, M2MB_OS_WAIT_FOREVER);
    if(freepos >=0 && freepos<4)
    {

      evaluate[freepos]=4;
      AZX_LOG_DEBUG("freepos = %d | evaluate[freepos]= %d \r\n",freepos,evaluate[freepos]);
      freepos++;
    }
    res=m2mb_os_mtx_put(mtxHandle);
    break;

  }

  return(0);
}



void killTasks(void)
{

  azx_tasks_destroyTask(CONSUMER);
  azx_tasks_destroyTask(PRODUCER);
  //forcibly release mutex
  res= m2mb_os_mtx_put( mtxHandle );
}

void restartTasks(void)
{
  PRODUCER = azx_tasks_createTask((char*) "PRODUCER", AZX_TASKS_STACK_L, 1, AZX_TASKS_MBOX_M, msgProducer);
  CONSUMER = azx_tasks_createTask((char*) "CONSUMER", AZX_TASKS_STACK_L, 1, AZX_TASKS_MBOX_M, msgConsumer);
}


/* Global functions =============================================================================*/

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

  azx_tasks_init();

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting MutEx app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);

  /*Producer and consumer has the same priority, support has priority 204 and support 2 has priority 205*/
  PRODUCER = azx_tasks_createTask((char*) "PRODUCER", AZX_TASKS_STACK_L, 1, AZX_TASKS_MBOX_M, msgProducer);
  CONSUMER = azx_tasks_createTask((char*) "CONSUMER", AZX_TASKS_STACK_L, 1, AZX_TASKS_MBOX_M, msgConsumer);
  SUPPORT = azx_tasks_createTask((char*) "HPTF_SUPPORT", AZX_TASKS_STACK_L,  4, AZX_TASKS_MBOX_M, msgSupport);
  SUPPORT2 = azx_tasks_createTask((char*) "HPTF_SUPPORT2", AZX_TASKS_STACK_L, 5, AZX_TASKS_MBOX_M, msgSupport2);

  if(PRODUCER<0 || CONSUMER < 0 || SUPPORT < 0 || SUPPORT2 < 0)
  {
    AZX_LOG_CRITICAL("Cannot initialize the tasks!\r\n");
    return;
  }

  /*Initialize mutex*/
  if(M2MB_RESULT_SUCCESS != mutex_init())
  {
    AZX_LOG_ERROR("Cannot initialize the mutex!\r\n");
    return;
  }

  /***********************************************************************************
   * Case 1: Producer and consumer have same idle time
   **********************************************************************************/
  AZX_LOG_INFO ("\r\n[CASE 1 ] Producer and consumer have same idle time\r\n\r\n");
  azx_tasks_sendMessageToTask( PRODUCER, 0, 1000, 0 );
  azx_tasks_sendMessageToTask( CONSUMER, 0, 1000, 0 );


  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 3000 ) );

  /*
   * Destroy PRODUCER and CONSUMER tasks so any pending message will be deleted,
   * and next demo case starts from a clean configuration*/
  killTasks();


  /***********************************************************************************
   * Case 2: Producer has double idle timer
   **********************************************************************************/
  AZX_LOG_INFO( "\r\n[CASE 2 ] Producer has double idle time\r\n\r\n");


  restartTasks();

  azx_tasks_sendMessageToTask( CONSUMER, 0, 1000, 0 );
  azx_tasks_sendMessageToTask( PRODUCER, 0, 2000, 0 );

  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 5000 ) );

  /*
   * Destroy PRODUCER and CONSUMER tasks so any pending message will be deleted,
   * and next demo case starts from a clean configuration*/
  killTasks();

  for(INT32 j=0; j<BUFSIZE-1;j++)
  {
    shared_buffer[j]=1;
  }
  buffer_size=0;

  /***********************************************************************************
   * Case 3: Producer has half idle timer
   **********************************************************************************/
  AZX_LOG_INFO( "\r\n[CASE 3 ] Producer has half idle time\r\n\r\n" );
  restartTasks();

  azx_tasks_sendMessageToTask( CONSUMER, 0, 2000, 0 );
  azx_tasks_sendMessageToTask( PRODUCER, 0, 1000, 0 );

  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 5000 ) );

  /*
   * Destroy PRODUCER and CONSUMER tasks so any pending message will be deleted,
   * and next demo case starts from a clean configuration*/
  killTasks();


  /***********************************************************************************
   * Case 4:
   * HTPF task3 has priority 204, task4 has priority 205
   * task1 is still at priority 201
   * The mutex is owned by the main task, so all other tasks will queue.
   * if HTPF is not used, the mutex queue order is 3,4,1
   * which is the order of task message sent
   *
   * NOTE:
   * The sleep after azx_tasks_sendMessageToTask is mandatory, otherwise
   * the main continues the execution, and the message is just in a queue but not executed.
   *
   * To force execution, put a sleep after task invocation.
   **********************************************************************************/

  AZX_LOG_INFO( "\r\n[CASE 4 ] NO HTPF\r\n\r\n");
  freepos=0;

  restartTasks();

  AZX_LOG_INFO("Reserve MUTEX so all tasks are enqueued\r\n");
  res = m2mb_os_mtx_get(mtxHandle, M2MB_OS_WAIT_FOREVER); //mutex is locked, who goes first?

  azx_tasks_sendMessageToTask( SUPPORT, 1, 0, 0);
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 1000 ) );

  azx_tasks_sendMessageToTask( SUPPORT2, 1, 0, 0);
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 1000 ) );

  azx_tasks_sendMessageToTask( PRODUCER, 1, 0, 0 );
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 1000 ) );

  res = m2mb_os_mtx_put(mtxHandle); //unlock and see what happens
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 5000 ) );


  /*
   * Destroy PRODUCER and CONSUMER tasks so any pending message will be deleted,
   * and next demo case starts from a clean configuration*/
  killTasks();


  AZX_LOG_DEBUG("EVALUATE SEQUENCE IS %d %d %d. Expected: 3 4 1 \r\n",evaluate[0],evaluate[1],evaluate[2]);


  if(evaluate[0]==3 && evaluate[1]==4 && evaluate[2]==1)
  {
    AZX_LOG_INFO("NO HTPF OK\r\n");
  }
  else
  {
    AZX_LOG_ERROR("NO HTPF fail.\r\n");
  }




  /***********************************************************************************
   * Case 4.1: HTPF task3 has priority 204 , task4 has priority 205
   * task1 is still at priority 201
   *
   * The mutex is owned by the main task, so all other tasks will queue.
   * in this case, HPTF is used so the execution flow shall be  1,3,4
   * because task1 has higher priority than 3 and 4.
   *
   * NOTE:
   * The sleep after azx_tasks_sendMessageToTask is mandatory, otherwise
   * the main continues the execution, and the message is just in a queue but not executed.
   *
   * To force execution, put a sleep after task invocation.
   **********************************************************************************/
  AZX_LOG_INFO( "\r\n[CASE 4.1 ] HTPF USED\r\n\r\n" );

  freepos=0;
  evaluate[0]=evaluate[1]=evaluate[2]=0;

  restartTasks();

  AZX_LOG_INFO("Reserve MUTEX so all tasks are enqueued\r\n");
  res = m2mb_os_mtx_get(mtxHandle,M2MB_OS_WAIT_FOREVER); //mutex locked

  azx_tasks_sendMessageToTask( SUPPORT, 1, 0, 0);
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 2000 ) );

  azx_tasks_sendMessageToTask( SUPPORT2, 1, 0, 0);
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 2000 ) );

  azx_tasks_sendMessageToTask( PRODUCER, 1, 0, 0 );
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 2000 ) );

  /*Force highest priority task as first in the queue*/
  res = m2mb_os_mtx_hptf(mtxHandle);
  if(res==M2MB_OS_SUCCESS)
  {
    AZX_LOG_INFO("m2mb_os_mtx_hptf OK\r\n");
  }
  else
  {
    AZX_LOG_ERROR("m2mb_os_mtx_hptf failed.%d\r\n", res);
  }

  //Now tasks will be released, with the highest priority one is the first in the queue
  res = m2mb_os_mtx_put(mtxHandle);
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS( 2000 ) );


  AZX_LOG_DEBUG("EVALUATE SEQUENCE IS %d %d %d, expected 1 3 4 \r\n",evaluate[0],evaluate[1],evaluate[2]);

  if(evaluate[0]==1 && evaluate[1]==3 && evaluate[2]==4)
  {
    AZX_LOG_INFO("HTPF DEMO OK\r\n");
  }
  else
  {
    AZX_LOG_ERROR("HTPF DEMO FAILED\r\n");
  }


  /************************************************************
   * Cleanup
   ************************************************************/

  azx_tasks_destroyTask(PRODUCER);
  azx_tasks_destroyTask(CONSUMER);
  azx_tasks_destroyTask(SUPPORT);
  azx_tasks_destroyTask(SUPPORT2);

  res=m2mb_os_mtx_deinit( mtxHandle);
  if(res!=M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("Mutex not deinitialized");
    return;
  }

  AZX_LOG_INFO("The application has ended...\r\n");

}

