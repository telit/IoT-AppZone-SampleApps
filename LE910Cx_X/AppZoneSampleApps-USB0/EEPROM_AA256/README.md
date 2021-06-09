
### EEPROM 24AA256

Sample application showing how to communicate with a MicroChip 24AA256T I2C EEPROM chip using azx eeprom utility APIs. Debug prints on **USB0**

**Setup**

This demo application requires that:
- A0, A1, and A2 pins (1,2,3 chip pins) are connected to ground (pin 4) for device address 0xA0
- Pin 7 (WP) is connected to ground
- Pin 6 (SCL) is connected to module GPIO 3
- Pin 5 (SDA) is connected to module GPIO 2
- Pin 4 is connected to one of the ground pins of the module
- Pin 8 is connected to 1v8 supply (e.g. VPWRMON pin on the module)

**Features**


- Initialize the logs on the output channel
- configure the EEPROM utility, setting the slave address and the memory parameters (page size, memory size)
- Write single bytes on a random address
- Read written bytes as a page
- Write data using pages
- Read the new data using pages
- Read again using sequential reading
- Read a single byte from a specific address
- Read next byte using read from current address
- Erase the EEPROM
- Deinit EEPROM utility


**Application workflow**

**`M2MB_main.c`**

- call azx_eeprom_init() to set the utility parameters (SDA and SCL pins, page and memory sizes)
- call azx_eeprom_writeByte() to store a single byte with value '5' at the address 0x0213
- call azx_eeprom_writeByte() to store a single byte with value '6' at the address 0x0214
- call azx_eeprom_readPages() from address 0x0213 to retrieve the 2 bytes from the EEPROM
- call azx_eeprom_writePages to write 1024 bytes from a buffer, starting from address 0x00
- call azx_eeprom_readPages() again, to read 256 bytes from address 0x00
- call azx_eeprom_readSequentially() to read 256 bytes from 0x00 by without pages (less overhead on I2C protocol)
- call azx_eeprom_readByte() to get a single byte from address 0x00
- call azx_eeprom_readByteFromCurrentAddress() to get a byte from next address (0x01)
- call azx_eeprom_eraseAll() to completely erase the EEPROM memory (this writes 0xFF in each byte)
- call azx_eeprom_readPages from address 0x0213 to get 2 bytes and verify the values have been written to 0xFF
- call azx_eeprom_deinit to close the eeprom handler and the I2C channel

![](../../pictures/samples/eeprom_AA256_bordered.png)

---------------------

