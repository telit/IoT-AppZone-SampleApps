

# AppZone m2mb Sample Apps 



Package Version: **1.0.13-G1**

Minimum Firmware Version: **37.00.XX1**


## Features

This package goal is to provide sample source code for common activities kickstart.
 

# Quick start

## Deployment Instructions


To manually deploy the Sample application on the devices perform the following steps:

1. Have **37.00.XX1** FW version flashed (`AT#SWPKGV` will give you the FW version)

1. Copy _m2mapz.bin_ to _/mod/_ 
	```
	AT#M2MWRITE="/mod/m2mapz.bin",<size>,1
	```
  where  \<size\> is in bytes
1. Configure the module to run the downloaded binary as default app: `AT#M2MRUN=2,m2mapz.bin`
1. Restart the module and if no AT commands are sent within **10** seconds, start the app: `AT+M2M=4,10`

## References

More info on

* [Getting started with ME910C1](https://y1cj3stn5fbwhv73k0ipk1eg-wpengine.netdna-ssl.com/wp-content/uploads/2018/11/Telit_ME910C1_Quick_Start_guide_r2.pdf) (doc ID 80529NT11661A)

* [How to run applications with AppZone](https://s3.amazonaws.com/site_support/Telit/AppZone-SDK/User+Guides+AppZone+2.0/az-c-m2mb-ug-r1/index.html#!Documents/appendixaatsyntax.htm)


## Known Issues

None

## Contact Information, Support

For general contact, technical support services, technical questions and report documentation errors contact Telit Technical Support at: [TS-EMEA@telit.com](TS-EMEA@telit.com).

For detailed information about where you can buy the Telit modules or for recommendations on accessories and components visit:

[http://www.telit.com](http://www.telit.com)

Our aim is to make this guide as helpful as possible. Keep us informed of your comments and suggestions for improvements.

Telit appreciates feedback from the users of our information.



## Troubleshooting

* Application does not work/start:
	+ Delete application binary and retry
    ```
    AT#M2MDEL="/mod/m2mapz.bin"
    ```
	+ Delete everything, reflash and retry
    ```
    AT#M2MDEL="/mod/m2mapz.bin"
    AT#M2MDEL="/mod/appcfg.ini"
    ```
      
* Application project does not compile
	+ Right click on project name
	+ Select Properties
	+ Select AppZone tab
	+ Select the right plugin (firmware) version
	+ Press "Restore Defaults", then "Apply", then "OK"
	+ Build project again

* Application project shows missing symbols on IDE
	+ Right click on project name
	+ Select Index
	+ Select Rebuild. This will regenerate the symbols index.

---

## Making source code changes

### Folder structure

The applications code follow the structure below:

* `hdr`: header files used by the application
    * `app_cfg.h`: the main configuration file for the application
* `src`: source code specific to the application
* `azx`: helpful utilities used by the application (for GPIOs, LOGGING etc)
    * `hdr`: generic utilities' header files
    * `src`: generic utilities' source files
* `Makefile.in`: customization of the Make process


## Import a Sample App into an IDE project

Consider that the app HelloWorld that prints on Main UART is a good starting point. To import it in a project, please follow the steps below:


On IDE, create a new project: "File"-> "New" -> "Telit Project"

![](pictures/new_proj.png)

Select the preferred firmware version (e.g. 30.00.xx7) and create an empty project.

in the samples package, go in the HelloWorld folder (e.g. `AppZoneSampleApps-MAIN_UART\HelloWorld` ), copy all the files and folders in it (as `src`, `hdr`, `azx` ) and paste them in the root of the newly created IDE project. You are now ready tyo build and try the sample app on your device.





# Applications 

## AUX UART 
*Applications that provide usage examples for various functionalities, log output on Auxiliary UART*


### App update OTA via FTP

Sample application showcasing Application OTA over FTP with AZX FTP. Debug prints on **AUX UART**


**Features**


- How to check module registration and activate PDP context
- How to connect to a FTP server 
- How to download an application binary and update the local version

The app uses a predefined set of parameters. To load custom parameters, upload the `ota_config.txt` file (provided in project's `/src` folder) in module's `/mod` folder, for example with 

```
AT#M2MWRITE="/mod/ota_config.txt",<filesize>

```


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage app OTA and start it


**`ftp_utils.c`**

- Set parameters to default
- Try to load parameters from `ota_config.txt` file
- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Initialize FTP client
- Connect to FTP server and log in
- Get new App binary file size on remote server
- Download the file in `/mod` folder, with the provided name
- Close FTP connection
- Disable PDP context
- Update applications configuration in **app_utils.c**

**`app_utils.c`**

- Set new application as default
- Delete old app binary
- Restart module

![](pictures/samples/app_ftp_ota_bordered.png)

---------------------



### ATI (AT Instance)

Sample application showing how to use AT Instance functionality (sending AT commands from code). The example supports both sync and async (using a callback) modes. Debug prints on **AUX UART**


**Features**


- How to open an AT interface from the application
- How to send AT commands and receive responses on the AT interface


**Application workflow, sync mode**

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

![](pictures/samples/ati_sync_bordered.png)


**Application workflow, async mode**

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

![](pictures/samples/ati_async_bordered.png)

---------------------



### CJSON example: 

Sample application showcasing how to manage JSON objects. Debug prints on **AUX UART**


**Features**


- How to read a JSON using cJSON library
- How to write a JSON
- How to manipulate JSON objects


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Parse an example string into a JSON object and print the result in a formatted string
- Print some test outcomes (e.g. non existing item correctly not found)
- Retrieve single elements from the parsed JSON object and use them to format a descriptive string
- Delete the JSON object
- Create a new JSON object appending elements to it
- Print the result JSON string from the object

![](pictures/samples/cjson_bordered.png)

---------------------



### Events

Sample application showcasing events setup and usage. Debug prints on **AUX UART**


**Features**


- How to setup OS events with a custom bitmask
- How to wait for events and generate them in callback functions to synchronize blocks of code


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create an event handler
- Create a timer to generate an event, with a 2 seconds expiration time
- Wait for a specific event bit on the event handler
- At timer expiration, set the same event bit and verify that the code flow went through after the event.

![](pictures/samples/events_bordered.png)

---------------------



### Events - Barrier (multi events)

Sample application showcasing how to setup and use multiple events to create a barrier. Debug prints on **AUX UART**


**Features**


- How to setup OS events to be used as a barrier
- How to wait for multiple events in the same point, and generate them in callback functions to synchronize blocks of code


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create an event handler
- Create a timer to generate an event, with a 3 seconds expiration time
- Create another timer to generate an event, with a 6 seconds expiration time
- Start both timers
- Wait for both event bits on the event handler (each one will be set by one of the timers)
- At first timer expiration, set the first event bit and verify that the code flow does not procede.
- At second timer expiration, set the second event bit and verify that the code flow went through after the event (implementing a barrier).

![](pictures/samples/events_barrier_bordered.png)

---------------------



### File System example 

Sample application showcasing M2MB File system API usage. Debug prints on **AUX UART**


**Features**


- How to open a file in write mode and write data in it
- How to reopen the file in read mode and read data from it


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Open file in write mode

- Write data in file

- Close file

- Reopen file in read mode

- Read data from file and print it

- Close file and delete it

![](pictures/samples/file_system_bordered.png)

---------------------



### FOTA example

Sample application showcasing FOTA usage with M2MB API. Debug prints on **AUX UART**


**Features**


- How download a delta file from a remote server
- How to apply the delta and update the module firmware


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a main task to manage connectivity.
- create a fota task to manage FOTA and start it with INIT option


**`fota.c`**

**fotaTask()**

- Initialize FOTA system then reset parameters.
- Check current FOTA state, if not in IDLE, return error.
- Send a message to mainTask so networking is initialized.
- after PdPCallback() notifies the correct context activation, configure the fota client parameters such as FTP server URL, username and password
- get delta file from server. when it is completed, FOTADownloadCallback is called.
- If delta download went fine, check it.
- If delta file is correct, apply it. Once complete, restart the module.


**mainTask()**

- Initialize Network structure and check registration
- Initialize PDP structure and start PDP context. Event will be received on **PdPCallback** function
- Disable PDP context when required to stop the app

**PdpCallback()**

- When PDP context is enabled, send a message to fotaTask to start the download




![](pictures/samples/fota_bordered.png)

---------------------



### FTP

Sample application showcasing FTP client demo with AZX FTP. Debug prints on **AUX UART**


**Features**


- How to check module registration and activate PDP context
- How to connect to a FTP server 
- How to exchange data with the server


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage FTP client and start it


**`ftp_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Init FTP client and set the debug function for it
- Connect to the server
- Perform log in
- Check remote file size and last modification time
- Download file from server to local filesystem. A data callback is set to report periodic info about the download status
- Upload the same file to the server with a different name. A data callback is set to report periodic info about the upload status
- Download another file content in a buffer instead of a file. A data callback is set to report periodic info about the download status
- Close the connection with FTP server
- Disable PDP context

![](pictures/samples/ftp_bordered.png)

---------------------



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

![](pictures/samples/gnss_bordered.png)

---------------------



### GPIO interrupt example 

Sample application showing how to use GPIOs and interrupts. Debug prints on **AUX UART**


**Features**


- How to open a GPIO in input mode with interrupt
- How to open a second GPIO in output mode to trigger the first one


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Open *GPIO 4* as output

- Open *GPIO 3* as input and set interrupt for any edge (rising and falling). **A jumper must be used to short GPIO 3 and 4 pins.**

- Toggle *GPIO 4* status high and low every second

- An interrupt is generated on *GPIO 3*

![](pictures/samples/gpio_interrupt_bordered.png)

---------------------



### Hello World

The application prints "Hello World!" over selected output every two seconds. Debug prints on **AUX UART**, <ins>using AZX log example functions</ins>


**Features**


- How to open an output channel using AZX LOG sample functions
- How to print logging information on the channel using AZX LOG sample functions


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print "Hello World!" every 2 seconds in a while loop

![](pictures/samples/hello_world_bordered.png)

---------------------



### HTTP Client

Sample application showing how to use HTTPs client functionalities. Debug prints on **AUX UART**


**Features**


- How to check module registration and activate PDP context
- How to initialize the http client, set the debug hook function and the data callback to manage incoming data
- How to perform GET, HEAD or POST operations


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage HTTP client and start it

**`httpTaskCB`**

- Initialize Network structure and check registration
- Initialize PDP structure and start PDP context
- Create HTTP client options and initialize its functionality
- Create HTTP SSL config and initialize the SSL options
- Configure data management options for HTTP client
- Appy all configurations to HTTP client
- Perform a GET request to a server
- Disable PDP context


**`DATA_CB`**

- Print incoming data
- Set the abort flag to 0 to keep going.

![](pictures/samples/http_client_bordered.png)

---------------------



### HW Timer (Hardware Timer)

The sample application shows how to use HW Timers M2MB API. Debug prints on **AUX UART**


**Features**


- How to open configure a HW timer
- How to use the timer to manage recurring events



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create hw timer structure

- Configure it with 100 ms timeout, periodic timer (auto fires when expires) and autostart

- Init the timer with the parameters

- Wait 10 seconds

- Stop the timer

**`TimerCb`**

- Print a message with an increasing counter

![](pictures/samples/hw_timer_bordered.png)

---------------------



### I2C example 

Sample application showing how to communicate with an I2C slave device. Debug prints on **AUX UART**


**Features**


- How to open a communication channel with an I2C slave device
- How to send and receive data to/from the slave device



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Open I2C bus, setting SDA an SCL pins as 2 and 3 respectively
- Set registers to configure accelerometer
-Read in a loop the 6 registers carrying the 3 axes values and show the g value for each of them



![](pictures/samples/i2c_bordered.png)

---------------------



### Logging Demo

Sample application showing how to print on one of the available output interfaces. Debug prints on **AUX UART**


**Features**


- How to open a logging channel
- How to set a logging level 
- How to use different logging macros


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Print a message with every log level

![](pictures/samples/logging_bordered.png)

---------------------



### LWM2M

Sample application showcasing TLS/SSL with client certificates usage with M2MB API. Debug prints on **AUX UART**


**Features**


- Configure LWM2M client and enable it

- Create an instance of a custom object

- Set an integer value on a read only resource

- Set two integer values on a multi-instance read only resource

- write a string on a read/write resource

- Manage exec requests from the portal

- Manage write, read and monitoring resources changed from the portal

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Create a task to manage the LWM2M client and start it


**`lwm2m_demo.c`**

**`msgLWM2MTask`**
- Check registration status

- Configure APN to the correct one for CID 1

- Initialize LWM2M client, 

  - Check for XML file fo custom object
  
  - Enable unsolicited messages from client
  
  - Create a task \(lwm2m_taskCB is its callback function \)to manage events from Portal
  
  - Enable LwM2M client
  
  - Create a new instance for the custom object
  
  - Wait for client to register to Portal
  
  - Send integer and string values
  
  - Wait for events from server


**`lwm2mIndicationCB`**

- Manage events arriving from client \(operations completion status and unsolicited events\)
- Run lwm2m_taskCB when a monitored resource changes, to manage the action to be done

#### Custom Object configuration

The XML file content must be loaded on the Telit IoT Portal for the demo application to be fully executed.

First, enter Developer section from the top menu

![](pictures/samples/lwm2m_xml_1_developer_bordered.png)


Choose Object Registry

![](pictures/samples/lwm2m_xml_2_object_registry_bordered.png)


Create a New Object

![](pictures/samples/lwm2m_xml_3_new_object_bordered.png)


Copy the xml file content and paste it in the new Object form

![](pictures/samples/lwm2m_xml_4_paste_content_bordered.png)



Also, the application requires the XML file `/xml/object_35000.xml` (provided with the sample files) to be stored in module's `/XML/` folder. 
It can be done with 

`AT#M2MWRITE=/XML/object_35000.xml,<size_in_bytes>`

To load the XML file in the module, Telit AT Controller (TATC) can be used. Once the command above is issued, press the load content button:

![](pictures/samples/lwm2m_xml_5_load_xml_bordered.png)


Select the file from your computer

![](pictures/samples/lwm2m_xml_6_file_select_bordered.png)


The file is successfully loaded on the module

![](pictures/samples/lwm2m_xml_7_done_bordered.png)


#### Application execution example

![](pictures/samples/lwm2m_1_bordered.png)

![](pictures/samples/lwm2m_2_bordered.png)


After the Demo completes the initialization, it is possible to access the object resources from the Portal Object Browser

![](pictures/samples/lwm2m_portal_object_browser_bordered.png)

An instance of the object will be present and the resources can be modified. 

![](pictures/samples/lwm2m_portal_object_bordered.png)

For example, executing the two Exec Resources at the bottom of the list, the application will react accordingly:

![](pictures/samples/lwm2m_3_exec_bordered.png)

Writing a string resource (id /35000/0/11 ), the application will notify the change

![](pictures/samples/lwm2m_4_write_bordered.png)

---------------------



### MQTT Client

Sample application showcasing MQTT client functionalities (with SSL). Debug prints on **AUX UART**


**Features**


- How to check module registration and enable PDP context
- How to configure MQTT client parameters
- How to connect to a broker with SSL and exchange data over a subscribed topic


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage MQTT client and start it


**`mqtt_demo.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Init MQTT client
- Configure it with all parameters (Client ID, username, password, PDP context ID, keepalive timeout...)

- Connect MQTT client to broker
- Subscribe to two topics
- Publish 10 messages with increasing counter. Even messages are sent to topic 1, odd messages on topic 2.
- Print received message in mqtt_topc_cb function
- Disconnect MQTT client and deinit it 

- Disable PDP context

![](pictures/samples/mqtt_bordered.png)

---------------------



### MultiTask

Sample application showcasing multi tasking functionalities with M2MB API. Debug prints on **AUX UART**


**Features**


- How to create tasks using azx utilities
- How to use send messages to tasks
- How to use a semaphore to synchronize two tasks


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create three tasks with the provided utility (this calls public m2mb APIs)

- Send a message to the task1, its callback function azx_msgTask1 will be called

**`azx_msgTask1`**

- Print received parameters from main
- Send modified parameters to task2 \(its callback function azx_msgTask2 will be called\)
- wait for an InterProcess Communication semaphore to be available \(released by task3\)
- Once the semaphore is available, print a message and return

**`azx_msgTask2`**

- Print received parameters from caller
- If first parameter is bigger than a certain value, Send modified parameters to task3
- Else, use the second parameter as a task handle and print the corresponding name plus the value of the first parameter

**`azx_msgTask3`**

- Print received parameters from task 2
- release IPC semaphore
- send message to task 2 with first parameter below the threshold and second parameter with task3 handle


![](pictures/samples/multitask_bordered.png)

---------------------



### MutEx

Sample application showing mutex usage, with ownership and prioritization usage. Debug prints on **AUX UART**


**Features**


- How to create a mutex 
- How to use the mutex with tasks having different priorities
- how to reorder the pending tasks queue for the mutex


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print welcome message
- Create four tasks with the provided utility (this calls public m2mb APIs). The first task is a "producer", putting data on a shared buffer. The second is a "consumer" of said data, the other two are used for prioritization demo
- run producer and consumer tasks at the same pace. the shared buffer will stay empty, because the resource is consumed right after creation
- run producer twice as fast as consumer. The buffer is slowly filled
- run consumer twice as fast as publisher. The buffer is always empty.
- reserve the mutex in the main task and run producer, support and support2 tasks (in this order). Then release the mutex and check the execution order. It should be by arrival.
- reserve the mutex in the main task and run the same three task, but before releasing the mutex, call the prioritization API. the task with highest priority \(producer\) is put as first in the queue.

![](pictures/samples/mutex_1_bordered.png)

![](pictures/samples/mutex_2_bordered.png)

![](pictures/samples/mutex_3_bordered.png)

![](pictures/samples/mutex_4_bordered.png)

---------------------



### SMS PDU

Sample application showcasing how to create and decode PDUs to be used with m2mb_sms_* API set. A SIM card and antenna must be present. Debug prints on **AUX UART**


**Features**


- How to enable SMS functionality
- How to use encode an SMS PDU to be sent with m2mb_api
- How to decode a received SMS response from PDU to ASCII mode.


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Init sms functionality
- Create PDU from text message
- Send message to destination number
- Wait for response
- When SMS PDU response is received, decode it and print information about it, plus the message content

![](pictures/samples/sms_pdu_bordered.png)

---------------------



### SW Timer (Software Timer)

The sample application shows how to use SW Timers M2MB API. Debug prints on **AUX UART**


**Features**


- How to open configure a SW timer
- How to use the timer to manage recurring events


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create sw timer structure

- Configure it with 4 seconds timeout, periodic timer (auto fires when expires)

- Init the timer with the parameters

- Start the timer

- Wait 10 seconds

- Stop the timer

**`timerCb`**

- Print a message with inside the callback

![](pictures/samples/sw_timer_bordered.png)

---------------------



### TCP IP 

Sample application showcasing TCP echo demo with M2MB API. Debug prints on **AUX UART**


**Features**


- How to check module registration and activate PDP context
- How to open a TCP client socket 
- How to communicate over the socket


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage socket and start it

 

**`m2m_tcp_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Create socket and link it to the PDP context id

- Connect to the server

- Send data and receive response

- Close socket

- Disable PDP context

![](pictures/samples/tcp_ip_bordered.png)

---------------------



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

![](pictures/samples/tcp_server_bordered.png)

Data on a PuTTY terminal

![](pictures/samples/tcp_server_putty_bordered.png)

---------------------



### TLS SSL Client

Sample application showcasing TLS/SSL with client certificates usage with M2MB API. Debug prints on **AUX UART**


**Features**


- How to check module registration and enable PDP context
- How to open a SSL client socket
- How to communicate over SSL socket



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Create a task to manage the connection and start it

 

**`ssl_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Create socket and link it to the PDP context id

- Connect to the server over TCP socket

- Initialize the TLS parameters (TLS1.2) andh auth mode (server+client auth in the example)

- Create SSL context

- Read certificates files and store them

- Create secure socket and connect to the server using SSL

- Send data and receive response

- Close secure socket

- Close socket

- Delete SSL context

- Disable PDP context

 
The application requires the certificates to be stored in `/test_ssl_certs/` folder. It can be created with `AT#M2MMKDIR=/test_ssl_certs`


![](pictures/samples/tcp_ssl_client_bordered.png)

---------------------



### UDP client

Sample application showcasing UDP echo demo with M2MB API. Debug prints on **AUX UART**


**Features**


- How to check module registration and activate PDP context
- How to open a UDP client socket 
- How to communicate over the socket


**Application workflow**

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

![](pictures/samples/udp_bordered.png)

---------------------



### ZLIB example 

Sample application showing how to compress/uncompress with ZLIB. Debug prints on **AUX UART**


**Features**


- How to compress a file
- How to uncompress a file

In order to execute the entire test, copy `test.gz` file into your module running the following AT command: 

```
AT#M2MWRITE="/mod/test.gz",138
>>> here receive the prompt; then type or send the file, sized 138 bytes
```

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Test the compression and decompression of a data string
- Test the decompression of a .gz file (test.gz), expected to be in `/mod` folder, into its content `test.txt`. The file must be uploaded by the user (see steps above).

![](pictures/samples/zlib_bordered.png)

---------------------



## BASIC 
*Basic applications showing simple operations with minimum code overhead*


### Basic Hello World (Aux UART)

The application prints "Hello World!" on Auxiliary UART every 2 seconds using


**Features**


- How to open Auxiliary UART as an output channel
- How to print messages out of the channel


**Application workflow**

**`M2MB_main.c`**

- Open Auxiliary UART with **`m2mb_uart_open`** function
- write a welcome message using **`m2mb_uart_write`**
- write "Hello World!" every 2 seconds in a while loop, using **`m2mb_uart_write`**

![](pictures/samples/hello_world_basic_bordered.png)

---------------------



### Basic Hello World (Main UART)

The application prints "Hello World!" on Main UART every 2 seconds using


**Features**


- How to open Main UART as an output channel
- How to print messages out of the channel


**Application workflow**

**`M2MB_main.c`**

- Open Main UART with **`m2mb_uart_open`** function
- write a welcome message using **`m2mb_uart_write`**
- write "Hello World!" every 2 seconds in a while loop, using **`m2mb_uart_write`**

![](pictures/samples/hello_world_basic_bordered.png)

---------------------



### Basic Hello World (USB0)

The application prints "Hello World!" on USB 0 every 2 seconds using


**Features**


- How to open USB 0 as an output channel
- How to print messages out of the channel


**Application workflow**

**`M2MB_main.c`**

- Open USB 0 with **`m2mb_usb_open`** function
- write a welcome message using **`m2mb_usb_write`**
- write "Hello World!" every 2 seconds in a while loop, using **`m2mb_usb_write`**

![](pictures/samples/hello_world_basic_bordered.png)

---------------------



### Basic Task 

The application shows how to create and manage tasks with m2mb APIs. Debug prints on MAIN UART (can be changed in M2MB_Main function)


**Features**


- How to create a new task using m2mb APIs
- How to start the task and send messages to it
- how to destroy the task

**Application workflow**

**`M2MB_main.c`**

- Open UART
- Print welcome message
- Configure and create message queue for task
- Configure and create task
- Send 2 messages to the task queue

**`task_entry_function`**

- Receive messages from the task queue in a loop
- Print the message data when one arrives

![](pictures/samples/basic_task_bordered.png)

---------------------



## C++ 
*Applications that provide usage examples with C++*


### Logging C++ 

Sample application showcasing how to create a C++ OO code, providing a logging class (equivalent to the one in Logging demo)


**Features**


- how to define a class object
- how to instantiate and call the class from a C++ main
- how to configure makefile flags to build the application


**Application workflow**

**`M2MB_main.c`**

- Call C++ main function

**`main.cpp`**

- Create a Logger class instance and set it to USB/UART/UART_AUX
- Print one message for every log level

![](pictures/samples/cpp_logging_bordered.png)

---------------------



### C++  method to function pointer

Sample application showing how to manage class methods as function pointers. Debug prints on MAIN_UART


**Features**


- how to define a class object with a generic method with the same prototype as a m2mb callback function (in this case, a hw timer callback)
- how to use a single static function in the class workspace to call multiple class instances method by using "this" as argument in the timer creation
- how to configure the static function to convert the input parameter with a static cast and call the input class instance method


**Application workflow**

**`M2MB_main.c`**

- Call C++ main function

**`main.cpp`**

- Create two HwTimer class instance with different timeouts
- Start both timers.
- Each will expire at a different time, and both m2mb timers will call the static function, which will run the appropriate class instance method as callback.


![](pictures/samples/cpp_method_bordered.png)

---------------------



## MAIN UART 
*Applications that provide usage examples for various functionalities, log output on Main UART*


### App update OTA via FTP

Sample application showcasing Application OTA over FTP with AZX FTP. Debug prints on **MAIN UART**


**Features**


- How to check module registration and activate PDP context
- How to connect to a FTP server 
- How to download an application binary and update the local version

The app uses a predefined set of parameters. To load custom parameters, upload the `ota_config.txt` file (provided in project's `/src` folder) in module's `/mod` folder, for example with 

```
AT#M2MWRITE="/mod/ota_config.txt",<filesize>

```


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage app OTA and start it


**`ftp_utils.c`**

- Set parameters to default
- Try to load parameters from `ota_config.txt` file
- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Initialize FTP client
- Connect to FTP server and log in
- Get new App binary file size on remote server
- Download the file in `/mod` folder, with the provided name
- Close FTP connection
- Disable PDP context
- Update applications configuration in **app_utils.c**

**`app_utils.c`**

- Set new application as default
- Delete old app binary
- Restart module

![](pictures/samples/app_ftp_ota_bordered.png)

---------------------



### ATI (AT Instance)

Sample application showing how to use AT Instance functionality (sending AT commands from code). The example supports both sync and async (using a callback) modes. Debug prints on **MAIN UART**


**Features**


- How to open an AT interface from the application
- How to send AT commands and receive responses on the AT interface


**Application workflow, sync mode**

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

![](pictures/samples/ati_sync_bordered.png)


**Application workflow, async mode**

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

![](pictures/samples/ati_async_bordered.png)

---------------------



### AT Tunnel 

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

![](pictures/samples/at_tunnel_main_uart_bordered.png)

USB1 debug log:

![](pictures/samples/at_tunnel_usb_debug_bordered.png)

---------------------




### CJSON example: 

Sample application showcasing how to manage JSON objects. Debug prints on **MAIN UART**


**Features**


- How to read a JSON using cJSON library
- How to write a JSON
- How to manipulate JSON objects


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Parse an example string into a JSON object and print the result in a formatted string
- Print some test outcomes (e.g. non existing item correctly not found)
- Retrieve single elements from the parsed JSON object and use them to format a descriptive string
- Delete the JSON object
- Create a new JSON object appending elements to it
- Print the result JSON string from the object

![](pictures/samples/cjson_bordered.png)

---------------------



### Events

Sample application showcasing events setup and usage. Debug prints on **MAIN UART**


**Features**


- How to setup OS events with a custom bitmask
- How to wait for events and generate them in callback functions to synchronize blocks of code


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create an event handler
- Create a timer to generate an event, with a 2 seconds expiration time
- Wait for a specific event bit on the event handler
- At timer expiration, set the same event bit and verify that the code flow went through after the event.

![](pictures/samples/events_bordered.png)

---------------------



### Events - Barrier (multi events)

Sample application showcasing how to setup and use multiple events to create a barrier. Debug prints on **MAIN UART**


**Features**


- How to setup OS events to be used as a barrier
- How to wait for multiple events in the same point, and generate them in callback functions to synchronize blocks of code


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create an event handler
- Create a timer to generate an event, with a 3 seconds expiration time
- Create another timer to generate an event, with a 6 seconds expiration time
- Start both timers
- Wait for both event bits on the event handler (each one will be set by one of the timers)
- At first timer expiration, set the first event bit and verify that the code flow does not procede.
- At second timer expiration, set the second event bit and verify that the code flow went through after the event (implementing a barrier).

![](pictures/samples/events_barrier_bordered.png)

---------------------



### File System example 

Sample application showcasing M2MB File system API usage. Debug prints on **MAIN UART**


**Features**


- How to open a file in write mode and write data in it
- How to reopen the file in read mode and read data from it


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Open file in write mode

- Write data in file

- Close file

- Reopen file in read mode

- Read data from file and print it

- Close file and delete it

![](pictures/samples/file_system_bordered.png)

---------------------



### FOTA example

Sample application showcasing FOTA usage with M2MB API. Debug prints on **MAIN UART**


**Features**


- How download a delta file from a remote server
- How to apply the delta and update the module firmware


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a main task to manage connectivity.
- create a fota task to manage FOTA and start it with INIT option


**`fota.c`**

**fotaTask()**

- Initialize FOTA system then reset parameters.
- Check current FOTA state, if not in IDLE, return error.
- Send a message to mainTask so networking is initialized.
- after PdPCallback() notifies the correct context activation, configure the fota client parameters such as FTP server URL, username and password
- get delta file from server. when it is completed, FOTADownloadCallback is called.
- If delta download went fine, check it.
- If delta file is correct, apply it. Once complete, restart the module.


**mainTask()**

- Initialize Network structure and check registration
- Initialize PDP structure and start PDP context. Event will be received on **PdPCallback** function
- Disable PDP context when required to stop the app

**PdpCallback()**

- When PDP context is enabled, send a message to fotaTask to start the download




![](pictures/samples/fota_bordered.png)

---------------------



### FTP

Sample application showcasing FTP client demo with AZX FTP. Debug prints on **MAIN UART**


**Features**


- How to check module registration and activate PDP context
- How to connect to a FTP server 
- How to exchange data with the server


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage FTP client and start it


**`ftp_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Init FTP client and set the debug function for it
- Connect to the server
- Perform log in
- Check remote file size and last modification time
- Download file from server to local filesystem. A data callback is set to report periodic info about the download status
- Upload the same file to the server with a different name. A data callback is set to report periodic info about the upload status
- Download another file content in a buffer instead of a file. A data callback is set to report periodic info about the download status
- Close the connection with FTP server
- Disable PDP context

![](pictures/samples/ftp_bordered.png)

---------------------



### GNSS example 

Sample application showing how to use GNSS functionality. Debug prints on **MAIN UART**


**Features**


- How to enable GNSS receiver on module
- How to collect location information from receiver


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Init gnss, enable position report and start it.
- When a fix is available, a message will be printed by the GNSS callback function

![](pictures/samples/gnss_bordered.png)

---------------------



### GPIO interrupt example 

Sample application showing how to use GPIOs and interrupts. Debug prints on **MAIN UART**


**Features**


- How to open a GPIO in input mode with interrupt
- How to open a second GPIO in output mode to trigger the first one


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Open *GPIO 4* as output

- Open *GPIO 3* as input and set interrupt for any edge (rising and falling). **A jumper must be used to short GPIO 3 and 4 pins.**

- Toggle *GPIO 4* status high and low every second

- An interrupt is generated on *GPIO 3*

![](pictures/samples/gpio_interrupt_bordered.png)

---------------------



### Hello World

The application prints "Hello World!" over selected output every two seconds. Debug prints on **MAIN UART**, <ins>using AZX log example functions</ins>


**Features**


- How to open an output channel using AZX LOG sample functions
- How to print logging information on the channel using AZX LOG sample functions


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print "Hello World!" every 2 seconds in a while loop

![](pictures/samples/hello_world_bordered.png)

---------------------



### HTTP Client

Sample application showing how to use HTTPs client functionalities. Debug prints on **MAIN UART**


**Features**


- How to check module registration and activate PDP context
- How to initialize the http client, set the debug hook function and the data callback to manage incoming data
- How to perform GET, HEAD or POST operations


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage HTTP client and start it

**`httpTaskCB`**

- Initialize Network structure and check registration
- Initialize PDP structure and start PDP context
- Create HTTP client options and initialize its functionality
- Create HTTP SSL config and initialize the SSL options
- Configure data management options for HTTP client
- Appy all configurations to HTTP client
- Perform a GET request to a server
- Disable PDP context


**`DATA_CB`**

- Print incoming data
- Set the abort flag to 0 to keep going.

![](pictures/samples/http_client_bordered.png)

---------------------



### HW Timer (Hardware Timer)

The sample application shows how to use HW Timers M2MB API. Debug prints on **MAIN UART**


**Features**


- How to open configure a HW timer
- How to use the timer to manage recurring events



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create hw timer structure

- Configure it with 100 ms timeout, periodic timer (auto fires when expires) and autostart

- Init the timer with the parameters

- Wait 10 seconds

- Stop the timer

**`TimerCb`**

- Print a message with an increasing counter

![](pictures/samples/hw_timer_bordered.png)

---------------------



### I2C example 

Sample application showing how to communicate with an I2C slave device. Debug prints on **MAIN UART**


**Features**


- How to open a communication channel with an I2C slave device
- How to send and receive data to/from the slave device



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Open I2C bus, setting SDA an SCL pins as 2 and 3 respectively
- Set registers to configure accelerometer
-Read in a loop the 6 registers carrying the 3 axes values and show the g value for each of them



![](pictures/samples/i2c_bordered.png)

---------------------



### Logging Demo

Sample application showing how to print on one of the available output interfaces. Debug prints on **MAIN UART**


**Features**


- How to open a logging channel
- How to set a logging level 
- How to use different logging macros


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Print a message with every log level

![](pictures/samples/logging_bordered.png)

---------------------



### LWM2M

Sample application showcasing TLS/SSL with client certificates usage with M2MB API. Debug prints on **MAIN UART**


**Features**


- Configure LWM2M client and enable it

- Create an instance of a custom object

- Set an integer value on a read only resource

- Set two integer values on a multi-instance read only resource

- write a string on a read/write resource

- Manage exec requests from the portal

- Manage write, read and monitoring resources changed from the portal

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Create a task to manage the LWM2M client and start it


**`lwm2m_demo.c`**

**`msgLWM2MTask`**
- Check registration status

- Configure APN to the correct one for CID 1

- Initialize LWM2M client, 

  - Check for XML file fo custom object
  
  - Enable unsolicited messages from client
  
  - Create a task \(lwm2m_taskCB is its callback function \)to manage events from Portal
  
  - Enable LwM2M client
  
  - Create a new instance for the custom object
  
  - Wait for client to register to Portal
  
  - Send integer and string values
  
  - Wait for events from server


**`lwm2mIndicationCB`**

- Manage events arriving from client \(operations completion status and unsolicited events\)
- Run lwm2m_taskCB when a monitored resource changes, to manage the action to be done

#### Custom Object configuration

The XML file content must be loaded on the Telit IoT Portal for the demo application to be fully executed.

First, enter Developer section from the top menu

![](pictures/samples/lwm2m_xml_1_developer_bordered.png)


Choose Object Registry

![](pictures/samples/lwm2m_xml_2_object_registry_bordered.png)


Create a New Object

![](pictures/samples/lwm2m_xml_3_new_object_bordered.png)


Copy the xml file content and paste it in the new Object form

![](pictures/samples/lwm2m_xml_4_paste_content_bordered.png)



Also, the application requires the XML file `/xml/object_35000.xml` (provided with the sample files) to be stored in module's `/XML/` folder. 
It can be done with 

`AT#M2MWRITE=/XML/object_35000.xml,<size_in_bytes>`

To load the XML file in the module, Telit AT Controller (TATC) can be used. Once the command above is issued, press the load content button:

![](pictures/samples/lwm2m_xml_5_load_xml_bordered.png)


Select the file from your computer

![](pictures/samples/lwm2m_xml_6_file_select_bordered.png)


The file is successfully loaded on the module

![](pictures/samples/lwm2m_xml_7_done_bordered.png)


#### Application execution example

![](pictures/samples/lwm2m_1_bordered.png)

![](pictures/samples/lwm2m_2_bordered.png)


After the Demo completes the initialization, it is possible to access the object resources from the Portal Object Browser

![](pictures/samples/lwm2m_portal_object_browser_bordered.png)

An instance of the object will be present and the resources can be modified. 

![](pictures/samples/lwm2m_portal_object_bordered.png)

For example, executing the two Exec Resources at the bottom of the list, the application will react accordingly:

![](pictures/samples/lwm2m_3_exec_bordered.png)

Writing a string resource (id /35000/0/11 ), the application will notify the change

![](pictures/samples/lwm2m_4_write_bordered.png)

---------------------



### MQTT Client

Sample application showcasing MQTT client functionalities (with SSL). Debug prints on **MAIN UART**


**Features**


- How to check module registration and enable PDP context
- How to configure MQTT client parameters
- How to connect to a broker with SSL and exchange data over a subscribed topic


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage MQTT client and start it


**`mqtt_demo.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Init MQTT client
- Configure it with all parameters (Client ID, username, password, PDP context ID, keepalive timeout...)

- Connect MQTT client to broker
- Subscribe to two topics
- Publish 10 messages with increasing counter. Even messages are sent to topic 1, odd messages on topic 2.
- Print received message in mqtt_topc_cb function
- Disconnect MQTT client and deinit it 

- Disable PDP context

![](pictures/samples/mqtt_bordered.png)

---------------------



### MultiTask

Sample application showcasing multi tasking functionalities with M2MB API. Debug prints on **MAIN UART**


**Features**


- How to create tasks using azx utilities
- How to use send messages to tasks
- How to use a semaphore to synchronize two tasks


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create three tasks with the provided utility (this calls public m2mb APIs)

- Send a message to the task1, its callback function azx_msgTask1 will be called

**`azx_msgTask1`**

- Print received parameters from main
- Send modified parameters to task2 \(its callback function azx_msgTask2 will be called\)
- wait for an InterProcess Communication semaphore to be available \(released by task3\)
- Once the semaphore is available, print a message and return

**`azx_msgTask2`**

- Print received parameters from caller
- If first parameter is bigger than a certain value, Send modified parameters to task3
- Else, use the second parameter as a task handle and print the corresponding name plus the value of the first parameter

**`azx_msgTask3`**

- Print received parameters from task 2
- release IPC semaphore
- send message to task 2 with first parameter below the threshold and second parameter with task3 handle


![](pictures/samples/multitask_bordered.png)

---------------------



### MutEx

Sample application showing mutex usage, with ownership and prioritization usage. Debug prints on **MAIN UART**


**Features**


- How to create a mutex 
- How to use the mutex with tasks having different priorities
- how to reorder the pending tasks queue for the mutex


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print welcome message
- Create four tasks with the provided utility (this calls public m2mb APIs). The first task is a "producer", putting data on a shared buffer. The second is a "consumer" of said data, the other two are used for prioritization demo
- run producer and consumer tasks at the same pace. the shared buffer will stay empty, because the resource is consumed right after creation
- run producer twice as fast as consumer. The buffer is slowly filled
- run consumer twice as fast as publisher. The buffer is always empty.
- reserve the mutex in the main task and run producer, support and support2 tasks (in this order). Then release the mutex and check the execution order. It should be by arrival.
- reserve the mutex in the main task and run the same three task, but before releasing the mutex, call the prioritization API. the task with highest priority \(producer\) is put as first in the queue.

![](pictures/samples/mutex_1_bordered.png)

![](pictures/samples/mutex_2_bordered.png)

![](pictures/samples/mutex_3_bordered.png)

![](pictures/samples/mutex_4_bordered.png)

---------------------



### SMS PDU

Sample application showcasing how to create and decode PDUs to be used with m2mb_sms_* API set. A SIM card and antenna must be present. Debug prints on **MAIN UART**


**Features**


- How to enable SMS functionality
- How to use encode an SMS PDU to be sent with m2mb_api
- How to decode a received SMS response from PDU to ASCII mode.


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Init sms functionality
- Create PDU from text message
- Send message to destination number
- Wait for response
- When SMS PDU response is received, decode it and print information about it, plus the message content

![](pictures/samples/sms_pdu_bordered.png)

---------------------



### SPI Echo

Sample application showing how to communicate over SPI with m2mb API. Debug prints on **MAIN UART**


**Features**


- How to open an SPI bus. MOSI and MISO will be shorted, to have an echo.
- How to communicate over SPI bus


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Open SPI bus, set parameters

- Send data on MOSI and read the same in MISO


![](pictures/samples/spi_echo_bordered.png)

---------------------



### SPI sensors

Sample application showing SPI usage, configuring two ST devices: a magnetometer (ST LIS3MDL) and a gyroscope (ST L3G4200D). The application will read values from both devices using GPIO4 and 3 (respectively) as magnetometer CS and gyro CS. Debug prints on **MAIN UART**


**Features**


- How to open an SPI bus with a slave device
- How to communicate with the device over the SPI bus

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Open SPI bus, set parameters
- Configure `GPIO 3` and `GPIO 4` as output, set them high (idle)
- Set registers to configure magnetometer
- Read in a loop \(10 iterations\) the registers carrying the 3 axes values and show the gauss value for each of them. A metal object is put close to the sensor to change the read values.
- Set registers to configure gyroscope
- Read in a loop \(10 iterations\) the registers carrying the 3 axes values and show the degrees per second value for each of them. The board is rotated to change the read values.

![](pictures/samples/spi_sensors_bordered.png)

---------------------



### SW Timer (Software Timer)

The sample application shows how to use SW Timers M2MB API. Debug prints on **MAIN UART**


**Features**


- How to open configure a SW timer
- How to use the timer to manage recurring events


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create sw timer structure

- Configure it with 4 seconds timeout, periodic timer (auto fires when expires)

- Init the timer with the parameters

- Start the timer

- Wait 10 seconds

- Stop the timer

**`timerCb`**

- Print a message with inside the callback

![](pictures/samples/sw_timer_bordered.png)

---------------------



### TCP IP 

Sample application showcasing TCP echo demo with M2MB API. Debug prints on **MAIN UART**


**Features**


- How to check module registration and activate PDP context
- How to open a TCP client socket 
- How to communicate over the socket


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage socket and start it

 

**`m2m_tcp_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Create socket and link it to the PDP context id

- Connect to the server

- Send data and receive response

- Close socket

- Disable PDP context

![](pictures/samples/tcp_ip_bordered.png)

---------------------



### TCP Server 

Sample application showcasing TCP listening socket demo with M2MB API. Debug prints on **MAIN UART**


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

![](pictures/samples/tcp_server_bordered.png)

Data on a PuTTY terminal

![](pictures/samples/tcp_server_putty_bordered.png)

---------------------



### TLS SSL Client

Sample application showcasing TLS/SSL with client certificates usage with M2MB API. Debug prints on **MAIN UART**


**Features**


- How to check module registration and enable PDP context
- How to open a SSL client socket
- How to communicate over SSL socket



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Create a task to manage the connection and start it

 

**`ssl_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Create socket and link it to the PDP context id

- Connect to the server over TCP socket

- Initialize the TLS parameters (TLS1.2) andh auth mode (server+client auth in the example)

- Create SSL context

- Read certificates files and store them

- Create secure socket and connect to the server using SSL

- Send data and receive response

- Close secure socket

- Close socket

- Delete SSL context

- Disable PDP context

 
The application requires the certificates to be stored in `/test_ssl_certs/` folder. It can be created with `AT#M2MMKDIR=/test_ssl_certs`


![](pictures/samples/tcp_ssl_client_bordered.png)

---------------------



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

![](pictures/samples/uart_to_server_main_bordered.png)

Debug log on AUX UART:

![](pictures/samples/uart_to_server_aux_bordered.png)

---------------------



### UDP client

Sample application showcasing UDP echo demo with M2MB API. Debug prints on **MAIN UART**


**Features**


- How to check module registration and activate PDP context
- How to open a UDP client socket 
- How to communicate over the socket


**Application workflow**

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

![](pictures/samples/udp_bordered.png)

---------------------



### ZLIB example 

Sample application showing how to compress/uncompress with ZLIB. Debug prints on **MAIN UART**


**Features**


- How to compress a file
- How to uncompress a file

In order to execute the entire test, copy `test.gz` file into your module running the following AT command: 

```
AT#M2MWRITE="/mod/test.gz",138
>>> here receive the prompt; then type or send the file, sized 138 bytes
```

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Test the compression and decompression of a data string
- Test the decompression of a .gz file (test.gz), expected to be in `/mod` folder, into its content `test.txt`. The file must be uploaded by the user (see steps above).

![](pictures/samples/zlib_bordered.png)

---------------------



## MISC 
*Applications that provide usage examples for various functionalities, without prints*


### GPIO toggle example 

Sample application showcasing GPIO usage with M2MB API


**Features**


- How to open a gpio in output mode and change its status




## USB0 
*Applications that provide usage examples for various functionalities, log output on USB0*


### App update OTA via FTP

Sample application showcasing Application OTA over FTP with AZX FTP. Debug prints on **USB0**


**Features**


- How to check module registration and activate PDP context
- How to connect to a FTP server 
- How to download an application binary and update the local version

The app uses a predefined set of parameters. To load custom parameters, upload the `ota_config.txt` file (provided in project's `/src` folder) in module's `/mod` folder, for example with 

```
AT#M2MWRITE="/mod/ota_config.txt",<filesize>

```


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage app OTA and start it


**`ftp_utils.c`**

- Set parameters to default
- Try to load parameters from `ota_config.txt` file
- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Initialize FTP client
- Connect to FTP server and log in
- Get new App binary file size on remote server
- Download the file in `/mod` folder, with the provided name
- Close FTP connection
- Disable PDP context
- Update applications configuration in **app_utils.c**

**`app_utils.c`**

- Set new application as default
- Delete old app binary
- Restart module

![](pictures/samples/app_ftp_ota_bordered.png)

---------------------



### ATI (AT Instance)

Sample application showing how to use AT Instance functionality (sending AT commands from code). The example supports both sync and async (using a callback) modes. Debug prints on **USB0**


**Features**


- How to open an AT interface from the application
- How to send AT commands and receive responses on the AT interface


**Application workflow, sync mode**

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

![](pictures/samples/ati_sync_bordered.png)


**Application workflow, async mode**

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

![](pictures/samples/ati_async_bordered.png)

---------------------



### CJSON example: 

Sample application showcasing how to manage JSON objects. Debug prints on **USB0**


**Features**


- How to read a JSON using cJSON library
- How to write a JSON
- How to manipulate JSON objects


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Parse an example string into a JSON object and print the result in a formatted string
- Print some test outcomes (e.g. non existing item correctly not found)
- Retrieve single elements from the parsed JSON object and use them to format a descriptive string
- Delete the JSON object
- Create a new JSON object appending elements to it
- Print the result JSON string from the object

![](pictures/samples/cjson_bordered.png)

---------------------



### Events

Sample application showcasing events setup and usage. Debug prints on **USB0**


**Features**


- How to setup OS events with a custom bitmask
- How to wait for events and generate them in callback functions to synchronize blocks of code


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create an event handler
- Create a timer to generate an event, with a 2 seconds expiration time
- Wait for a specific event bit on the event handler
- At timer expiration, set the same event bit and verify that the code flow went through after the event.

![](pictures/samples/events_bordered.png)

---------------------



### Events - Barrier (multi events)

Sample application showcasing how to setup and use multiple events to create a barrier. Debug prints on **USB0**


**Features**


- How to setup OS events to be used as a barrier
- How to wait for multiple events in the same point, and generate them in callback functions to synchronize blocks of code


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create an event handler
- Create a timer to generate an event, with a 3 seconds expiration time
- Create another timer to generate an event, with a 6 seconds expiration time
- Start both timers
- Wait for both event bits on the event handler (each one will be set by one of the timers)
- At first timer expiration, set the first event bit and verify that the code flow does not procede.
- At second timer expiration, set the second event bit and verify that the code flow went through after the event (implementing a barrier).

![](pictures/samples/events_barrier_bordered.png)

---------------------



### File System example 

Sample application showcasing M2MB File system API usage. Debug prints on **USB0**


**Features**


- How to open a file in write mode and write data in it
- How to reopen the file in read mode and read data from it


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Open file in write mode

- Write data in file

- Close file

- Reopen file in read mode

- Read data from file and print it

- Close file and delete it

![](pictures/samples/file_system_bordered.png)

---------------------



### FOTA example

Sample application showcasing FOTA usage with M2MB API. Debug prints on **USB0**


**Features**


- How download a delta file from a remote server
- How to apply the delta and update the module firmware


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a main task to manage connectivity.
- create a fota task to manage FOTA and start it with INIT option


**`fota.c`**

**fotaTask()**

- Initialize FOTA system then reset parameters.
- Check current FOTA state, if not in IDLE, return error.
- Send a message to mainTask so networking is initialized.
- after PdPCallback() notifies the correct context activation, configure the fota client parameters such as FTP server URL, username and password
- get delta file from server. when it is completed, FOTADownloadCallback is called.
- If delta download went fine, check it.
- If delta file is correct, apply it. Once complete, restart the module.


**mainTask()**

- Initialize Network structure and check registration
- Initialize PDP structure and start PDP context. Event will be received on **PdPCallback** function
- Disable PDP context when required to stop the app

**PdpCallback()**

- When PDP context is enabled, send a message to fotaTask to start the download




![](pictures/samples/fota_bordered.png)

---------------------



### FTP

Sample application showcasing FTP client demo with AZX FTP. Debug prints on **USB0**


**Features**


- How to check module registration and activate PDP context
- How to connect to a FTP server 
- How to exchange data with the server


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage FTP client and start it


**`ftp_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Init FTP client and set the debug function for it
- Connect to the server
- Perform log in
- Check remote file size and last modification time
- Download file from server to local filesystem. A data callback is set to report periodic info about the download status
- Upload the same file to the server with a different name. A data callback is set to report periodic info about the upload status
- Download another file content in a buffer instead of a file. A data callback is set to report periodic info about the download status
- Close the connection with FTP server
- Disable PDP context

![](pictures/samples/ftp_bordered.png)

---------------------



### GNSS example 

Sample application showing how to use GNSS functionality. Debug prints on **USB0**


**Features**


- How to enable GNSS receiver on module
- How to collect location information from receiver


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Init gnss, enable position report and start it.
- When a fix is available, a message will be printed by the GNSS callback function

![](pictures/samples/gnss_bordered.png)

---------------------



### GPIO interrupt example 

Sample application showing how to use GPIOs and interrupts. Debug prints on **USB0**


**Features**


- How to open a GPIO in input mode with interrupt
- How to open a second GPIO in output mode to trigger the first one


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Open *GPIO 4* as output

- Open *GPIO 3* as input and set interrupt for any edge (rising and falling). **A jumper must be used to short GPIO 3 and 4 pins.**

- Toggle *GPIO 4* status high and low every second

- An interrupt is generated on *GPIO 3*

![](pictures/samples/gpio_interrupt_bordered.png)

---------------------



### Hello World

The application prints "Hello World!" over selected output every two seconds. Debug prints on **USB0**, <ins>using AZX log example functions</ins>


**Features**


- How to open an output channel using AZX LOG sample functions
- How to print logging information on the channel using AZX LOG sample functions


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print "Hello World!" every 2 seconds in a while loop

![](pictures/samples/hello_world_bordered.png)

---------------------



### HTTP Client

Sample application showing how to use HTTPs client functionalities. Debug prints on **USB0**


**Features**


- How to check module registration and activate PDP context
- How to initialize the http client, set the debug hook function and the data callback to manage incoming data
- How to perform GET, HEAD or POST operations


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage HTTP client and start it

**`httpTaskCB`**

- Initialize Network structure and check registration
- Initialize PDP structure and start PDP context
- Create HTTP client options and initialize its functionality
- Create HTTP SSL config and initialize the SSL options
- Configure data management options for HTTP client
- Appy all configurations to HTTP client
- Perform a GET request to a server
- Disable PDP context


**`DATA_CB`**

- Print incoming data
- Set the abort flag to 0 to keep going.

![](pictures/samples/http_client_bordered.png)

---------------------



### HW Timer (Hardware Timer)

The sample application shows how to use HW Timers M2MB API. Debug prints on **USB0**


**Features**


- How to open configure a HW timer
- How to use the timer to manage recurring events



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create hw timer structure

- Configure it with 100 ms timeout, periodic timer (auto fires when expires) and autostart

- Init the timer with the parameters

- Wait 10 seconds

- Stop the timer

**`TimerCb`**

- Print a message with an increasing counter

![](pictures/samples/hw_timer_bordered.png)

---------------------



### I2C example 

Sample application showing how to communicate with an I2C slave device. Debug prints on **USB0**


**Features**


- How to open a communication channel with an I2C slave device
- How to send and receive data to/from the slave device



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Open I2C bus, setting SDA an SCL pins as 2 and 3 respectively
- Set registers to configure accelerometer
-Read in a loop the 6 registers carrying the 3 axes values and show the g value for each of them



![](pictures/samples/i2c_bordered.png)

---------------------



### Logging Demo

Sample application showing how to print on one of the available output interfaces. Debug prints on **USB0**


**Features**


- How to open a logging channel
- How to set a logging level 
- How to use different logging macros


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Print a message with every log level

![](pictures/samples/logging_bordered.png)

---------------------



### LWM2M

Sample application showcasing TLS/SSL with client certificates usage with M2MB API. Debug prints on **USB0**


**Features**


- Configure LWM2M client and enable it

- Create an instance of a custom object

- Set an integer value on a read only resource

- Set two integer values on a multi-instance read only resource

- write a string on a read/write resource

- Manage exec requests from the portal

- Manage write, read and monitoring resources changed from the portal

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Create a task to manage the LWM2M client and start it


**`lwm2m_demo.c`**

**`msgLWM2MTask`**
- Check registration status

- Configure APN to the correct one for CID 1

- Initialize LWM2M client, 

  - Check for XML file fo custom object
  
  - Enable unsolicited messages from client
  
  - Create a task \(lwm2m_taskCB is its callback function \)to manage events from Portal
  
  - Enable LwM2M client
  
  - Create a new instance for the custom object
  
  - Wait for client to register to Portal
  
  - Send integer and string values
  
  - Wait for events from server


**`lwm2mIndicationCB`**

- Manage events arriving from client \(operations completion status and unsolicited events\)
- Run lwm2m_taskCB when a monitored resource changes, to manage the action to be done

#### Custom Object configuration

The XML file content must be loaded on the Telit IoT Portal for the demo application to be fully executed.

First, enter Developer section from the top menu

![](pictures/samples/lwm2m_xml_1_developer_bordered.png)


Choose Object Registry

![](pictures/samples/lwm2m_xml_2_object_registry_bordered.png)


Create a New Object

![](pictures/samples/lwm2m_xml_3_new_object_bordered.png)


Copy the xml file content and paste it in the new Object form

![](pictures/samples/lwm2m_xml_4_paste_content_bordered.png)



Also, the application requires the XML file `/xml/object_35000.xml` (provided with the sample files) to be stored in module's `/XML/` folder. 
It can be done with 

`AT#M2MWRITE=/XML/object_35000.xml,<size_in_bytes>`

To load the XML file in the module, Telit AT Controller (TATC) can be used. Once the command above is issued, press the load content button:

![](pictures/samples/lwm2m_xml_5_load_xml_bordered.png)


Select the file from your computer

![](pictures/samples/lwm2m_xml_6_file_select_bordered.png)


The file is successfully loaded on the module

![](pictures/samples/lwm2m_xml_7_done_bordered.png)


#### Application execution example

![](pictures/samples/lwm2m_1_bordered.png)

![](pictures/samples/lwm2m_2_bordered.png)


After the Demo completes the initialization, it is possible to access the object resources from the Portal Object Browser

![](pictures/samples/lwm2m_portal_object_browser_bordered.png)

An instance of the object will be present and the resources can be modified. 

![](pictures/samples/lwm2m_portal_object_bordered.png)

For example, executing the two Exec Resources at the bottom of the list, the application will react accordingly:

![](pictures/samples/lwm2m_3_exec_bordered.png)

Writing a string resource (id /35000/0/11 ), the application will notify the change

![](pictures/samples/lwm2m_4_write_bordered.png)

---------------------



### MQTT Client

Sample application showcasing MQTT client functionalities (with SSL). Debug prints on **USB0**


**Features**


- How to check module registration and enable PDP context
- How to configure MQTT client parameters
- How to connect to a broker with SSL and exchange data over a subscribed topic


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage MQTT client and start it


**`mqtt_demo.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Init MQTT client
- Configure it with all parameters (Client ID, username, password, PDP context ID, keepalive timeout...)

- Connect MQTT client to broker
- Subscribe to two topics
- Publish 10 messages with increasing counter. Even messages are sent to topic 1, odd messages on topic 2.
- Print received message in mqtt_topc_cb function
- Disconnect MQTT client and deinit it 

- Disable PDP context

![](pictures/samples/mqtt_bordered.png)

---------------------



### MultiTask

Sample application showcasing multi tasking functionalities with M2MB API. Debug prints on **USB0**


**Features**


- How to create tasks using azx utilities
- How to use send messages to tasks
- How to use a semaphore to synchronize two tasks


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create three tasks with the provided utility (this calls public m2mb APIs)

- Send a message to the task1, its callback function azx_msgTask1 will be called

**`azx_msgTask1`**

- Print received parameters from main
- Send modified parameters to task2 \(its callback function azx_msgTask2 will be called\)
- wait for an InterProcess Communication semaphore to be available \(released by task3\)
- Once the semaphore is available, print a message and return

**`azx_msgTask2`**

- Print received parameters from caller
- If first parameter is bigger than a certain value, Send modified parameters to task3
- Else, use the second parameter as a task handle and print the corresponding name plus the value of the first parameter

**`azx_msgTask3`**

- Print received parameters from task 2
- release IPC semaphore
- send message to task 2 with first parameter below the threshold and second parameter with task3 handle


![](pictures/samples/multitask_bordered.png)

---------------------



### MutEx

Sample application showing mutex usage, with ownership and prioritization usage. Debug prints on **USB0**


**Features**


- How to create a mutex 
- How to use the mutex with tasks having different priorities
- how to reorder the pending tasks queue for the mutex


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print welcome message
- Create four tasks with the provided utility (this calls public m2mb APIs). The first task is a "producer", putting data on a shared buffer. The second is a "consumer" of said data, the other two are used for prioritization demo
- run producer and consumer tasks at the same pace. the shared buffer will stay empty, because the resource is consumed right after creation
- run producer twice as fast as consumer. The buffer is slowly filled
- run consumer twice as fast as publisher. The buffer is always empty.
- reserve the mutex in the main task and run producer, support and support2 tasks (in this order). Then release the mutex and check the execution order. It should be by arrival.
- reserve the mutex in the main task and run the same three task, but before releasing the mutex, call the prioritization API. the task with highest priority \(producer\) is put as first in the queue.

![](pictures/samples/mutex_1_bordered.png)

![](pictures/samples/mutex_2_bordered.png)

![](pictures/samples/mutex_3_bordered.png)

![](pictures/samples/mutex_4_bordered.png)

---------------------



### SMS PDU

Sample application showcasing how to create and decode PDUs to be used with m2mb_sms_* API set. A SIM card and antenna must be present. Debug prints on **USB0**


**Features**


- How to enable SMS functionality
- How to use encode an SMS PDU to be sent with m2mb_api
- How to decode a received SMS response from PDU to ASCII mode.


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Init sms functionality
- Create PDU from text message
- Send message to destination number
- Wait for response
- When SMS PDU response is received, decode it and print information about it, plus the message content

![](pictures/samples/sms_pdu_bordered.png)

---------------------



### SPI Echo

Sample application showing how to communicate over SPI with m2mb API. Debug prints on **USB0**


**Features**


- How to open an SPI bus. MOSI and MISO will be shorted, to have an echo.
- How to communicate over SPI bus


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Open SPI bus, set parameters

- Send data on MOSI and read the same in MISO


![](pictures/samples/spi_echo_bordered.png)

---------------------



### SPI sensors

Sample application showing SPI usage, configuring two ST devices: a magnetometer (ST LIS3MDL) and a gyroscope (ST L3G4200D). The application will read values from both devices using GPIO4 and 3 (respectively) as magnetometer CS and gyro CS. Debug prints on **USB0**


**Features**


- How to open an SPI bus with a slave device
- How to communicate with the device over the SPI bus

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Open SPI bus, set parameters
- Configure `GPIO 3` and `GPIO 4` as output, set them high (idle)
- Set registers to configure magnetometer
- Read in a loop \(10 iterations\) the registers carrying the 3 axes values and show the gauss value for each of them. A metal object is put close to the sensor to change the read values.
- Set registers to configure gyroscope
- Read in a loop \(10 iterations\) the registers carrying the 3 axes values and show the degrees per second value for each of them. The board is rotated to change the read values.

![](pictures/samples/spi_sensors_bordered.png)

---------------------



### SW Timer (Software Timer)

The sample application shows how to use SW Timers M2MB API. Debug prints on **USB0**


**Features**


- How to open configure a SW timer
- How to use the timer to manage recurring events


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create sw timer structure

- Configure it with 4 seconds timeout, periodic timer (auto fires when expires)

- Init the timer with the parameters

- Start the timer

- Wait 10 seconds

- Stop the timer

**`timerCb`**

- Print a message with inside the callback

![](pictures/samples/sw_timer_bordered.png)

---------------------



### TCP IP 

Sample application showcasing TCP echo demo with M2MB API. Debug prints on **USB0**


**Features**


- How to check module registration and activate PDP context
- How to open a TCP client socket 
- How to communicate over the socket


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage socket and start it

 

**`m2m_tcp_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Create socket and link it to the PDP context id

- Connect to the server

- Send data and receive response

- Close socket

- Disable PDP context

![](pictures/samples/tcp_ip_bordered.png)

---------------------



### TCP Server 

Sample application showcasing TCP listening socket demo with M2MB API. Debug prints on **USB0**


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

![](pictures/samples/tcp_server_bordered.png)

Data on a PuTTY terminal

![](pictures/samples/tcp_server_putty_bordered.png)

---------------------



### TLS SSL Client

Sample application showcasing TLS/SSL with client certificates usage with M2MB API. Debug prints on **USB0**


**Features**


- How to check module registration and enable PDP context
- How to open a SSL client socket
- How to communicate over SSL socket



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Create a task to manage the connection and start it

 

**`ssl_test.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Create socket and link it to the PDP context id

- Connect to the server over TCP socket

- Initialize the TLS parameters (TLS1.2) andh auth mode (server+client auth in the example)

- Create SSL context

- Read certificates files and store them

- Create secure socket and connect to the server using SSL

- Send data and receive response

- Close secure socket

- Close socket

- Delete SSL context

- Disable PDP context

 
The application requires the certificates to be stored in `/test_ssl_certs/` folder. It can be created with `AT#M2MMKDIR=/test_ssl_certs`


![](pictures/samples/tcp_ssl_client_bordered.png)

---------------------



### UDP client

Sample application showcasing UDP echo demo with M2MB API. Debug prints on **USB0**


**Features**


- How to check module registration and activate PDP context
- How to open a UDP client socket 
- How to communicate over the socket


**Application workflow**

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

![](pictures/samples/udp_bordered.png)

---------------------



### ZLIB example 

Sample application showing how to compress/uncompress with ZLIB. Debug prints on **USB0**


**Features**


- How to compress a file
- How to uncompress a file

In order to execute the entire test, copy `test.gz` file into your module running the following AT command: 

```
AT#M2MWRITE="/mod/test.gz",138
>>> here receive the prompt; then type or send the file, sized 138 bytes
```

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Test the compression and decompression of a data string
- Test the decompression of a .gz file (test.gz), expected to be in `/mod` folder, into its content `test.txt`. The file must be uploaded by the user (see steps above).

![](pictures/samples/zlib_bordered.png)

---------------------



# Installing beta version libraries Plug-in

## New beta plug-in installation

To install a new plug-in for a beta firmware into the IDE, first receive plug-in ".zip" packet, then unzip the file in a local folder and open the SDK IDE.

**PLEASE DO NOT USE BETA PLUGINS FOR PRODUCTION DEPLOYMENTS, SOFTWARE IS PROVIDED AS IS AND CUSTOMER ACKNOWLEDGES THAT IT IS POSSIBLE THE DEVICE MAY MISFUNCTION. PLEASE REFER TO Contact Information, Support section**

![](pictures/help.png)



Click on "Help" tag and choose "Install New Software...". This window will appear:

![](pictures/add.png)


Click on "Add..." button and then in the following window click on "Local..." to select the unzipped folder with the plug-in content.

 ![](pictures/add2.png)
 
 ![](pictures/browse.png)


Once selected the plug-in folder, the "Location:" form will present the selected path. Now in "Name:" write a name for the new libraries (for example 37.00.xx0_B037) and click on "OK" button.

![](pictures/local_name.png)


The new packet is now ready to be installed: select it and click on "Next >" button until "Review Licenses" window will appear.

![](pictures/select.png)


Accept the licenses when required and click on "Finish" button to complete the installation.


## Change existing project libraries

To align an old project to the new libraries, right click on the project and choose "Properties".

![](pictures/change.png)

Now select "AppZone" on the left side of the window, and on the right choose the packet with the same name as the firmware version to be used. Then click on "OK" (or "Apply") button. 

## Create a project with the new plug-in

To use the new libraries, create a new project: "File"-> "New" -> "Telit Project"

![](pictures/new_proj.png)

Select the new firmware version (37.00.xx0-B037) and create an empty project.

