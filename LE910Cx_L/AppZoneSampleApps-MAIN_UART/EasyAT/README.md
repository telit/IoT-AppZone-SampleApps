
### Easy AT example 

Sample application showcasing Easy AT functionalities. Debug prints on **MAIN UART**


**Features**


- Shows how to register custom commands


The application adds two custom commands to the list of available ones:

- AT#MYCMD
- AT#MYINPUT


#### AT#MYCMD

This is a simple parameter-waiting command. It expects one string parameter, and will print it on the logging interface once received. The command simply returns OK 

#### AT#MYINPUT

This command expects a numeric parameter, which indicates how many bytes will be received over the interface at most (the command will provide a prompt indicating it is waiting data). Then the data management callback will print when data is received, and if CTRL+Z (0x1A in hex) is received, it will complete the process, printing in the log interface what was received. sending ESC will terminate the process discarding any pending data.