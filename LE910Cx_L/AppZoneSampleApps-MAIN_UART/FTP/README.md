
### FTP

Sample application showcasing FTP client demo with AZX FTP. Debug prints on **MAIN UART**


**Features**


- How to check module registration and activate PDP context
- How to connect to a FTP server 
- How to exchange data with the server


#### Application workflow

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

![](../../pictures/samples/ftp_bordered.png)

---------------------

