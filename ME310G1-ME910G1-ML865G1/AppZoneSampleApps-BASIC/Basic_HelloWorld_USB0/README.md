
### Basic Hello World (USB0)

The application prints "Hello World!" on USB 0 every 2 seconds using


**Features**


- How to open USB 0 as an output channel
- How to print messages out of the channel


#### Application workflow

**`M2MB_main.c`**

- Open USB 0 with **`m2mb_usb_open`** function
- write a welcome message using **`m2mb_usb_write`**
- write "Hello World!" every 2 seconds in a while loop, using **`m2mb_usb_write`**

![](../../pictures/samples/hello_world_basic_bordered.png)

---------------------

