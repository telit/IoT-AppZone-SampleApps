
### UART USB tunnel example 

Sample application that opens a tunnel between main UART and USB0 port.


**Features**


- Opens `Main UART` port with a callback function
- Opens `USB0` port with a callback function
- Creates a simple task to manage data exchange between ports


**Application workflow**

**`M2MB_main function`**

- Create `Main UART` handle and configure its parameters
- Create `USB0` handle and configure its parameters
- Create the data management task
- Write **`READY`** on both ports when the tunneling is ready

**`USB_Cb`**

- When data are received on the `USB0` port, retrieve the available amount and send the value to the data management task with the proper command

**`UART_Cb`**

- When data are received on the `Main UART` port, retrieve the available amount and send the value to the data management task with the proper command


**`dataTask_Cb`**

- if command is `TASK_UART_READ_AND_USB_WRITE`, read the requested amount from the `Main UART` port and write it on `USB0`
- if command is `TASK_USB_READ_AND_UART_WRITE`, read the requested amount from the `USB0` port and write it on `Main UART`


UART output received from USB0 (in RED, the user input data from UART )
![](../../pictures/samples/uart_usb_tunnel_uart_bordered.png)


USB0 output received from UART (in RED, the user input data from USB0 )
![](../../pictures/samples/uart_usb_tunnel_usb_bordered.png)

---------------------

