/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application that opens a tunnel between main UART and USB0 port.
    Any data sent on one port will be received on the other, and vice-versa.
    The data read and write operations from both ports are managed with a simple dedicated task.
  @version
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()
  @author

  @date
    28/10/2021
*/

/* Include files ================================================================================*/
#include "stdio.h"
#include "string.h"

#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "m2mb_usb.h"
#include "m2mb_uart.h"


/* Local defines ================================================================================*/
#define TASKSTACK_SIZE 8*1024 /*8 kB of task stack size*/
#define TASKMSQ_Q_SIZE 5 /*up to 5 messages in task queue*/
#define TASK_PRIORITY 5 /*from 1 to 32 as an example*/

/* Local typedefs ===============================================================================*/

/*
 *
 * prototype of the task user callback function
 *
 * */
typedef INT32 (*USER_TASK_CB)(INT32, INT32);




/*
 *
 * Structure holding the required parameters for the running task
 *
 * */
typedef struct
{
  M2MB_OS_TASK_HANDLE Task_H;      /*Task m2mb handler*/
  M2MB_OS_Q_HANDLE Task_Queue_H;   /*Task queue m2mb handler*/
  USER_TASK_CB Task_UserCB;        /*Task user provided callback function */
} TASKS_ELEM_T;


/*
 *
 * Structure holding the task message parameters
 *
 * */
typedef struct
{
  INT32 cmd;
  INT32  datalen;
} AZX_TASKS_MESSAGE_T;


/* Local statics ================================================================================*/

/*
 *
 * Data task command options
 *
 * */
enum
{
  TASK_UART_READ_AND_USB_WRITE,
  TASK_USB_READ_AND_UART_WRITE
};


/*UART and USB global handles*/
static INT32 USBfd = -1;
static INT32 UARTfd = -1;


/*Task structure entry, will be used for the UART read operations*/
static TASKS_ELEM_T TaskElem;

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/



/*
 *
 *  The entry function for the data task. It will manage the messages dispatching to the user callback function
 *
 */
static void Task_EntryFn( void *arg )
{
  (void) arg;
  AZX_TASKS_MESSAGE_T inPars;


  while( 1 )
  {
    /*Wait for messages and pass them to user callback*/
    if(M2MB_OS_SUCCESS != m2mb_os_q_rx( TaskElem.Task_Queue_H, (void*)&inPars, M2MB_OS_WAIT_FOREVER ))
    {
      break;
    }
    /*send data to callback*/
    TaskElem.Task_UserCB( inPars.cmd, inPars.datalen );
  }

}

/*-----------------------------------------------------------------------------------------------*/

/*
 *
 * Allows to create the Data task, given an user CB function
 *
 * */
static int createTask( USER_TASK_CB cb)
{
  M2MB_OS_RESULT_E os_res;

  M2MB_OS_TASK_ATTR_HANDLE Task_Attr_H;
  M2MB_OS_Q_ATTR_HANDLE Task_Queue_Attr_H;
  UINT32 queue_area_size;

  INT32 stack_size = TASKSTACK_SIZE;
  INT32 msg_q_size = TASKMSQ_Q_SIZE;
  INT32 task_prio = 200 + TASK_PRIORITY; /*priority from 200 and above*/

  queue_area_size = msg_q_size * BYTES_FOR_MSG(AZX_TASKS_MESSAGE_T);


  if ( m2mb_os_q_setAttrItem( &Task_Queue_Attr_H, 1,M2MB_OS_Q_SEL_CMD_CREATE_ATTR,NULL) != M2MB_OS_SUCCESS )
  {
    return -1;
  }
  else
  {
    /*configure queue area to store msg_q_size messages*/
    os_res = m2mb_os_q_setAttrItem( &Task_Queue_Attr_H,
        CMDS_ARGS
        (
            M2MB_OS_Q_SEL_CMD_MSG_SIZE, WORD32_FOR_MSG(AZX_TASKS_MESSAGE_T),
            M2MB_OS_Q_SEL_CMD_QSIZE, queue_area_size
        ));
    if ( M2MB_OS_SUCCESS != os_res )
    {
      return -2;
    }
    else
    {
      /*init queue*/
      os_res = m2mb_os_q_init( &TaskElem.Task_Queue_H, &Task_Queue_Attr_H );
      if ( M2MB_OS_SUCCESS != os_res )
      {
        return -3;
      }
      else
      {

        /*Init task parameters*/
        os_res = m2mb_os_taskSetAttrItem( &Task_Attr_H,
            CMDS_ARGS(
                M2MB_OS_TASK_SEL_CMD_CREATE_ATTR, NULL,
                M2MB_OS_TASK_SEL_CMD_STACK_SIZE, (void*)stack_size,
                M2MB_OS_TASK_SEL_CMD_NAME, "TTY_READ",
                M2MB_OS_TASK_SEL_CMD_PRIORITY, task_prio,      /* Use priority values above 200. max value: 255 (less priority) */
                M2MB_OS_TASK_SEL_CMD_PREEMPTIONTH, task_prio,
                M2MB_OS_TASK_SEL_CMD_AUTOSTART, M2MB_OS_TASK_AUTOSTART
            )
        );
        if ( M2MB_OS_SUCCESS != os_res )
        {
          return -4;
        }

        /* Creating the task */
        os_res = m2mb_os_taskCreate(
            &(TaskElem.Task_H),
            &Task_Attr_H,
            &Task_EntryFn,
            (void*) NULL
        );

        if ( M2MB_OS_SUCCESS != os_res )
        {
          return -5;
        }
        else
        {
          if ( M2MB_OS_TASK_INVALID == TaskElem.Task_H )
          {

            return -6;
          }
          TaskElem.Task_UserCB = cb;
          return 0;
        }
      }
    }
  }
}

