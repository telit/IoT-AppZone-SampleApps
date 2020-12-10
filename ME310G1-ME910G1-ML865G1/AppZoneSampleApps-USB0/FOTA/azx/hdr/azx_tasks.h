/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

#ifndef HDR_AZX_TASKS_H_
#define HDR_AZX_TASKS_H_
/**
 * @file azx_tasks.h
 * @version 1.0.4
 * @dependencies core/azx_log core/azx_utils
 * @author Alessio Quieti
 * @date 07/04/2018
 *
 * @brief Tasks related utilities
 *
 * Functions usable to simplify the tasks creation and usage
 * (instead of directly using m2mb_os_q.h low level APIs, which in turn allow
 * much greater control of the tasks)
 *
 * The library is modelled after the way tasks were handled in the old M2M API.
 */
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "azx_log.h"

/** @defgroup taskUsage Usage of tasks example functionalities
 * Functions usable to simplify the tasks creation and usage
*/

/**
 * @brief Task user callback signature
 *
 * This is the structure of the task user callback. Each time a message is
 * received by an user task, a callback with this structure will be called.
 *
 * It is responsibility of the user to define the callback logic.
 *
 * @ingroup taskUsage
 *
 * @see azx_tasks_init()
*/
typedef INT32 (*USER_TASK_CB)(INT32, INT32, INT32);

/** @addtogroup  taskUsage
@{ */

/**
 * @name Stack defines
 * @brief Available ranges for tasks
 *  @{ */
#define AZX_TASKS_MIN_STACK_SIZE 1024  /**<Minimum task stack size in bytes*/
#define AZX_TASKS_MAX_STACK_SIZE 32768 /**<Maximum task stack size in bytes*/

#define AZX_TASKS_MIN_QUEUE_SIZE 1     /**<Minimum task message queue size in slots*/
#define AZX_TASKS_MAX_QUEUE_SIZE 100   /**<Maximum task message queue size in slots*/

#define AZX_TASKS_PRIORITY_MAX   1     /**<Maximum task priority value */
#define AZX_TASKS_PRIORITY_MIN  32     /**<Minimum task priority value*/

#define AZX_TASKS_MAX_TASKS 32         /**< Maximum allowed tasks number */

#define AZX_TASKS_QUEUE_MSG_SIZE 3     /**< Size in Words (4 bytes) of each queue message ->
                                         `struct (INT32 type, INT32 param1, INT32 param2)` */
#define AZX_TASKS_TASK_NAME_SIZE 64    /**<Maximum task name length*/
  /** @} */
/** @} */

/**
* @brief Task related return codes
* @ingroup taskUsage
*/
typedef enum
{
  AZX_TASKS_OK = 1,                   /**<Success*/

  AZX_TASKS_NOTINIT_ERR = -1,         /**<Task structures not initialized, see azx_tasks_init()*/
  AZX_TASKS_WRONG_PRIO_ERR = -2,      /**<Priority set in task creation out of bounds*/
  AZX_TASKS_STACK_SIZE_ERR = -3,      /**<Task stack size set in creation was out of bounds*/
  AZX_TASKS_MSG_Q_SIZE_ERR = -4,      /**<Task queue size set in creation was out of bounds*/
  AZX_TASKS_NO_FREE_SLOTS_ERR = -5,   /**<All of the tasks slots (32) are in use, cannot create more tasks*/
  AZX_TASKS_ALLOC_ERR = -6,           /**<Task resources allocation failed*/
  AZX_TASKS_Q_ATTRIB_SET_ERR = -7,    /**<Task message queue attributes set failed*/
  AZX_TASKS_Q_INIT_ERR = -8,          /**<Task message queue init failed*/
  AZX_TASKS_ATTRIB_SET_ERR = -9,      /**<Task attributes set failed*/
  AZX_TASKS_CREATE_ERR = -10,         /**<Task creation failed*/

  AZX_TASKS_INVALID_ID_ERR = -20,     /**<Task id is not valid (out of bounds)*/
  AZX_TASKS_ID_NOT_DEFINED_ERR = -21, /**<Task id does not refer to a valid task*/
  AZX_TASKS_MSG_SEND_ERR = -22        /**<Error when sending a message to task queue*/
} AZX_TASKS_ERR_E;

