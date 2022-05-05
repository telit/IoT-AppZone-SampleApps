
### Basic Hello World (Aux UART)

The application prints "Hello World!" on Auxiliary UART every 2 seconds using


**Features**


- How to open Auxiliary UART as an output channel
- How to print messages out of the channel


#### Application workflow

**`M2MB_main.c`**

- Open Auxiliary UART with **`m2mb_uart_open`** function
- write a welcome message using **`m2mb_uart_write`**
- write "Hello World!" every 2 seconds in a while loop, using **`m2mb_uart_write`**

![](../../pictures/samples/hello_world_basic_bordered.png)

---------------------

