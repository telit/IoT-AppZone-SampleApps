
### USB Cable Check 

Sample application showing how to check if USB cable is plugged in or not. Debug prints on **AUX UART**


**Features**


- How to open an USB channel and configure it with a callback function
- How to manage USB cable events in the callback function



**Application workflow**

**`M2MB_main.c`**

- Open UART/UART_AUX for debug
- open usb channel and set the callback
- Print greeting message
- Print current usb status

**`USB_Cb`**

- if the event is a connection/disconnection, show the current status

![](../../pictures/samples/usb_cable_check_bordered.png)

---------------------