/**
* @brief Task convenient task size values
* @ingroup taskUsage
*/
typedef enum
{
  AZX_TASKS_STACK_S = 2048,      /**<2KB of stack size */
  AZX_TASKS_STACK_M = 4096,      /**< 4KB  of stack size*/
  AZX_TASKS_STACK_L = 8192,      /**< 8KB  of stack size*/
  AZX_TASKS_STACK_XL = 16384,    /**< 16KB  of stack size*/

  M2M_OS_TASK_STACK_LIMIT = 32768 /**< 32K  of stack size. Consider this as a max limit*/

}AZX_TASKS_STACK_SIZE;

/**
* @brief Task convenient message box size values
* @ingroup taskUsage
*/
typedef enum
{
   AZX_TASKS_MBOX_S = 10,  /**<Task message box of 10 messages */
   AZX_TASKS_MBOX_M = 50,  /**<Task message box of 50 messages */
   AZX_TASKS_MBOX_L = 100, /**<Task message box of 100 messages */

   AZX_TASKS_MBOX_LIMIT = AZX_TASKS_MBOX_L /**<Consider this as a max limit */
}AZX_TASKS_MBOX_SIZE;

/**
 * @brief Structure holding the task message data
 *
 * This structure holds the parameters passed with a task message
 *
 * @ingroup taskUsage
 *
 * @see m2mb_send_message_to_task()
*/
typedef struct
{
   INT32  type;   /**<Type of message*/
   INT32  param1; /**<Parameter 1*/
   INT32  param2; /**<Parameter 2*/
} AZX_TASKS_MESSAGE_T;

/** @private */
typedef struct
{
  M2MB_OS_TASK_HANDLE Task_H;      /**<Task m2mb handler*/
  M2MB_OS_Q_HANDLE Task_Queue_H;   /**<Task queue m2mb handler*/
  USER_TASK_CB Task_UserCB;        /**<Task user provided callback function */
  UINT8 SlotInUse;                 /**<Is task slot already in use?*/
  CHAR Task_NameBuf[AZX_TASKS_TASK_NAME_SIZE * sizeof(CHAR)];  /**<Task name*/
} AZX_TASKS_SLOT_T;


/** @private */
typedef struct
{
  AZX_TASKS_SLOT_T task_slots[AZX_TASKS_MAX_TASKS];
  M2MB_OS_TASK_HANDLE M2MMain_Handle;
  INT8 isInit;
} _AZX_TASKS_PARAMS;

/**
 * @brief Initializes the parameters needed to use tasks.
 *
 * @return AZX_TASKS_OK
 *
 * @ingroup taskUsage
*/
INT32 azx_tasks_init(void);
/** \example{lineno} azx_tasks_example.c
  * This is a detailed example of tasks functions usage.
  */


