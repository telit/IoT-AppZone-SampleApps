
### MD5 example 

Sample application showing how to compute MD5 hashes using m2mb crypto. Debug prints on **USB0**


**Features**

- Compute MD5 hash of a file
- Compute MD5 hash of a string


#### Application workflow

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create a temporary file with the expected content
- Compute MD5 hash of the provided text file
- Compare the hash with the expected one
- Compute MD5 hash of a string
- Compare the hash with the expected one
- Delete test file

![](../../pictures/samples/md5_bordered.png)

---------------------

