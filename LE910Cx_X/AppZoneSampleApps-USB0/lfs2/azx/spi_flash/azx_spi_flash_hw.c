/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
 @file
 	 azx_spi_flash_hw.c

 @brief
 	 Contains the hardware dependency functions for SPI NAND FLASH JSC JS28(P_U)xGQSxHG-83 family.

 @details

 @version
 	 1.0.0

 @note

 @author
 	 Norman Argiolas

 @date
 	 File created on: Feb 13, 2020
 */

/* Include files ================================================================================*/
//------------- include NAND JSC dependancies
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stdarg.h"

#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_spi.h"
#include "m2mb_gpio.h"
#include "m2mb_os.h"
#include "m2mb_rtc.h"
#include "azx_log.h"
#include "azx_utils.h"

#include "azx_spi_flash_types.h"
#include "azx_spi_flash_hw.h"

/* Global defines ================================================================================*/

/* Local function prototypes ====================================================================*/
INT32 		   			openGpioOutput	(int pin);
INT32	  	   			setGPIO			(INT32 fd, AZX_SPI_FLASH_GPIO_VALUE_E value);
AZX_SPI_FLASH_CODE_RESULT_E 	spiRead 		(INT32 fd, void *buf, SIZE_T nbyte);
AZX_SPI_FLASH_CODE_RESULT_E 	spiWrite 		(INT32 fd, const void *buf, SIZE_T nbyte);


/* Local functions definition ===================================================================*/


/*******************************************************************************
                  setGPIO
Function:
Arguments:
Description:

*******************************************************************************/
INT32 setGPIO( INT32 fd, AZX_SPI_FLASH_GPIO_VALUE_E value )
{
	if(fd < 0)
	{
		return -1;
	}
	return m2mb_gpio_write( fd, (M2MB_GPIO_VALUE_E) value );
}
/***************** setGPIO ***********************************************/


/*******************************************************************************
                  spiRead
Function:
Arguments:
Description:

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E spiRead ( INT32 fd, void *buf, SIZE_T nbyte )
{
	SSIZE_T ret_r = -1;
	ret_r = m2mb_spi_read(fd, buf, nbyte);
	if (ret_r != -1 && (SIZE_T) ret_r == nbyte)
	{
		return AZX_SPI_FLASH_CODE_PASS;
	}
	return AZX_SPI_FLASH_CODE_FAIL;
}
/***************** spiRead ***********************************************/


/*******************************************************************************
                  spiWrite
Function:
Arguments:
Description:

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E spiWrite ( INT32 fd, const void *buf, SIZE_T nbyte )
{
	INT32 ret_w = -1;
	ret_w = m2mb_spi_write(fd, buf, nbyte);
	if (ret_w != -1 && (SIZE_T) ret_w == nbyte)
	{
		return AZX_SPI_FLASH_CODE_PASS;
	}
	return AZX_SPI_FLASH_CODE_FAIL;
}
/***************** spiWrite ***********************************************/


/*******************************************************************************
                  getTicks

Function:       JSC_uint32 getTicks(void)
Arguments:
Description:    get system current tick(unit: 10ms) function ;

*******************************************************************************/
JSC_uint32 azx_spi_flash_get_ticks(void)
{
	INT32 fd;
	M2MB_RTC_TIMEVAL_T timeval;

	fd = m2mb_rtc_open("/dev/rtc0", 0);
	m2mb_rtc_ioctl(fd, M2MB_RTC_IOCTL_GET_TIMEVAL, &timeval);
	m2mb_rtc_close(fd);

    return timeval.sec;
}
/***************** getTicks ***********************************************/


