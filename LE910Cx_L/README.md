

# AppZone m2mb Sample Apps 



Package Version: **1.1.10-CxL**

Minimum Firmware Version: **25.21.XX0**


## Features

This package goal is to provide sample source code for common activities kickstart.
 

# Quick start

## Deployment Instructions


To manually deploy the Sample application on the devices perform the following steps:

1. Have **25.21.XX0** FW version flashed (`AT#SWPKGV` will give you the FW version)

1. Copy _m2mapz.bin_ to _/data/azc/mod/_ 
	```
	AT#M2MWRITE="/data/azc/mod/m2mapz.bin",<size>,1
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
    AT#M2MDEL="/data/azc/mod/m2mapz.bin"
    ```
	+ Delete everything, reflash and retry
    ```
    AT#M2MDEL="/data/azc/mod/m2mapz.bin"
    AT#M2MDEL="/data/azc/mod/appcfg.ini"
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



## Main contents

[MAIN UART](#main-uart)

[USB0](#usb0)

[BASIC](#basic)

[MISC](#misc)

[Installing beta version libraries Plug-in](#installing-beta-version-libraries-plug-in)


# Applications 

## MISC 
*Applications that provide usage examples for various functionalities, without prints*


### GPIO toggle example 

Sample application showcasing GPIO usage with M2MB API


**Features**


- How to open a gpio in output mode and change its status




## MAIN UART 
*Applications that provide usage examples for various functionalities, log output on MAIN UART*


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




### AWS demo

Sample application showcasing AWS Iot Core MQTT communication. Debug prints on **MAIN UART**


**Features**


- How to check module registration and enable PDP context
- How to load certificates into device SSL session storage
- How to configure MQTT client parameters
- How to connect to AWS server with SSL and exchange data over a topic


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage MQTT client and start it


**`aws_demo.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Init MQTT client

- Configure it with all parameters (Client ID, PDP context ID, keepalive timeout...)

- Initialize the TLS parameters (TLS1.2) andh auth mode (server+client auth in the example)

- Create SSL context

- Read certificates files and store them

- Connect MQTT client to broker
- Subscribe to topic
- Publish 10 messages with increasing counter
- Print received message in mqtt_topc_cb function
- Disconnect MQTT client and deinit it 

- Disable PDP context


### How to get started with AWS IoT

