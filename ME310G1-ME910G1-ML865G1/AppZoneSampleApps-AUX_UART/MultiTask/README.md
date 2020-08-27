
###MultiTask

Sample application showcasing multi tasking functionalities with M2MB API. Debug prints on **AUX UART**


**Features**


- How to create tasks using azx utilities
- How to use send messages to tasks
- How to use a semaphore to synchronize two tasks


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create three tasks with the provided utility (this calls public m2mb APIs)

- Send a message to the task1, its callback function azx_msgTask1 will be called

**`azx_msgTask1`**

- Print received parameters from main
- Send modified parameters to task2 \(its callback function azx_msgTask2 will be called\)
- wait for an InterProcess Communication semaphore to be available \(released by task3\)
- Once the semaphore is available, print a message and return

**`azx_msgTask2`**

- Print received parameters from caller
- If first parameter is bigger than a certain value, Send modified parameters to task3
- Else, use the second parameter as a task handle and print the corresponding name plus the value of the first parameter

**`azx_msgTask3`**

- Print received parameters from task 2
- release IPC semaphore
- send message to task 2 with first parameter below the threshold and second parameter with task3 handle


![](../../pictures/samples/multitask_bordered.png)

---------------------

