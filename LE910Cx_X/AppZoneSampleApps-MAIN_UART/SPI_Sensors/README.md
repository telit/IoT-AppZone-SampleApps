
### SPI sensors

Sample application showing SPI usage, configuring two ST devices: a magnetometer (ST LIS3MDL) and a gyroscope (ST L3G4200D). The application will read values from both devices using GPIO4 and 3 (respectively) as magnetometer CS and gyro CS. Debug prints on **MAIN UART**


**Features**


- How to open an SPI bus with a slave device
- How to communicate with the device over the SPI bus

**Setup**

- Connect sensor VDD to 3v8 supply (e.g. Vbatt on the module)
- Connect sensor GND to a GND pin of the module
- Connect sensors MOSI to module SPI_MOSI
- Connect sensors MISO to module SPI_MISO
- Connect sensors CLK to module SPI_CLK
- Connect magnetometer CS to module GPIO 2
- Connect gyroscope CS to module GPIO 3


#### Application workflow

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

![](../../pictures/samples/spi_sensors_bordered.png)

---------------------

