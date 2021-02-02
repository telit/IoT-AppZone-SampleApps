
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

![](../../pictures/samples/app_ftp_ota_bordered.png)

---------------------