/*******************************************************************************
                  get_uptime

Function:       JSC_uint32 get_uptime(void)
Arguments:
Description:    get system current tick(unit: 10ms) function ;

*******************************************************************************/
JSC_uint32 azx_spi_flash_get_uptime(void)
{

  UINT32 sysTicks = m2mb_os_getSysTicks();

  FLOAT32 ms_per_tick = m2mb_os_getSysTickDuration_ms();

  return (JSC_uint32) (sysTicks * ms_per_tick); //milliseconds
}
/***************** get_uptime ***********************************************/


/*******************************************************************************
                  openGpioOutput

Function:
Arguments:
Description:

*******************************************************************************/
INT32 openGpioOutput(int pin)
{
	INT32 ret;
	INT32 g_fd;
	char path[16];

	memset(path, 0, sizeof(path));
	sprintf(path, "/dev/GPIO%d", pin);

	g_fd = m2mb_gpio_open( path, 0 );
	if( g_fd != -1 )
	{
	    /* SET GPIO as output */
	    ret = m2mb_gpio_ioctl( g_fd, M2MB_GPIO_IOCTL_SET_DIR, M2MB_GPIO_MODE_OUTPUT );
	    if ( ret == -1 )
	    {
	      return -1;
	    }

	    ret = m2mb_gpio_ioctl( g_fd, M2MB_GPIO_IOCTL_SET_PULL, M2MB_GPIO_PULL_UP );
	    if ( ret == -1 )
	    {
	      return -1;
	    }

	    ret = m2mb_gpio_ioctl( g_fd, M2MB_GPIO_IOCTL_SET_DRIVE, M2MB_GPIO_MEDIUM_DRIVE );
	    if ( ret == -1 )
	    {
	      return -1;
	    }

	    return g_fd;
	}
	else
	{
		AZX_LOG_ERROR("Cannot open GPIO descriptors!\r\n");
		return -1;
	}
}
/***************** openGpioOutput ***********************************************/



/* Global functions definition ===================================================================*/

/*
* MUST IMPLEMENT -- SPI Transfer function
* @slave: spi device structure
* @bitlen: transfer length in bit
* @dout: buffer for TX
* @din: buffer for RX
* @flags: flags for SPI transfer
*     SPI_XFER_BEGIN - Assert CS before transfer
*     SPI_XFER_END   - Deassert CS after transfer
*     SPI_XFER_DUAL  - 2 pin data transfer
*     SPI_XFER_QUAD  - 4 pin data transfer
*     if not SPI_XFER_DUAL and not SPI_XFER_QUAD - 1 pin data transfer
*/

AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_spi_xfer(AZX_SPI_FLASH_SPI_SLAVE *slave, JSC_uint32 bitlen,
		void *dout, void *din, JSC_uint64 flags)
{
	INT32 ret = AZX_SPI_FLASH_CODE_FAIL;
	UINT32 nbyte = bitlen/8;
	AZX_SPI_FLASH_DESCRIPTOR *fd = (AZX_SPI_FLASH_DESCRIPTOR*) slave->hspi;


	if( (flags & SPI_XFER_BEGIN) == SPI_XFER_BEGIN)
	{
		setGPIO(fd->gpio, LOW_VALUE);
	}

	if (dout == NULL && din != NULL)
	{
		ret = spiRead(fd->spi, din, nbyte);
		if (ret != AZX_SPI_FLASH_CODE_PASS)
		{
			AZX_SPI_FLASH_LOG_ERROR("micro->fd.spi READ FAILED\r\n");
			return AZX_SPI_FLASH_CODE_SPI_READ_FAIL;
		}
	}
	if (dout != NULL && din == NULL)
	{
		ret = spiWrite(fd->spi, dout, nbyte);
		if (ret != AZX_SPI_FLASH_CODE_PASS)
		{
			AZX_SPI_FLASH_LOG_ERROR("micro->fd.spi WRITE FAILED\r\n");
			return AZX_SPI_FLASH_CODE_SPI_WRITE_FAIL;
		}
	}

////	(dout == NULL && din == NULL) -> FAIL
////	(dout != NULL && din != NULL) -> READ + WRITE

	if( (flags & SPI_XFER_END) == SPI_XFER_END)
	{
		setGPIO(fd->gpio, HIGH_VALUE);
	}

//	azx_sleep_ms(200);
    return AZX_SPI_FLASH_CODE_PASS;
}
/***************** spi_xfer ***********************************************/


