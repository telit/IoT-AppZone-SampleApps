
### RTC example 

Sample application that shows RTC apis functionalities: how to get/set moudle system time and timestamp. Debug prints on **AUX UART**


**Features**


- How to read module timestamp 
- How to read module system time
- How to set new system time


**Application workflow**

**`M2MB_main.c`**

- Init log azx and print a welcome message
- Init net functionality and wait for module registration
- Init RTC functionality and get module time in timestamp format (seconds from the epoch)
- Get moudle system time in date/time format
- Add 1 hour to timestamp, convert it to system time and set it to module

![](../../pictures/samples/RTC_output_bordered.png)

---------------------

