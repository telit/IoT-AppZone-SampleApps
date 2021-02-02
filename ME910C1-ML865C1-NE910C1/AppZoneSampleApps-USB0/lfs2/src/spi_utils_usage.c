/*
 * azx_lfs_usage.c
 *
 *  Created on: Sep 10, 2020
 *      Author: NormanAr
 */


/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
  	  row_lfs_usage.h

  @brief
  	  Project: SPI data flash

  @details

  @version
 	 1.0.0

  @note

  @author
  	  Norman Argiolas

  @note
  	  File created on: Jul 30, 2020
*/

/* Include files ================================================================================*/
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "m2mb_os_api.h"

#include "lfs_usage.h"

#include "lfs2.h"
#include "azx_utils.h"
#include "azx_log.h"

#include "azx_lfs_utils.h"

#include "azx_spi_flash_jsc.h"



/* Local variables definition =============================================================================*/

static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
static AZX_SPI_FLASH_JSC_HANDLE  		clbkNandJscHndl;
static AZX_SPI_FLASH_SPI_CHIP*			g_chip;

// variables used by the filesystem
static lfs2_t lfs2;

static void *read_buffer;
static void *prog_buffer;
static void *lookahead_buffer;
static lfs2_size_t lookahead_size = 16;// 2048;	//16;
static int32_t block_cycles 		 = 500; //1000;

/* Local function prototypes ====================================================================*/
static TEST_RESULT_E testAllInOneFunction (void);

//static TEST_RESULT_E directoryList 		(void);


/*------------------------------*/
static TEST_RESULT_E fileListUtils 		(void);



static void nandLFSCallback 			(void *h, AZX_SPI_FLASH_JSC_EVENT_E event, UINT16 resp_size, void *resp_struct, void *userdata );

static int user_provided_block_device_read  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size);
static int user_provided_block_device_prog  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size);
static int user_provided_block_device_erase (const struct lfs2_config *c, lfs2_block_t block);
static int user_provided_block_device_sync  (const struct lfs2_config *c);


static AZX_LFS_CONTEXT_S g_context;
static struct lfs2_config lfsContext;

/* Local functions definition =============================================================================*/

// Read a region in a block. Negative error codes are propogated
// to the user.
static int user_provided_block_device_read (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size)
{
	AZX_SPI_FLASH_SPI_CHIP * chip = (AZX_SPI_FLASH_SPI_CHIP *) c->context;
	//chip->page_size must be size; I speculate this, to better verify

	AZX_SPI_FLASH_CODE_RESULT_E ret;
	UINT8 read_buf_oob[256];
	JSC_loff_t block_addr;
	block_addr = block << g_chip->block_shift;

	AZX_SPI_FLASH_MTD_OOB_OPS ops;
	memset(&ops, 0xFF, sizeof(ops));
	ops.mode		= MTD_OPS_AUTO_OOB;
	ops.len  		= size;
	ops.ooblen      = g_chip->ecclayout->oobavail;
	ops.ooboffs     = 0;
	ops.datbuf      = (JSC_uint8 *) buffer;
	ops.oobbuf      = read_buf_oob;

    //ret = NAND_readOob(chip, off, &ops);
    ret = azx_spi_flash_readOob(chip, block_addr + off, &ops);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	//an error occur
    	return LFS2_ERR_IO;
    }
    else
    {
    	//no error occur
    	return LFS2_ERR_OK;
    }
};

static int user_provided_block_device_prog (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size)
{
	AZX_SPI_FLASH_SPI_CHIP * chip = (AZX_SPI_FLASH_SPI_CHIP *) c->context;

	AZX_SPI_FLASH_CODE_RESULT_E ret;
	UINT8 write_buf_oob[256];
	JSC_loff_t block_addr;
	block_addr = block << chip->block_shift;

	AZX_SPI_FLASH_MTD_OOB_OPS ops;
	memset(&ops, 0xFF, sizeof(ops));
    ops.mode        = MTD_OPS_AUTO_OOB;
    ops.len         = size;
    ops.ooblen      = chip->ecclayout->oobavail;
    ops.ooboffs     = 0;
    ops.datbuf      = (JSC_uint8 *) buffer;
    ops.oobbuf      = write_buf_oob;

//    JSC_uint32 page_addr = off << g_chip->page_shift;

    ret = azx_spi_flash_writeOob(chip, block_addr + off, &ops);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	//an error occur
    	return LFS2_ERR_IO;
    }
    else
    {
    	//no error occur
    	return LFS2_ERR_OK;
    }


};