/*-----------------------------------------------------------------------------------------------*/

/*
 *
 * Sends a message to the Data task, passing command
 * and how much data is available to read
 *
 * */
static INT32 sendMessageToTask( INT32 cmd, INT32 datalen )
{
  AZX_TASKS_MESSAGE_T tmpMsg;
  M2MB_OS_RESULT_E osRes;


  tmpMsg.cmd = cmd;
  tmpMsg.datalen = datalen;

  osRes = m2mb_os_q_tx( TaskElem.Task_Queue_H, (void*)&tmpMsg, M2MB_OS_NO_WAIT, 0 );

  if( osRes != M2MB_OS_SUCCESS )
  {
    return -1;  // failure
  }
  else
  {
    return 0;  // success
  }
}

/*-----------------------------------------------------------------------------------------------*/


/*
 *
 * USB events callback
 *
 * */
static void USB_Cb( INT32 fd, M2MB_USB_IND_E usb_event, UINT16 resp_size, void *resp_struct, void *userdata )
{
  (void) fd;
  (void) resp_size;
  (void) userdata;

  UINT32 usbRxLen;

  if( M2MB_USB_RX_EVENT == usb_event )
  {
    /*The number of available bytes*/
    usbRxLen = *((UINT32 *)resp_struct);
    sendMessageToTask(TASK_USB_READ_AND_UART_WRITE, usbRxLen); /*this call will be executed by myTTYCB function */
  }
}

/*-----------------------------------------------------------------------------------------------*/

/*
 *
 * UART events callback
 *
 * */
static void UART_Cb( INT32 fd, M2MB_UART_IND_E uart_event, UINT16 resp_size, void *resp_struct, void *userdata )
{
  (void) fd;
  (void) resp_size;
  (void) userdata;
  UINT32 uartRxLen;

  if( M2MB_UART_RX_EV == uart_event )
  {
    uartRxLen = *((UINT32 *)resp_struct);
    sendMessageToTask(TASK_UART_READ_AND_USB_WRITE, uartRxLen); /*this call will be executed by myTTYCB function */
  }
}

/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/

/*
 *
 * The data management task. Depending on cmd, it will read from UART and write on USB or vice-versa
 *
 * */
INT32 dataTask_Cb(INT32 cmd, INT32 datalen)
{
  char * databuffer;
  INT32 read_data;

  databuffer = (char *) m2mb_os_malloc(datalen + 1);
  if(!databuffer)
  {
    m2mb_usb_write(USBfd, "--FAILED ALLOCATION\r\n", strlen("--FAILED ALLOCATION\r\n"));
    m2mb_usb_write(UARTfd, "--FAILED ALLOCATION\r\n", strlen("--FAILED ALLOCATION\r\n"));
    return -1;
  }

  switch(cmd)
  {
  case TASK_UART_READ_AND_USB_WRITE:
    read_data = m2mb_uart_read(UARTfd, databuffer, datalen);
    if(read_data > 0)
    {
      m2mb_usb_write(USBfd, databuffer, read_data);
    }
    break;
  case TASK_USB_READ_AND_UART_WRITE:
    read_data = m2mb_usb_read(USBfd, databuffer, datalen);
    if(read_data > 0)
    {
      m2mb_uart_write(UARTfd, databuffer, datalen);
    }
    break;

  }
  m2mb_os_free(databuffer);
  return 0;
}


/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void) argc;
  (void) argv;
  UARTfd = m2mb_uart_open( "/dev/tty0", 0 );
  USBfd = m2mb_usb_open("/dev/USB0", 0);

  if(UARTfd == -1 || USBfd == -1)
  {
    return;
  }

  if (0 != createTask(dataTask_Cb))
  {
    m2mb_usb_write(USBfd, "FAILED CREATING TASK\r\n", strlen("FAILED CREATING TASK\r\n"));
    return;
  }

  /*UART configuration*/
  m2mb_uart_ioctl(UARTfd, M2MB_UART_IOCTL_SET_TX_TIMEOUT, 10);
  m2mb_uart_ioctl(UARTfd, M2MB_UART_IOCTL_SET_RX_TIMEOUT, 10);
  m2mb_uart_ioctl(UARTfd, M2MB_UART_IOCTL_SET_CB_FN, UART_Cb);

  m2mb_uart_ioctl(UARTfd, M2MB_UART_IOCTL_SET_FCTL, M2MB_UART_FCTL_OFF);


  /*USB configuration*/

  m2mb_usb_ioctl(USBfd, M2MB_USB_IOCTL_SET_CB, USB_Cb);

  /*notify user that the two channels are ready*/
  m2mb_usb_write(USBfd, "READY", 5);
  m2mb_uart_write(UARTfd, "READY", 5);

}



