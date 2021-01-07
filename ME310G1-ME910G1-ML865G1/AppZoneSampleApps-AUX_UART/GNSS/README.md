
### GNSS example 

Sample application showing how to use GNSS functionality. Debug prints on **AUX UART**


**Features**


- How to enable GNSS receiver on module
- How to collect location information from receiver


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Init gnss, enable position report and start it.
- When a fix is available, a message will be printed by the GNSS callback function

![](../../pictures/samples/gnss_bordered.png)

---------------------