static int user_provided_block_device_erase (const struct lfs2_config *c, lfs2_block_t block)
{
	AZX_SPI_FLASH_SPI_CHIP * chip = (AZX_SPI_FLASH_SPI_CHIP *) c->context;
	AZX_SPI_FLASH_CODE_RESULT_E ret;
	JSC_loff_t block_addr;

	//retrieve the address of the block
	block_addr = block << g_chip->block_shift;
    ret = azx_spi_flash_erase(chip, block_addr, chip->block_size);

    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	//an error occur
    	return LFS2_ERR_IO;
    }
    else
    {
    	//no error occur
    	return LFS2_ERR_OK;
    }
};

static int user_provided_block_device_sync (const struct lfs2_config *c)
{
	AZX_SPI_FLASH_SPI_CHIP * chip = (AZX_SPI_FLASH_SPI_CHIP *) c->context;
	AZX_SPI_FLASH_CODE_RESULT_E ret;
	JSC_uint8 status;

	ret = azx_spi_flash_waitForReady(chip, &status);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	//an error occur
    	return LFS2_ERR_IO;
    }
    else
    {
    	//no error occur
    	return LFS2_ERR_OK;
    }

};

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


static TEST_RESULT_E fileListUtils (void)
{

	AZX_LOG_INFO("\r\r\n\n<><><><>fileListUtils\r\n");

	AZX_LFS_INFO_FILE_E *listContent = NULL;

	char root[] = "/";
	if (azx_lfsListsContentByContext(root,
									 &listContent,
									 &lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfs_flashListsContentByContext failed!!\r\n");
		return TEST_ERROR;
	}

	if (listContent != NULL)
	{
		AZX_LFS_INFO_FILE_E *current = listContent;
		AZX_LFS_INFO_FILE_E *prev = NULL;

		AZX_LOG_INFO("\r\nList:\r\n");
		do
		{
			AZX_LOG_INFO("%s, %d, %d\r\n", current->info.name, current->info.size, current->info.type);
			prev = current;
			current = current->next;
			prev->next = NULL;
			M2MB_OS_RESULT_E res;
			res =  m2mb_os_free((AZX_LFS_INFO_FILE_E*) prev);
			if (res != M2MB_OS_SUCCESS)
			{
				AZX_LOG_ERROR("azx_lfs_flashListsContentByContext failed!!\r\n");
				return TEST_ERROR;
			}
		}
		while(current != NULL);
	}
	AZX_LOG_INFO("\r\n");
	return TEST_OK;
}

static TEST_RESULT_E testAllInOneFunction ( void )
{

	// This gets run before every test
	AZX_LOG_INFO("Starting initialization...\r\n");

	opts.clbk.clbkHndl = &clbkNandJscHndl;
	opts.clbk.clbkFunc = nandLFSCallback;

	/* Ability of SPI Controllers : choose one of x1/x2/x4 */
	opts.spi.op_mode_rx = SPI_OPM_RX_SING;
	opts.spi.op_mode_tx = SPI_OPM_TX_SING;

	//g_chip initialization with memory allocation
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}

	//test callback
	g_chip->opts->clbk.clbkFunc(NULL,
							    (AZX_SPI_FLASH_JSC_EVENT_E) AZX_SPI_FLASH_INITIALIZED,
							    sizeof(AZX_SPI_FLASH_JSC_EVENT_RES_T),
							    NULL, NULL);



	//initialization struct lfs2_config lfsContext;
	lfsContext.context = g_chip;
	lfsContext.read = user_provided_block_device_read;
	lfsContext.prog = user_provided_block_device_prog;
	lfsContext.erase = user_provided_block_device_erase;
	lfsContext.sync = user_provided_block_device_sync;

	lfsContext.read_size = g_chip->page_size;						//read_size;
	lfsContext.prog_size = g_chip->page_size; 						//prog_size;
	lfsContext.block_size = g_chip->block_size; 					// block_size;
	lfsContext.block_count = g_chip->size / g_chip->block_size;		//block_count;
	lfsContext.block_cycles = block_cycles;
	lfsContext.cache_size = g_chip->page_size;  					//cache_size;

	lfsContext.lookahead_size = lookahead_size;
	lfsContext.read_buffer = read_buffer;
	lfsContext.prog_buffer = prog_buffer;
	lfsContext.lookahead_buffer = lookahead_buffer;

	lfsContext.name_max = LFS2_NAME_MAX;
	lfsContext.file_max = LFS2_FILE_MAX;
	lfsContext.attr_max = LFS2_ATTR_MAX;

	//set file system context
	g_context.context = &lfsContext;
	g_context.type = AZX_LFSCONTEXT_FLASH;


	int err;
	LFS2_ERROR_T ret;


	//print information
	lfs2_log_info(&lfs2, &lfsContext);

	AZX_LOG_INFO("\r\nMounting partition...\r\n");
	// mount the filesystem
	ret = azx_lfsMountByContext(AZX_LFSMOUNT_MODE_FORMAT_AND_MOUNT,
								&g_context,
								&lfs2);
	if (ret != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("\r\nError lfsMountByContext memory: err = %d \r\n", ret);
		return TEST_ERROR;
	}

	if (fileListUtils() != TEST_OK)
	{
		AZX_LOG_ERROR("fileListUtils failed!!\r\n");
		return TEST_ERROR;
	}

	ret = azx_lfsMountByContext(AZX_LFSMOUNT_MODE_FORMAT_AND_MOUNT,
								&g_context,
								&lfs2);
	if (ret != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("\r\nError lfsMountByContext memory: err = %d \r\n", ret);
		return TEST_ERROR;
	}

	if (fileListUtils() != TEST_OK)
	{
		AZX_LOG_ERROR("fileListUtils failed!!\r\n");
		return TEST_ERROR;
	}

	char content [10] = {0};

	memset(content, 0x00, sizeof(content));
	strcat(content, "content000");

	char file000[] = "file000.txt";
	if (azx_lfsWriteByContext(file000,
							  strlen(content),
							  (UINT8*) content,
							  AZX_LFSWRITE_MODE_CREAT,
							  &lfs2,
							  &g_context) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("fileCreationUtils failed!!\r\n");
		return TEST_ERROR;
	}
	memset(content, 0x00, sizeof(content));
	strcat(content, "content002");

	char file002[] = "file002.txt";
	if (azx_lfsWriteByContext(file002,
									strlen(content),
									(UINT8*) content,
									AZX_LFSWRITE_MODE_CREAT,
									&lfs2,
									&g_context) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("fileCreationUtils failed!!\r\n");
		return TEST_ERROR;
	}

	memset(content, 0x00, sizeof(content));
	strcat(content, "content001");

	char file001[] = "file001.txt";
	if (azx_lfsWriteByContext(file001,
							  strlen(content),
							  (UINT8*) content,
							  AZX_LFSWRITE_MODE_CREAT,
							  &lfs2,
							  &g_context) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("fileCreationUtils failed!!\r\n");
		return TEST_ERROR;
	}

	if (fileListUtils() != TEST_OK)
	{
		AZX_LOG_ERROR("fileListUtils failed!!\r\n");
		return TEST_ERROR;
	}

	memset(content, 0x00, sizeof(content));
	strcat(content, "content003");

	char file003[] = "file003.txt";
	if (azx_lfsWriteByContext(file003,
									strlen(content),
									(UINT8*) content,
									AZX_LFSWRITE_MODE_CREAT,
									&lfs2,
									&g_context
									) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("fileCreationUtils failed!!\r\n");
		return TEST_ERROR;
	}
	memset(content, 0x00, sizeof(content));
	strcat(content, "content004");

	char file004[] = "file004.txt";
	if (azx_lfsWriteByContext(file004,
							  strlen(content),
							  (UINT8*) content,
							  AZX_LFSWRITE_MODE_CREAT,
							  &lfs2,
							  &g_context) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfsWriteByContext failed!!\r\n");
		return TEST_ERROR;
	}

	if (fileListUtils() != TEST_OK)
	{
		AZX_LOG_ERROR("fileListUtils failed!!\r\n");
		return TEST_ERROR;
	}


	AZX_LOG_INFO("------>File reading\r\n");
	char buffer[11];
	unsigned int size = 10;

	memset(buffer, 0x00, sizeof(buffer));
	if (azx_lfsReadByContext(file000,
			  	  	  	  	buffer,
							size,
							&lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfsReadByContext failed!!\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\r\nFile: %s, Size: %d, Buffer: %s\r\n\n", "file000.txt", size, buffer);


	memset(buffer, 0x00, sizeof(buffer));
	if (azx_lfsReadByContext(file004,
			  	  	  	  	buffer,
							size,
							&lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfsReadByContext failed!!\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\r\nFile: %s, Size: %d, Buffer: %s\r\n\n", "file004.txt", size, buffer);


	memset(buffer, 0x00, sizeof(buffer));
	if (azx_lfsReadByContext("file002.txt",
			  	  	  	  	buffer,
							size,
							&lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfsReadByContext failed!!\r\n\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\r\nFile: %s, Size: %d, Buffer: %s\r\n", "file002.txt", size, buffer);


	AZX_LOG_INFO("------>File removing\r\n");
	if (azx_lfsRmFileByContext(&lfs2, "file001.txt") != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfs_flashRmFileByContext failed!!\r\n");
		return TEST_ERROR;
	}
	if (azx_lfsRmFileByContext(&lfs2, "file000.txt") != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfs_flashRmFileByContext failed!!\r\n");
		return TEST_ERROR;
	}

	if (azx_lfsRmFileByContext(&lfs2, "file004.txt") != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfs_flashRmFileByContext failed!!\r\n");
		return TEST_ERROR;
	}

	if (fileListUtils() != TEST_OK)
	{
		AZX_LOG_ERROR("fileListUtils failed!!\r\n");
		return TEST_ERROR;
	}

	char dir000[] = "dir000";
	if (azx_lfsDirCreationByContext(dir000,
									AZX_LFSDIR_MODE_CREAT,
									&lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("directoryCreation failed!!\r\n");
		return TEST_ERROR;
	}


	if (azx_lfsDirCreationByContext(dir000,
									AZX_LFSDIR_MODE_APPEND,
									&lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("directoryCreation failed!!\r\n");
		return TEST_ERROR;
	}

	char dir001[] = "dir001";
	if (azx_lfsDirCreationByContext(dir001,
									AZX_LFSDIR_MODE_CREAT,
									&lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("directoryCreation failed!!\r\n");
		return TEST_ERROR;
	}

	char dir002[] = "dir002";
	if (azx_lfsDirCreationByContext(dir002,
									AZX_LFSDIR_MODE_CREAT,
									&lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("directoryCreation failed!!\r\n");
		return TEST_ERROR;
	}

	if (fileListUtils() != TEST_OK)
	{
		AZX_LOG_ERROR("fileListUtils failed!!\r\n");
		return TEST_ERROR;
	}

	if (azx_lfsDirRmByContext(dir000,
							  AZX_LFSRMDIR_MODE_EMPTY,
							  &lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("directoryCreation failed!!\r\n");
		return TEST_ERROR;
	}

	if (fileListUtils() != TEST_OK)
	{
		AZX_LOG_ERROR("fileListUtils failed!!\r\n");
		return TEST_ERROR;
	}

	if (azx_spi_flash_releaseResources(g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Release memory failed!!\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("Nand released\r\n");

	err = lfs2_unmount(&lfs2);
	if (err)
	{
		AZX_LOG_ERROR("\r\nError unmounting memory: err = %d \r\n", err);
		return TEST_ERROR;
	}
	AZX_LOG_INFO("Partition unmounted\r\n");

	AZX_LOG_INFO("\r\nUnmounted process ended...\r\n");


	AZX_LOG_INFO("\r\ntestAllInOneFunction ended...\r\n");
	return TEST_OK;
}

TEST_RESULT_E flashDiskDemo(void)
{
	AZX_LOG_INFO("\r\r\n\n>>>>>>> Starting FlashDiskDemo ...\r\n");

	//all in one function
	if (testAllInOneFunction() != TEST_OK) //if error
	{
		AZX_LOG_ERROR("setup failed!!\r\n");
		return TEST_ERROR;
	}

	return TEST_OK;
}


