/*
 * ram_utils_usage.c
 *
 *  Created on: Oct 13, 2020
 *      Author: NormanAr
 */

/* Include files ================================================================================*/
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "azx_utils.h"
#include "azx_log.h"

#include "azx_spi_flash_jsc.h"

#include "test_main.h"


/* Local variables definition =============================================================================*/
static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
static AZX_SPI_FLASH_JSC_HANDLE  		clbkNandJscHndl;
static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;

static UINT8 read_buf[4096];

/* Local function prototypes ====================================================================*/
static void nandLFSCallback (void *h, AZX_SPI_FLASH_JSC_EVENT_E event, UINT16 resp_size, void *resp_struct, void *userdata);

static TEST_RESULT_E nandReadMemoryInfo (void);
static TEST_RESULT_E nandInitialize		(void);
static TEST_RESULT_E nandReadUID		(void);
static TEST_RESULT_E nandParameter		(void);




/*******************************************************************************
                  nandLFSCallback
Function:      TODO: da finire
Arguments:
Return Value:
Description:
*******************************************************************************/
static void nandLFSCallback (void *h, AZX_SPI_FLASH_JSC_EVENT_E event, UINT16 resp_size, void *resp_struct, void *userdata )
{
	AZX_LOG_INFO("nandLFSCallback Callback event <%d>\r\n", (int) event);
	(void) h, (void) resp_size, (void) resp_struct, (void) userdata;

	switch( event )
	{
	case AZX_SPI_FLASH_INITIALIZED:
		{
			AZX_LOG_INFO("NAND Callback event: NAND_JSC_INITIALIZED <%d>\r\n", event);
			break;
		}
	case AZX_SPI_FLASH_SPI_INITIALIZED:
		{
			break;
		}
	case AZX_SPI_FLASH_GPIO_INITIALIZED:
		{
			break;
		}
	case AZX_SPI_FLASH_RESETTED:
		{
			break;
		}
	default:
		break;
	}
}
/***************** nandLFSCallback ***********************************************/


static TEST_RESULT_E nandReadMemoryInfo(void)
{
	AZX_LOG_INFO("Reading ID...\r\n");
	UINT8 id[3] = {0};

	if (g_chip->opts->init != 1)
	{
		AZX_LOG_ERROR("Nand not initialized!!\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("NAND memory correclty initialized. Retrieving informations...\r\n");

	if (azx_spi_flash_readID(g_chip, id) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand read ID failed!!\r\n");
		return TEST_ERROR;
	}

	AZX_LOG_INFO("\r\n");
	AZX_LOG_INFO("SPI-NAND type mfr_id: %x, dev_id: %x, dev_id2: %x is not in id table.\r\n",
        id[0], id[1], id[2]);
	AZX_LOG_INFO("\r\n");

	AZX_LOG_INFO("Part number            : %s\r\n", g_chip->name);
	AZX_LOG_INFO("VCC                    : %s\r\n", g_chip->vcc_33 ? "3.3V" : "1.8V");
	AZX_LOG_INFO("MID                    : 0x%02X\r\n", g_chip->mfr_id);
	AZX_LOG_INFO("DID                    : 0x%02X\r\n", g_chip->dev_id);
	AZX_LOG_INFO("DID2                   : 0x%02X\r\n", g_chip->dev_id2);
	AZX_LOG_INFO("Blocks per LUN         : %lld\r\n", g_chip->size / g_chip->block_size);
	AZX_LOG_INFO("Max bad blocks per LUN : %d\r\n", g_chip->max_bad_blks_per_lun);
	AZX_LOG_INFO("Pages per block        : %d\r\n", g_chip->block_size / g_chip->page_size);
	AZX_LOG_INFO("Device size            : %lld MB - %lld B\r\n", g_chip->size >> 20, g_chip->size);
	AZX_LOG_INFO("Block size             : %d KB - %d B\r\n", g_chip->block_size >> 10, g_chip->block_size);
	AZX_LOG_INFO("Page size              : %d KB - %d B\r\n", g_chip->page_size >> 10, g_chip->page_size);
	AZX_LOG_INFO("OOB size               : %d B\r\n", g_chip->oob_size);
	AZX_LOG_INFO("ECC size               : %d B\r\n", g_chip->ecclayout->eccbytes);
	AZX_LOG_INFO("OOB available size     : %d B\r\n", g_chip->ecclayout->oobavail);
	AZX_LOG_INFO("ECC strength           : %d\r\n", g_chip->ecc_strength);
	AZX_LOG_INFO("Refresh strength       : %d\r\n", g_chip->refresh_threshold);
	AZX_LOG_INFO("OTP block              : %d\r\n", g_chip->otp_block);
	AZX_LOG_INFO("OTP page start         : %d\r\n", g_chip->otp_page_start);
	AZX_LOG_INFO("OTP page number        : %d\r\n", g_chip->otp_page_number);

	AZX_LOG_INFO("==================================================\r\n");
	return TEST_OK;
}

static TEST_RESULT_E nandReadUID(void)
{
	AZX_SPI_FLASH_CODE_RESULT_E ret;
    JSC_uint8 xorJSC;

    memset(read_buf, 0xFF, 4096);

    /* Read UID */
	AZX_LOG_INFO("Read UID...\r\n");
    ret = azx_spi_flash_readUIDpage(g_chip, 0, read_buf, g_chip->page_size);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	AZX_LOG_ERROR("UID page : read operation ERROR, test finished\r\n");
		return TEST_ERROR;
    }

    AZX_LOG_INFO(" unique ID            : 0x");
    for (JSC_uint32 byte = 0; byte < 16; byte++)
    {
    	AZX_LOG_INFO("%02X", read_buf[byte]);
    }
    AZX_LOG_INFO("\r\n");

    AZX_LOG_INFO(" unique ID complement : 0x");
    for (JSC_uint32 byte = 0; byte < 16; byte++)
    {
    	AZX_LOG_INFO("%02X", read_buf[byte + 16]);
    }
    AZX_LOG_INFO("\r\n");

    AZX_LOG_INFO(" unique ID XOR        : 0x");
    for (JSC_uint32 byte = 0; byte < 16; byte++)
    {
    	xorJSC = read_buf[byte] ^ read_buf[byte + 16];
    	AZX_LOG_INFO("%02X", xorJSC);

        if (xorJSC != 0xFF)
        {
        	ret = AZX_SPI_FLASH_CODE_FAIL;
        }
    }
    AZX_LOG_INFO("\r\n");

    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	AZX_LOG_ERROR(" unique ID invalid\r\n");
    }
    else
    {
    	AZX_LOG_INFO(" unique ID valid\r\n");
    }


	AZX_LOG_INFO("==================================================\r\n");
	return TEST_OK;
}

