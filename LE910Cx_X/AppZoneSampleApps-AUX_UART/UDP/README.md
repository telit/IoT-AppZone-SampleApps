
### UDP client

Sample application showcasing UDP echo demo with M2MB API. Debug prints on **AUX UART**


**Features**


- How to check module registration and activate PDP context
- How to open a UDP client socket 
- How to communicate over the socket


#### Application workflow

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print welcome message
- Create a task and start it

**`m2m_udp_test.c`**
- Initialize Network structure and check registration
- Initialize PDP structure and start PDP context
- Create socket and link it to the PDP context id
- Send data and receive response
- Close socket
- Disable PDP context

![](../../pictures/samples/udp_bordered.png)

---------------------