/*******************************************************************************
                  NAND_SPI_initialization

Function:
Arguments:
Description:

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_spi_initialization(JSC_int32 *spi_fd)
{
	//user variables
	M2MB_SPI_CFG_T cfg;
	INT32 s_fd;
	INT32 ret = -1;

//	UINT8 userdata[3];
//	userdata[0] = 0x0F;

	//Open the SPI device
//	JSC_LOG_DEBUG("Opening the SPI device...\r\n");
	s_fd = m2mb_spi_open("/dev/spidev", 0); //dev/spidev5.0 but dev/spidev is default auto-converted path

	if (s_fd != -1)
	{
		memset(&cfg, 0, sizeof(cfg));

		cfg.spi_mode = M2MB_SPI_MODE_0; //clock idle LOW, data driven on falling edge and sampled on rising edge
		cfg.cs_polarity = M2MB_SPI_CS_ACTIVE_LOW;
		cfg.cs_mode = M2MB_SPI_CS_KEEP_ASSERTED; //M2MB_SPI_CS_DEASSERT;
		cfg.endianness = M2MB_SPI_NATIVE; //M2MB_SPI_LITTLE_ENDIAN; //M2MB_SPI_BIG_ENDIAN;
		cfg.callback_fn = NULL;
		cfg.callback_ctxt = NULL; //(void *)userdata; //NULL;;
		cfg.clk_freq_Hz = 1000000;
		cfg.bits_per_word = 8;
		cfg.cs_clk_delay_cycles = 3;
		cfg.inter_word_delay_cycles = 0;
		cfg.loopback_mode = FALSE;

		ret = m2mb_spi_ioctl(s_fd, M2MB_SPI_IOCTL_SET_CFG, (void *)&cfg);
		if(ret == 0)
		{
			*spi_fd = s_fd;
			return AZX_SPI_FLASH_CODE_PASS;
		}
		else
		{
			AZX_LOG_CRITICAL("Cannot configure a SPI device!\r\n");
			return AZX_SPI_FLASH_CODE_SPI_INIT_FAIL;
		}
	}
	else
	{
		AZX_LOG_CRITICAL("Cannot open SPI channel!\r\n");
		return AZX_SPI_FLASH_CODE_SPI_INIT_FAIL;
	}
}
/***************** NAND_SPI_initialization ***********************************************/


/*******************************************************************************
                  NAND_SPI_close

Function:
Arguments:
Description:

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_spi_close(JSC_int32 *spi_fd)
{
	INT32 ret = -1;
    ret = m2mb_spi_close(*spi_fd);
    if(ret != 0)
    {
    	AZX_LOG_ERROR( "m2mb_spi_close FAIL   ret = %d \r\n", ret );
    	return AZX_SPI_FLASH_CODE_FAIL;
    }

    *spi_fd=0;
    return AZX_SPI_FLASH_CODE_PASS;
}
/***************** azx_nand_spi_close ***********************************************/


/*******************************************************************************
                  NAND_GPIO_initialization

Function:
Arguments:
Description:

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_gpio_initialization(const JSC_uint8 CS_GPIO_pin, JSC_int32 *gpio_fd)
{
	INT32 g_fd;
	INT32 ret;

	g_fd = openGpioOutput(CS_GPIO_pin);
	if (g_fd < 0)
	{
		AZX_LOG_CRITICAL("Cannot set GPIO descriptors!\r\n");
		return AZX_SPI_FLASH_CODE_GPIO_INIT_FAIL;
	}

	ret = m2mb_gpio_write( g_fd, (M2MB_GPIO_VALUE_E) HIGH_VALUE );
	if (ret != 0)
	{
		AZX_LOG_CRITICAL("Cannot write GPIO!\r\n");
		return AZX_SPI_FLASH_CODE_GPIO_INIT_FAIL;
	}

	*gpio_fd = g_fd;
	return AZX_SPI_FLASH_CODE_PASS;
}
/***************** NAND_GPIO_initialization ***********************************************/