/**
 * @brief Creates a new user task.
 *
 * Creates a new user task with the provided stack size, priority, name and
 * message queue size.
 *
 * Calling \ref azx_tasks_init() is mandatory before using this.
 *
 * @param[in] task_name Name that will be applied to new task. If NULL, the
 *     default name `"Task<ID_number>"` will be given, where `<ID_number>` is
 *     the id of the created task. Max length 64.
 * @param[in] stack_size Task stack size in bytes. Can be any value, within the
 *     range: @ref AZX_TASKS_MIN_STACK_SIZE - @ref AZX_TASKS_MAX_STACK_SIZE
 * @param[in] priority Task priority. The greater the value, the smaller the
 *     priority of the task. Accepted range:
 *     @ref AZX_TASKS_PRIORITY_MAX - @ref AZX_TASKS_PRIORITY_MIN
 * @param[in] msg_q_size Task message queue slots number. Accepted range:
 *     @ref AZX_TASKS_MIN_QUEUE_SIZE - @ref AZX_TASKS_MAX_QUEUE_SIZE
 * @param[in] cb The user callback that will be executed every time a message is
 *     received in task message queue. See @ref USER_TASK_CB for signature.
 *
 * @return One of
 *     Task id number from 1 to 32 on success
 *     @ref AZX_TASKS_INVALID_ID_ERR
 *     @ref AZX_TASKS_ID_NOT_DEFINED_ERR
 *     @ref AZX_TASKS_WRONG_PRIO_ERR
 *     @ref AZX_TASKS_STACK_SIZE_ERR
 *     @ref AZX_TASKS_MSG_Q_SIZE_ERR
 *     @ref AZX_TASKS_NO_FREE_SLOTS_ERR
 *     @ref AZX_TASKS_ALLOC_ERR
 *     @ref AZX_TASKS_Q_ATTRIB_SET_ERR
 *     @ref AZX_TASKS_Q_INIT_ERR
 *     @ref AZX_TASKS_ATTRIB_SET_ERR
 *     @ref AZX_TASKS_CREATE_ERR
 *     -1 on internal failure.
 *
 * @see azx_tasks_init()
 *
 * @ingroup taskUsage
 */
INT32 azx_tasks_createTask( CHAR *task_name, INT32 stack_size, INT32 priority,
    INT32 msg_q_size, USER_TASK_CB cb);

/**
 * @brief Destroys an user task.
 *
 * This will destroy the specified user task.
 *
 * @param[in] task_id The task id number to be destroyed,
 *     range: 1 - @ref AZX_TASKS_MAX_TASKS
 *
 * @return One of:
 *     @ref AZX_TASKS_OK
 *     @ref AZX_TASKS_INVALID_ID_ERR
 *     @ref AZX_TASKS_ID_NOT_DEFINED_ERR
 *     One of @ref M2MB_OS_RESULT_E values
 *
 * @see azx_tasks_init()
 *
 *  @ingroup taskUsage
*/
INT32 azx_tasks_destroyTask(INT8 task_id);

/**
 * @brief Sends a message to a task
 *
 * Sends a message to the provided task id, passing 3 integer parameters as
 * message parameters.
 *
 * @param[in] task_id The task id to receive the message
 *     range: 1 - @ref AZX_TASKS_MAX_TASKS
 * @param[in] type User parameter
 * @param[in] param1 User parameter
 * @param[in] param2 User parameter
 *
 * @return One of
 *     @ref AZX_TASKS_OK
 *     @ref AZX_TASKS_INVALID_ID_ERR
 *     @ref AZX_TASKS_ID_NOT_DEFINED_ERR
 *     @ref AZX_TASKS_MSG_SEND_ERR
 *
 * @see azx_tasks_init()
 *
 * @ingroup taskUsage
*/
INT32 azx_tasks_sendMessageToTask( INT8 task_id, INT32 type, INT32 param1, INT32 param2 );


/**
 * @brief Retrieves the current task ID value
 *
 * Retrieves the value of the current running task id (1-32)
 *
 * @return The current the task ID;
 *
 * @see azx_tasks_init()
 * @see azx_tasks_createTask()
 *
 * @ingroup taskUsage
*/
INT32 azx_tasks_getCurrentTaskId( void );

/**
 * @brief Retrieves the current task name string
 *
 * @param[out] name The current task name.
 *
 * @return The current the task name pointer (same as the input parameter).
 *
 * @see azx_tasks_init()
 * @see azx_tasks_createTask()
 *
 * @ingroup taskUsage
*/
CHAR* azx_tasks_getCurrentTaskName( CHAR *name );

/**
 * Returns the number of messages in the queue of a task.
 *
 * @param[in] The ID of the task whose queue should be checked.
 *
 * @return The number of messages that are queued. In case of error, -1 is returned.
 */
INT32 azx_tasks_getEnqueuedCount( INT8 task_id );

#endif /* HDR_AZX_TASKS_H_ */