- Go to [AWS console](https://aws.amazon.com/console/) and create an account if one is not available yet.
- Go to **`IoT Core`** section
- Go to **`Secure`** > **`Policies`** section
- Create a new policy, which describes what the device will be allowed to do (e.g. subscribe, publish)
- Give it a name, then configure it using the configuration below (it is possible to copy/paste by clicking on **`Add statements`** section, then **`Advanced mode`** ) :
```json
{
  "Version": "2012-10-17",
  "Statement": [
  {
    "Action": [
      "iot:Publish",
      "iot:Subscribe",
      "iot:Connect",
      "iot:Receive"
    ],
    "Effect": "Allow",
    "Resource": [
      "*"
    ]
  }
  ]
}
```

- Click on create to complete the policy creation.
- Go to **`Manage`** section
- Press **`Create`**, then **`Create a single thing`**
- Give the new thing a name, then click on `Next`
- Select **`One-click certificate creation (recommended)`** by clicking on **`Create certificate`**
- Once presented with the **`Certificate created`** page, download all certificates and keys
- Click on the **`Activate`** button to enable the certificate authentication of the newly created device
- Click on **`Attach a policy`** and select the policy created in a previous step

For further information, please refer to the full [AWS IoT documentation](https://docs.aws.amazon.com/iot/latest/developerguide/iot-console-signin.html)

### Application setup

- Set **`CLIENTCERTFILE`** and **`CLIENTKEYFILE`** defines in **`aws_demo.c file`** in order to match the certificate and key created in the previous section.
- Set **`AWS_BROKER_ADDRESS`** to the correct AWS URL. It can be retrieved from AWS IoT **`Manage`** > **`Things`** > **`Interact`** in the HTTPS **`Rest API Endpoint`** URL. 
- Set **`CLIENT_ID`** to the desired Client ID for your AWS device
- (Optional) if required, change **`CACERTFILE`** to match the one to be used.

### Device setup

The application requires the certificates (provided in sample app **`certs`** subfolder ) to be stored in **`/data/azc/mod/ssl_certs/`** folder. It can be created with 

`AT#M2MMKDIR=/data/azc/mod/ssl_certs`

Certificates can then be loaded with

`AT#M2MWRITE="/data/azc/mod/ssl_certs/preload_CACert_01.crt",1468`
`AT#M2MWRITE="/data/azc/mod/ssl_certs/Amazon-IoT.crt",1646`

providing the file content in RAW mode (for example using the "Transfer Data" button in Telit AT Controller)

For client certificates, the commands will be

```
AT#M2MWRITE="/data/azc/mod/ssl_certs/xxxxx.crt",yyyy
AT#M2MWRITE="/data/azc/mod/ssl_certs/xxxxx.key",zzzz
```

PLEASE NOTE: always verify the file sizes to be used in the commands above as they might change

![](pictures/samples/aws_bordered.png)

Data received from a subscriber:

![](pictures/samples/aws2_bordered.png)

---------------------



### App Manager

Sample application showing how to manage AppZone apps from m2mb code. Debug prints on **MAIN UART**


**Features**


- How to get how many configured apps are available
- How to get the handle to manage the running app (change start delay, enable/disable)
- How to create the handle for a new binary app, enable it and set its parameters
- How to start the new app without rebooting the device, then stop it after a while.

#### Prerequisites

This app will try to manage another app called "second.bin", which already exists in the module filesystem and can be anything (e.g. another sample app as GPIO toggle).
the app must be built using the flag ROM_START=<address> in the Makefile to set a different starting address than the main app (by default, 0x40000000). For example, 0x41000000.


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- get a non existing app handle and verify it is NULL
- get the current app handle, then get the start delay **set in the INI file (so persistent)**
- change the current app delay value **in the INI file**
- verify that the change has been stored
- get current app state
- create an handle for a second application binary. 
- add it to the INI file
- set its execution flag to 0
- get the delay time and the state from INI file for the new app
- get the current set address for the new app
- set the app delay **in RAM, INI will not be affected**.
- start the new app without reboot, using the right set delay
- wait some time, then get the app state and the used RAM amount
- wait 10 seconds, then stop the second app.
- set its execution flag to 1 so it will run at next boot.

![](pictures/samples/appManager_bordered.png)

---------------------



### App update OTA via FTP

Sample application showcasing Application OTA over FTP with AZX FTP. Debug prints on **MAIN UART**


**Features**


- How to check module registration and activate PDP context
- How to connect to a FTP server 
- How to download an application binary and update the local version

The app uses a predefined set of parameters. To load custom parameters, upload the `ota_config.txt` file (provided in project's `/src` folder) in module's `/data/azc/mod` folder, for example with 

```
AT#M2MWRITE="/data/azc/mod/ota_config.txt",<filesize>

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
- Download the file in `/data/azc/mod` folder, with the provided name
- Close FTP connection
- Disable PDP context
- Update applications configuration in **app_utils.c**

**`app_utils.c`**

- Set new application as default
- Delete old app binary
- Restart module

![](pictures/samples/app_ftp_ota_bordered.png)

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



### Easy AT example 

Sample application showcasing Easy AT functionalities. Debug prints on **MAIN UART**


**Features**


- Shows how to register custom commands





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



### General_INFO example 

Sample application prints some Module/SIM information as IMEI, fw version, IMSI and so on; it prints also some information about registration. Debug prints on **MAIN UART**


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

![](pictures/samples/general_INFO_bordered.png)

---------------------



### HTTP Client

Sample application showing how to use HTTPs client functionalities. Debug prints on **MAIN UART**


**Features**


- How to check module registration and activate PDP context
- How to initialize the http client, set the debug hook function and the data callback to manage incoming data
- How to perform GET, HEAD or POST operations

NOTE: the sample app has an optional dependency on azx_base64.h if basic authentication is required (refer to `HTTP_BASIC_AUTH_GET` define in `M2MB_main.c` for further details)

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



### Little FileSystem 2 

Sample application showing how use lfs2 porting with RAM disk and SPI data flash. Debug prints on **MAIN UART**


**Features**


- How to create and manage Ram Disk
- How to manage file-system in Ram disk partition
- How to create and manage SPI Flash memory partition
- How to manage file-system in SPI Flash memory partition

**Application workflow**

**`M2MB_main.c`**

- Init logging system
- Call Ram Disk tests
- Call Flash memory tests

**`ram_utils_usage.c`**

- Initialize Ram Disk
- Format and Mount partition
- List files 
- Files creation and write content
- List files 
- Read files 
- Unmount and Release resources


**`spi_utils_usage.c`**
- Initialize SPI Flash chip
- Initialize SPI Flash Disk
- Format and Mount partition
- List files 
- Files creation and write content
- List files 
- Read files 
- Delete files
- Directories creation and deletion
- Unmount and Release resources

**Notes:**

For SPI Flash a JSC memory is used with chip select pin connected to module GPIO2 pin.
For better performances, a 33kOhm pull-down resistor on SPI clock is suggested.
Please refer to SPI_echo sample app for SPI connection details.

For LE910Cx (both Linux and ThreadX based devices), `AT#SPIEN=1` command must be sent once before running the app

![](pictures/samples/lfs2_ramdisk_bordered.png)
![](pictures/samples/lfs2_spiflash_01_bordered.png)
![](pictures/samples/lfs2_spiflash_02_bordered.png)
![](pictures/samples/lfs2_spiflash_03_bordered.png)



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



### MD5 example 

Sample application showing how to compute MD5 hashes using m2mb crypto. Debug prints on **MAIN UART**


**Features**

- Compute MD5 hash of a file
- Compute MD5 hash of a string


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create a temporary file with the expected content
- Compute MD5 hash of the provided text file
- Compare the hash with the expected one
- Compute MD5 hash of a string
- Compare the hash with the expected one
- Delete test file

![](pictures/samples/md5_bordered.png)

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



### NTP example 

The application connects to an NTP server, gets current date and time and updates module's internal clock. Debug prints on **MAIN UART**


**Features**


- How to get current date and time from an NTP server
- How to set current date and time on module



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print welcome message
- Send message to ntpTask

**`ntp_task.c`**

*NTP_task()*
- Waits module registration
- When module is registered, initializes ntp setting CID, server url and timeout
- When PDP context is correctly opened, a query to NTP server is done to get current date and time
- On SET_MODULE_RTC message type reception, module RTC is set with date time value got from NTP server.

*m2mb_ntp_ind_callback()*
- As soon as M2MB_NTP_VALID_TIME event is received, current date and time is printend and a message (with SET_MODULE_RTC type) is sent to NTP_task


![](pictures/samples/NTP_bordered.png)

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



### SMS_atCmd example 

Sample application showcasing how to receive an SMS containing an AT command, process the AT command and send its answer to sender (configurable in sms_config.txt). A SIM card and antenna must be present. Debug prints on **MAIN UART**


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

![](pictures/samples/sms_atCmd_bordered.png)

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

**Notes:**

For LE910Cx (both Linux and ThreadX based devices), `AT#SPIEN=1` command must be sent once before running the app

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
- Configure `GPIO 2` and `GPIO 3` as output, set them high (idle)
- Set registers to configure magnetometer
- Read in a loop \(10 iterations\) the registers carrying the 3 axes values and show the gauss value for each of them. A metal object is put close to the sensor to change the read values.
- Set registers to configure gyroscope
- Read in a loop \(10 iterations\) the registers carrying the 3 axes values and show the degrees per second value for each of them. The board is rotated to change the read values.

**Notes:**

For LE910Cx (both Linux and ThreadX based devices), `AT#SPIEN=1` command must be sent once before running the app

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



### TCP Socket status

Sample application showcasing how to check a TPC connected socket current status. Debug prints on **MAIN UART**


**Features**


- How to check module registration and activate PDP context
- How to open a TCP client socket 
- How to check if the TCP socket is still valid

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

- Check in a loop the current socket status using the adv_select function with a 2 seconds timeout

- Close socket when the remote host closes it

- Disable PDP context


![](pictures/samples/tcp_status_bordered.png)

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


The application requires the certificates to be stored in `/data/azc/mod/ssl_certs/` folder. It can be created with 

`AT#M2MMKDIR=/data/azc/mod/ssl_certs`

Certificates can then be loaded with

`AT#M2MWRITE="/data/azc/mod/ssl_certs/data/azc/modulesCA.crt",1740`

and providing the file content in RAW mode (for example using the "Transfer Data" button in Telit AT Controller)

For client certificates (if required), the commands will be

```
AT#M2MWRITE="/data/azc/mod/ssl_certs/data/azc/modulesClient.crt",1651
AT#M2MWRITE="/data/azc/mod/ssl_certs/data/azc/modulesClient_pkcs1.key",1679
```

PLEASE NOTE: always verify the file sizes to be used in the commands above as they might change

![](pictures/samples/tcp_ssl_client_bordered.png)
![](pictures/samples/tcp_ssl_client_2_bordered.png)

---------------------



### Uart To Server

Sample application showcasing how to send data from main UART to a connected TCP server. Debug messages are printed on AUX UART port.


**Features**


- How to open main UART to receive data
- How to connect to a server
- How to transmit received data from the UART to the server and viceversa


**Application workflow**

**`M2MB_main.c`**

- Open UART for data and USB1 for debug
- Init socket, activate PDP context and connect to server
- Init UART, set its callback function, create tasks to handle input from UART and response from server (optional)
- Send a confirmation on UART
- Wait for data, when it is received, send it to the server
- When a response is received, print it on UART.

Main UART: 

![](pictures/samples/uart_to_server_main_bordered.png)

Debug log on USB1:

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



### USB Cable Check 

Sample application showing how to check if USB cable is plugged in or not. Debug prints on **MAIN UART**


**Features**


- How to open an USB channel and configure it with a callback function
- How to manage USB cable events in the callback function



**Application workflow**

**`M2MB_main.c`**

- Open UART/UART_AUX for debug
- open usb channel and set the callback
- Print greeting message
- Print current usb status

**`USB_Cb`**

- if the event is a connection/disconnection, show the current status

![](pictures/samples/usb_cable_check_bordered.png)

---------------------



### ZLIB example 

Sample application showing how to compress/uncompress with ZLIB. Debug prints on **MAIN UART**


**Features**


- How to compress a file
- How to uncompress a file

In order to execute the entire test, copy `test.gz` file into your module running the following AT command: 

```
AT#M2MWRITE="/data/azc/mod/test.gz",138
>>> here receive the prompt; then type or send the file, sized 138 bytes
```

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Test the compression and decompression of a data string
- Test the decompression of a .gz file (test.gz), expected to be in `/data/azc/mod` folder, into its content `test.txt`. The file must be uploaded by the user (see steps above).

![](pictures/samples/zlib_bordered.png)

---------------------



## BASIC 
*Basic applications showing simple operations with minimum code overhead*


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



## USB0 
*Applications that provide usage examples for various functionalities, log output on USB0*


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



### AWS demo

Sample application showcasing AWS Iot Core MQTT communication. Debug prints on **USB0**


**Features**


- How to check module registration and enable PDP context
- How to load certificates into device SSL session storage
- How to configure MQTT client parameters
- How to connect to AWS server with SSL and exchange data over a topic


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create a task to manage MQTT client and start it


**`aws_demo.c`**

- Initialize Network structure and check registration

- Initialize PDP structure and start PDP context

- Init MQTT client

- Configure it with all parameters (Client ID, PDP context ID, keepalive timeout...)

- Initialize the TLS parameters (TLS1.2) andh auth mode (server+client auth in the example)

- Create SSL context

- Read certificates files and store them

- Connect MQTT client to broker
- Subscribe to topic
- Publish 10 messages with increasing counter
- Print received message in mqtt_topc_cb function
- Disconnect MQTT client and deinit it 

- Disable PDP context


### How to get started with AWS IoT

- Go to [AWS console](https://aws.amazon.com/console/) and create an account if one is not available yet.
- Go to **`IoT Core`** section
- Go to **`Secure`** > **`Policies`** section
- Create a new policy, which describes what the device will be allowed to do (e.g. subscribe, publish)
- Give it a name, then configure it using the configuration below (it is possible to copy/paste by clicking on **`Add statements`** section, then **`Advanced mode`** ) :
```json
{
  "Version": "2012-10-17",
  "Statement": [
  {
    "Action": [
      "iot:Publish",
      "iot:Subscribe",
      "iot:Connect",
      "iot:Receive"
    ],
    "Effect": "Allow",
    "Resource": [
      "*"
    ]
  }
  ]
}
```

- Click on create to complete the policy creation.
- Go to **`Manage`** section
- Press **`Create`**, then **`Create a single thing`**
- Give the new thing a name, then click on `Next`
- Select **`One-click certificate creation (recommended)`** by clicking on **`Create certificate`**
- Once presented with the **`Certificate created`** page, download all certificates and keys
- Click on the **`Activate`** button to enable the certificate authentication of the newly created device
- Click on **`Attach a policy`** and select the policy created in a previous step

For further information, please refer to the full [AWS IoT documentation](https://docs.aws.amazon.com/iot/latest/developerguide/iot-console-signin.html)

### Application setup

- Set **`CLIENTCERTFILE`** and **`CLIENTKEYFILE`** defines in **`aws_demo.c file`** in order to match the certificate and key created in the previous section.
- Set **`AWS_BROKER_ADDRESS`** to the correct AWS URL. It can be retrieved from AWS IoT **`Manage`** > **`Things`** > **`Interact`** in the HTTPS **`Rest API Endpoint`** URL. 
- Set **`CLIENT_ID`** to the desired Client ID for your AWS device
- (Optional) if required, change **`CACERTFILE`** to match the one to be used.

### Device setup

The application requires the certificates (provided in sample app **`certs`** subfolder ) to be stored in **`/data/azc/mod/ssl_certs/`** folder. It can be created with 

`AT#M2MMKDIR=/data/azc/mod/ssl_certs`

Certificates can then be loaded with

`AT#M2MWRITE="/data/azc/mod/ssl_certs/preload_CACert_01.crt",1468`
`AT#M2MWRITE="/data/azc/mod/ssl_certs/Amazon-IoT.crt",1646`

providing the file content in RAW mode (for example using the "Transfer Data" button in Telit AT Controller)

For client certificates, the commands will be

```
AT#M2MWRITE="/data/azc/mod/ssl_certs/xxxxx.crt",yyyy
AT#M2MWRITE="/data/azc/mod/ssl_certs/xxxxx.key",zzzz
```

PLEASE NOTE: always verify the file sizes to be used in the commands above as they might change

![](pictures/samples/aws_bordered.png)

Data received from a subscriber:

![](pictures/samples/aws2_bordered.png)

---------------------



### App Manager

Sample application showing how to manage AppZone apps from m2mb code. Debug prints on **USB0**


**Features**


- How to get how many configured apps are available
- How to get the handle to manage the running app (change start delay, enable/disable)
- How to create the handle for a new binary app, enable it and set its parameters
- How to start the new app without rebooting the device, then stop it after a while.

#### Prerequisites

This app will try to manage another app called "second.bin", which already exists in the module filesystem and can be anything (e.g. another sample app as GPIO toggle).
the app must be built using the flag ROM_START=<address> in the Makefile to set a different starting address than the main app (by default, 0x40000000). For example, 0x41000000.


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- get a non existing app handle and verify it is NULL
- get the current app handle, then get the start delay **set in the INI file (so persistent)**
- change the current app delay value **in the INI file**
- verify that the change has been stored
- get current app state
- create an handle for a second application binary. 
- add it to the INI file
- set its execution flag to 0
- get the delay time and the state from INI file for the new app
- get the current set address for the new app
- set the app delay **in RAM, INI will not be affected**.
- start the new app without reboot, using the right set delay
- wait some time, then get the app state and the used RAM amount
- wait 10 seconds, then stop the second app.
- set its execution flag to 1 so it will run at next boot.

![](pictures/samples/appManager_bordered.png)

---------------------



### App update OTA via FTP

Sample application showcasing Application OTA over FTP with AZX FTP. Debug prints on **USB0**


**Features**


- How to check module registration and activate PDP context
- How to connect to a FTP server 
- How to download an application binary and update the local version

The app uses a predefined set of parameters. To load custom parameters, upload the `ota_config.txt` file (provided in project's `/src` folder) in module's `/data/azc/mod` folder, for example with 

```
AT#M2MWRITE="/data/azc/mod/ota_config.txt",<filesize>

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
- Download the file in `/data/azc/mod` folder, with the provided name
- Close FTP connection
- Disable PDP context
- Update applications configuration in **app_utils.c**

**`app_utils.c`**

- Set new application as default
- Delete old app binary
- Restart module

![](pictures/samples/app_ftp_ota_bordered.png)

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



### Easy AT example 

Sample application showcasing Easy AT functionalities. Debug prints on **USB0**


**Features**


- Shows how to register custom commands





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

![](pictures/samples/general_INFO_bordered.png)

---------------------



### HTTP Client

Sample application showing how to use HTTPs client functionalities. Debug prints on **USB0**


**Features**


- How to check module registration and activate PDP context
- How to initialize the http client, set the debug hook function and the data callback to manage incoming data
- How to perform GET, HEAD or POST operations

NOTE: the sample app has an optional dependency on azx_base64.h if basic authentication is required (refer to `HTTP_BASIC_AUTH_GET` define in `M2MB_main.c` for further details)

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



### Little FileSystem 2 

Sample application showing how use lfs2 porting with RAM disk and SPI data flash. Debug prints on **USB0**


**Features**


- How to create and manage Ram Disk
- How to manage file-system in Ram disk partition
- How to create and manage SPI Flash memory partition
- How to manage file-system in SPI Flash memory partition

**Application workflow**

**`M2MB_main.c`**

- Init logging system
- Call Ram Disk tests
- Call Flash memory tests

**`ram_utils_usage.c`**

- Initialize Ram Disk
- Format and Mount partition
- List files 
- Files creation and write content
- List files 
- Read files 
- Unmount and Release resources


**`spi_utils_usage.c`**
- Initialize SPI Flash chip
- Initialize SPI Flash Disk
- Format and Mount partition
- List files 
- Files creation and write content
- List files 
- Read files 
- Delete files
- Directories creation and deletion
- Unmount and Release resources

**Notes:**

For SPI Flash a JSC memory is used with chip select pin connected to module GPIO2 pin.
For better performances, a 33kOhm pull-down resistor on SPI clock is suggested.
Please refer to SPI_echo sample app for SPI connection details.

For LE910Cx (both Linux and ThreadX based devices), `AT#SPIEN=1` command must be sent once before running the app

![](pictures/samples/lfs2_ramdisk_bordered.png)
![](pictures/samples/lfs2_spiflash_01_bordered.png)
![](pictures/samples/lfs2_spiflash_02_bordered.png)
![](pictures/samples/lfs2_spiflash_03_bordered.png)



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



### MD5 example 

Sample application showing how to compute MD5 hashes using m2mb crypto. Debug prints on **USB0**


**Features**

- Compute MD5 hash of a file
- Compute MD5 hash of a string


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create a temporary file with the expected content
- Compute MD5 hash of the provided text file
- Compare the hash with the expected one
- Compute MD5 hash of a string
- Compare the hash with the expected one
- Delete test file

![](pictures/samples/md5_bordered.png)

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



### NTP example 

The application connects to an NTP server, gets current date and time and updates module's internal clock. Debug prints on **USB0**


**Features**


- How to get current date and time from an NTP server
- How to set current date and time on module



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Print welcome message
- Send message to ntpTask

**`ntp_task.c`**

*NTP_task()*
- Waits module registration
- When module is registered, initializes ntp setting CID, server url and timeout
- When PDP context is correctly opened, a query to NTP server is done to get current date and time
- On SET_MODULE_RTC message type reception, module RTC is set with date time value got from NTP server.

*m2mb_ntp_ind_callback()*
- As soon as M2MB_NTP_VALID_TIME event is received, current date and time is printend and a message (with SET_MODULE_RTC type) is sent to NTP_task


![](pictures/samples/NTP_bordered.png)

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



### SMS_atCmd example 

Sample application showcasing how to receive an SMS containing an AT command, process the AT command and send its answer to sender (configurable in sms_config.txt). A SIM card and antenna must be present. Debug prints on **USB0**


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

![](pictures/samples/sms_atCmd_bordered.png)

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

**Notes:**

For LE910Cx (both Linux and ThreadX based devices), `AT#SPIEN=1` command must be sent once before running the app

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
- Configure `GPIO 2` and `GPIO 3` as output, set them high (idle)
- Set registers to configure magnetometer
- Read in a loop \(10 iterations\) the registers carrying the 3 axes values and show the gauss value for each of them. A metal object is put close to the sensor to change the read values.
- Set registers to configure gyroscope
- Read in a loop \(10 iterations\) the registers carrying the 3 axes values and show the degrees per second value for each of them. The board is rotated to change the read values.

**Notes:**

For LE910Cx (both Linux and ThreadX based devices), `AT#SPIEN=1` command must be sent once before running the app

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



### TCP Socket status

Sample application showcasing how to check a TPC connected socket current status. Debug prints on **USB0**


**Features**


- How to check module registration and activate PDP context
- How to open a TCP client socket 
- How to check if the TCP socket is still valid

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

- Check in a loop the current socket status using the adv_select function with a 2 seconds timeout

- Close socket when the remote host closes it

- Disable PDP context


![](pictures/samples/tcp_status_bordered.png)

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


The application requires the certificates to be stored in `/data/azc/mod/ssl_certs/` folder. It can be created with 

`AT#M2MMKDIR=/data/azc/mod/ssl_certs`

Certificates can then be loaded with

`AT#M2MWRITE="/data/azc/mod/ssl_certs/data/azc/modulesCA.crt",1740`

and providing the file content in RAW mode (for example using the "Transfer Data" button in Telit AT Controller)

For client certificates (if required), the commands will be

```
AT#M2MWRITE="/data/azc/mod/ssl_certs/data/azc/modulesClient.crt",1651
AT#M2MWRITE="/data/azc/mod/ssl_certs/data/azc/modulesClient_pkcs1.key",1679
```

PLEASE NOTE: always verify the file sizes to be used in the commands above as they might change

![](pictures/samples/tcp_ssl_client_bordered.png)
![](pictures/samples/tcp_ssl_client_2_bordered.png)

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
AT#M2MWRITE="/data/azc/mod/test.gz",138
>>> here receive the prompt; then type or send the file, sized 138 bytes
```

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Test the compression and decompression of a data string
- Test the decompression of a .gz file (test.gz), expected to be in `/data/azc/mod` folder, into its content `test.txt`. The file must be uploaded by the user (see steps above).

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