/*******************************************************************************
                  azx_nand_gpio_close

Function:
Arguments:
Description:

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_gpio_close(JSC_int32 *gpio_fd)
{
	INT32 ret = -1;
    ret = m2mb_gpio_close(*gpio_fd);
    if(ret != 0)
    {
    	AZX_LOG_ERROR( "m2mb_gpio_close FAIL   ret = %d \r\n", ret );
    	return AZX_SPI_FLASH_CODE_FAIL;
    }

    *gpio_fd = 0;
    return AZX_SPI_FLASH_CODE_PASS;
}
/***************** azx_nand_gpio_close ***********************************************/


/*******************************************************************************
                  NAND_free

Function:       void* NAND_free()
Arguments:
Description:    M2MB_OS_RESULT_E m2mb_os_free( void *pMem );

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_free(void *buf)
{
	M2MB_OS_RESULT_E res;
	res =  m2mb_os_free(buf);
	if (res == M2MB_OS_SUCCESS)
	{
		return AZX_SPI_FLASH_CODE_PASS;
	}
	else
	{
		return AZX_SPI_FLASH_CODE_FAIL;
	}
}
/***************** NAND_free ***********************************************/


/*******************************************************************************
                  NAND_malloc

Function:       void* NAND_malloc()
Arguments:
Description:    void *m2mb_os_malloc( UINT32 size );

*******************************************************************************/
void *azx_spi_flash_malloc(JSC_size_t size)
{
	//void *p = m2mb_os_malloc(size);
	//memset(p, 0xFF, size);
	//return p;
	return m2mb_os_malloc(size);
}
/***************** NAND_malloc ***********************************************/


/*******************************************************************************
                  NAND_memcpy

Function:       void* NAND_memcpy(void* dst, const void* src, size_t size)
Arguments:
Description:    memcpy(void* dst, const void* src, size_t size);

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_memcpy(void* dst, const void* src, JSC_size_t size)
{
	memcpy(dst, src, size);
    return AZX_SPI_FLASH_CODE_PASS;
}
/***************** NAND_memcpy ***********************************************/


/*******************************************************************************
                  NAND_memset

Function:       void* NAND_memset(void* buf, int val, size_t size)
Arguments:
Description:    memset(void* buf, int val, size_t size);

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_memset(void* buf, JSC_int32 val, JSC_size_t size)
{
	memset(buf, val, size);
    return AZX_SPI_FLASH_CODE_PASS;
}
/***************** NAND_memset ***********************************************/


/*******************************************************************************
                  NAND_memcmp

Function:       NAND_memcmp(const void* src1, const void* src2, size_t size)
Arguments:		NAND_PASS if src1 == src2
Description:    memcmp(const void* src1, const void* src2, size_t size);

*******************************************************************************/
JSC_int32 azx_spi_flash_memcmp(const void* src1, const void* src2, JSC_size_t size)
{
    return memcmp(src1, src2, sizeof(size));
}
/***************** NAND_memcmp ***********************************************/


/*******************************************************************************
                  NAND_delay

Function:       void NAND_delay(JSC_uint8 delay)
Arguments:		delay: numener of milliseconds
Description:    This function provides minimum delay (in milliseconds) based
 	 	 	 	on variable incremented;

*******************************************************************************/
void azx_spi_flash_delay(JSC_int32 delay)
{
	m2mb_os_taskSleep( M2MB_OS_MS2TICKS(delay) );
}
/***************** NAND_delay ***********************************************/



