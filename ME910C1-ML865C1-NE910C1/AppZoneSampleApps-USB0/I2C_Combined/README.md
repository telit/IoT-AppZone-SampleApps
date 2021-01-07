
### I2C Combined

Sample application showing how to communicate with an I2C slave device with I2C raw mode. Debug prints on **USB0**


**Features**


- How to open a communication channel with an I2C slave device
- How to send and receive data to/from the slave device using raw mode API



**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Open I2C bus, setting SDA an SCL pins as 2 and 3 respectively
- Set registers to configure accelerometer
-Read in a loop the 6 registers carrying the 3 axes values and show the g value for each of them



![](../../pictures/samples/i2c_combined_bordered.png)

---------------------

