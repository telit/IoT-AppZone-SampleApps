
### SMS_atCmd example 

Sample application showcasing how to receive an SMS containing an AT command, process the AT command and send its answer to sender (configurable in sms_config.txt). A SIM card and antenna must be present. Debug prints on **AUX UART**


**Features**


- How to receive an SMS with an AT command as text inside
- How to send AT command to parser and read the answer
- How to send the AT command answer back to sender via SMS

Optional configuration file to be put in `/data/azc/mod` folder, copy `sms_config.txt` file into your module running the following AT command: 

```
AT#M2MWRITE="/data/azc/mod/sms_config.txt",138
>>> here receive the prompt; then type or send the file, sized 138 bytes
```

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print welcome message
- Init SMS functionality
- Read configuration file sms_config.txt (send SMS with AT command answer back, delte SMS received)
- Init AT command parser
- Create a task to handle SMS parsing and AT command sending
- Wait for an incoming SMS


**`callbacks.c`**

**`msgSMSparse()`**

- When SMS has been received, content is decoded and printed. If there is an AT command inside, command is executed and answer printed and sent back to sender as an SMS (depending on sms_config.txt setting)

![](../../pictures/samples/sms_atCmd_bordered.png)

---------------------

