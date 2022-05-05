
### ATI (AT Instance)

Sample application showing how to use AT Instance functionality (sending AT commands from code). The example supports both sync and async (using a callback) modes. Debug prints on **MAIN UART**


**Features**


- How to open an AT interface from the application
- How to send AT commands and receive responses on the AT interface


#### Application workflow, sync mode

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Init AT0 (first AT instance)
- Send AT+CGMR command
- Print response.
- Release AT0

**`at_sync.c`**

- Init ati functionality and take AT0
- Send AT+CGMR command, then read response after 2 seconds, then return it
- Deinit ati, releasing AT0

![](../../pictures/samples/ati_sync_bordered.png)


#### Application workflow, async mode

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Init AT0 (first AT instance)
- Send AT+CGMR command
- Print response.
- Release AT0

**`at_async.c`**

- Init ati functionality and take AT0, register AT events callback
- Send AT+CGMR command, wait for response semaphore (released in callback), then read it and return it
- Deinit ati, releasing AT0

![](../../pictures/samples/ati_async_bordered.png)

---------------------

