
### TCP Server 

Sample application showcasing TCP listening socket demo with M2MB API. Debug prints on **AUX UART**


**Features**


- How to check module registration and activate PDP context
- How to open a TCP listening socket 
- How to manage external hosts connection and exchange data



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage socket and start it

 

**`m2m_tcp_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Create socket and set it in non-blocking mode
- Bind the socket to the listening port
- Start listening for incoming connection
- Check if a connection is incoming using m2mb_socket_bsd_select function
- If a client connects, perform accept on the child socket
- Send a "START" message to the client
- Send some data
- Wait for data from client and print it 
- Close the child socket
- Start listening again, up to 3 times

- Close listening socket

- Disable PDP context

Debug Log

![](../../pictures/samples/tcp_server_bordered.png)

Data on a PuTTY terminal

![](../../pictures/samples/tcp_server_putty_bordered.png)

---------------------

