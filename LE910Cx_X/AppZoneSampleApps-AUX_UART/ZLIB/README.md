
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

#### Application workflow

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Test the compression and decompression of a data string
- Test the decompression of a .gz file (test.gz), expected to be in `/mod` folder, into its content `test.txt`. The file must be uploaded by the user (see steps above).

![](../../pictures/samples/zlib_bordered.png)

---------------------