static TEST_RESULT_E nandParameter(void)
{
	AZX_LOG_INFO("Read parameters...\r\n");

	AZX_SPI_FLASH_CODE_RESULT_E ret;
	AZX_SPI_FLASH_ONFI_PARAMS *p;

	memset(read_buf, 0x00, g_chip->page_size);
	ret = azx_spi_flash_readParameterPage(g_chip, 0, read_buf, g_chip->page_size);
  	if (ret != AZX_SPI_FLASH_CODE_PASS)
  	{
  		AZX_LOG_ERROR("Parameter page : read operation ERROR, test finished\r\n");
		return TEST_ERROR;
  	}

  	p = (AZX_SPI_FLASH_ONFI_PARAMS *) &read_buf[0];
  	if (azx_spi_flash_checkParameter(p) != AZX_SPI_FLASH_CODE_PASS)
  	{
  		AZX_LOG_ERROR("parameter page : data invalid, test finished\r\n");
		return TEST_ERROR;
  	}

  	AZX_LOG_INFO("Test read parameter PASSED\r\n");


	AZX_LOG_INFO("==================================================\r\n");
	return TEST_OK;
}


static TEST_RESULT_E nandInitialize( void )
{
	AZX_LOG_INFO("Starting initialization...\r\n");

    opts.clbk.clbkHndl = &clbkNandJscHndl;
    opts.clbk.clbkFunc = nandLFSCallback;

    /* Ability of SPI Controllers : choose one of x1/x2/x4 */
	opts.spi.op_mode_rx = SPI_OPM_RX_SING;
    opts.spi.op_mode_tx = SPI_OPM_TX_SING;

	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}

	AZX_LOG_INFO("Initialized...\r\n");
	return TEST_OK;
}

TEST_RESULT_E runNand_TestGroup (void)
{
	// This gets run before every test
	AZX_LOG_INFO("Starting initialization...\r\n");

	//initialize nand memory
	if (nandInitialize() != TEST_OK) //if error
	{
		AZX_LOG_ERROR("setup failed!!\r\n");
		return TEST_ERROR;
	}

	//Nand Info
	if (nandReadMemoryInfo() != TEST_OK) //if error
	{
		AZX_LOG_ERROR("Read Memory Info failed!!\r\n");
		return TEST_ERROR;
	}

	//Nand UID
	if (nandReadUID() != TEST_OK) //if error
	{
		AZX_LOG_ERROR("Read UID failed!!\r\n");
		return TEST_ERROR;
	}

	//Nand Parameter
	if (nandParameter() != TEST_OK) //if error
	{
		AZX_LOG_ERROR("Parameter failed!!\r\n");
		return TEST_ERROR;
	}


	return TEST_OK;
}
