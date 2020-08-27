
###AT Tunnel 

Sample application showcasing how to perform an AT tunnel from Main UART to an AT instance. Debug prints on **USB1**.


**Features**


- How to open an AT interface from the application
- How to receive data from main UART and tunnel it to the AT interface, then report back to UART the AT response


**Application workflow**

**`M2MB_main.c`**

- Open USB1 for debug
- Initialize UART with callback function to manage input data
- Initialize AT system to manage AT commands from UART
- wait 5 minutes then deinit AT system

Main UART:

![](../../pictures/samples/at_tunnel_main_uart_bordered.png)

USB1 debug log:

![](../../pictures/samples/at_tunnel_usb_debug_bordered.png)

---------------------


