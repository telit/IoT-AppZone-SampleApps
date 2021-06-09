
### General_INFO example 

Sample application prints some Module/SIM information as IMEI, fw version, IMSI and so on; it prints also some information about registration. Debug prints on **USB0**


**Features**


- How to print some Module information as IMEI, FW version etc
- How to print some SIM information as IMSI, ICCID
- How to get and print some informatio about Module registration as Netowrk Operator, AcT, RSSI, etc


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print welcome message
- Init NET functionality
- Init INFO functionality
- Get and print Module and SIM info
- Wait form module to register to network
- Get and print registration INFO

![](../../pictures/samples/general_INFO_bordered.png)

---------------------

