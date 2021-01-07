
### Uart To Server

Sample application showcasing how to send data from main UART to a connected TCP server. Debug messages are printed on AUX UART port.


**Features**


- How to open main UART to receive data
- How to connect to a server
- How to transmit received data from the UART to the server and viceversa


**Application workflow**

**`M2MB_main.c`**

- Open UART for data and UART_AUX for debug
- Init socket, activate PDP context and connect to server
- Init UART, set its callback function, create tasks to handle input from UART and response from server (optional)
- Send a confirmation on UART
- Wait for data, when it is received, send it to the server
- When a response is received, print it on UART.

Main UART: 

![](../../pictures/samples/uart_to_server_main_bordered.png)

Debug log on AUX UART:

![](../../pictures/samples/uart_to_server_aux_bordered.png)

---------------------

