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

#include "test_main.h"

#include "lfs2.h"
//#include "azx_lfs2_utils.h"
#include "azx_utils.h"
#include "azx_log.h"

#include "azx_spi_flash_jsc.h"



/* Local variables definition ===================================================================*/
/* Local variables definition =============================================================================*/

static AZX_SPI_FLASH_MICRO_SPI_PARAMS opts;
static AZX_SPI_FLASH_JSC_HANDLE  clbkNandJscHndl;
static AZX_SPI_FLASH_SPI_CHIP *g_chip;

// variables used by the filesystem
static struct lfs2_config* lfs2_cfg = NULL;
static lfs2_t lfs2;
//static lfs2_file_t file;

static void *context;
static void *read_buffer;
static void *prog_buffer;
static void *lookahead_buffer;
static lfs2_size_t lookahead_size = 16;// 2048;	//16;
static int32_t block_cycles = 500; //1000;
static lfs2_size_t block_size = 131072; //131072;
static lfs2_size_t cache_size = 2048; //2048;		//131072/32=4096
static lfs2_size_t prog_size = 2048; //2048;		//16;
static lfs2_size_t block_count = 6; //1024;		//128;
static lfs2_size_t read_size = 2048;// 2048;		//16;


/* Local function prototypes ====================================================================*/
static TEST_RESULT_E directoryList (void);
static TEST_RESULT_E directoryCreation	(const char *name);
static TEST_RESULT_E directoryRemove	(const char *name);

static TEST_RESULT_E fileList (void);
static TEST_RESULT_E fileCreation	(const char *name, const char *content);
static TEST_RESULT_E fileRemove	(const char *name);
static TEST_RESULT_E fileRead	(const char *name);

static void nandLFSCallback (void *h, AZX_SPI_FLASH_JSC_EVENT_E event, UINT16 resp_size, void *resp_struct, void *userdata );

static int user_provided_block_device_read  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size);
static int user_provided_block_device_prog  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size);
static int user_provided_block_device_erase (const struct lfs2_config *c, lfs2_block_t block);
static int user_provided_block_device_sync  (const struct lfs2_config *c);


/* Local functions definition =============================================================================*/

// Read a region in a block. Negative error codes are propogated
// to the user.
static int user_provided_block_device_read (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size)
{
//	AZX_LOG_INFO("\r\n");
//	AZX_LOG_DEBUG("user_provided_block_device_read \r\n");
//	AZX_LOG_DEBUG("%p - block: 0x%08x , offset: 0x%08x, size: %u \r\n", c, block, off, size);
//	AZX_LOG_INFO("\r\n");

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
//	AZX_LOG_INFO("\r\n");
//	AZX_LOG_DEBUG("user_provided_block_device_prog \r\n");
//	AZX_LOG_DEBUG("%p - block: 0x%08x , offset: 0x%08x , size: %u \r\n", c, block, off, size);
//	AZX_LOG_INFO("\r\n");

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
//	AZX_LOG_INFO("\r\n");
//	AZX_LOG_DEBUG("user_provided_block_device_erase \r\n");
//	AZX_LOG_DEBUG("%p - block: 0x%08x \r\n", c, block);
//	AZX_LOG_INFO("\r\n");

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
//	AZX_LOG_INFO("\r\n");
//	AZX_LOG_DEBUG("user_provided_block_device_sync \r\n");
//	AZX_LOG_INFO("\r\n");

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
                  nandJscCallback
Function:      TODO: da finire
Arguments:
Return Value:
Description:
*******************************************************************************/
static void nandLFSCallback (void *h, AZX_SPI_FLASH_JSC_EVENT_E event, UINT16 resp_size, void *resp_struct, void *userdata )
{
	//AZX_LOG_INFO("LFS Callback event <%d>\r\n", (int) event);
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

static TEST_RESULT_E fileList (void)
{
	AZX_LOG_INFO("\n\n\r---------<><><><>fileList\r\n");

		int res;
		lfs2_dir_t dir;
		struct lfs2_info info;
		res = lfs2_dir_open(&lfs2, &dir, "/");
		if(res < 0)
		{
			AZX_LOG_ERROR("\n\rError opening directory\r\n");
			return TEST_ERROR;
		}

		res = lfs2_dir_read(&lfs2, &dir, &info);
		if(res < 0)
		{
			AZX_LOG_ERROR("\n\rError reading directory <.>\r\n");
		}
		assert(info.type == LFS2_TYPE_DIR);
		assert(strcmp(info.name, ".") == 0);

		res = lfs2_dir_read(&lfs2, &dir, &info);
		if(res < 0)
		{
			AZX_LOG_ERROR("\n\rError reading directory <..>\r\n");
			return TEST_ERROR;
		}
		assert(info.type == LFS2_TYPE_DIR);
		assert(strcmp(info.name, "..") == 0);

		do
		{
			res = lfs2_dir_read(&lfs2, &dir, &info);
			if(res < 0)
			{
				AZX_LOG_ERROR("\n\rError reading...\r\n");
				return TEST_ERROR;
			}

			if(info.type == LFS2_TYPE_REG)
			{
				AZX_LOG_INFO("\n\r%s, %d\r\n", info.name, info.size);
			}

		}
		while(res != 0);
		lfs2_dir_close(&lfs2, &dir);

		return TEST_OK;
}

static TEST_RESULT_E fileRead (const char *name)
{
	AZX_LOG_INFO("\n\n\r---------<><><><>fileRead\r\n");

	int res;
	lfs2_file_t p_file;
	res = lfs2_file_open(&lfs2, &p_file, name, LFS2_O_RDONLY);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError opening file: %s\r\n", name);
		return TEST_ERROR;
	}

	char buffer[10] = {0};
	int size = 10;
	res = lfs2_file_read(&lfs2, &p_file, buffer, size);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError reading file: %s\r\n", name);
		return TEST_ERROR;
	}
	AZX_LOG_INFO("Size: %d, Buffer: %s\r\n",size, buffer);

	res = lfs2_file_close(&lfs2, &p_file);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError closing file: %s\r\n", name);
		return TEST_ERROR;
	}
	return TEST_OK;
}

static TEST_RESULT_E fileCreation (const char *name, const char *content)
{
	AZX_LOG_INFO("\n\n\r---------<><><><>fileCreation\r\n");

	int res;
	lfs2_file_t p_file;


	res = lfs2_file_open(&lfs2, &p_file, name, LFS2_O_CREAT);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError creating file: %s\r\n", name);
		return TEST_ERROR;
	}

    int size = strlen(content);
	res = lfs2_file_write(&lfs2, &p_file, content, size);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError writing the content %s in the file: %s\r\n", content, name);
		return TEST_ERROR;
	}
	AZX_LOG_INFO("Size: %d, Content: %s\r\n",size, content);


	res = lfs2_file_close(&lfs2, &p_file);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError closing file: %s\r\n", name);
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\n\rFile created and closed: %s\r\n", name);
	return TEST_OK;
}

static TEST_RESULT_E fileRemove	(const char *name)
{
	AZX_LOG_INFO("\n\n\r---------<><><><>fileRemove\r\n");

	int res;
	lfs2_dir_t dir;

    res = lfs2_dir_open(&lfs2, &dir, "/");
	if(res < 0)
	{
		AZX_LOG_ERROR("\n\rError opening root directory\r\n");
		return TEST_ERROR;
	}

	res = lfs2_remove(&lfs2, name);
	if(res < 0)
	{
		AZX_LOG_ERROR("\n\rError removing file\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\n\rFile removed: %s\r\n", name);

    res = lfs2_dir_close(&lfs2, &dir);
	if(res < 0)
	{
		AZX_LOG_ERROR("\n\rError closing root directory\r\n");
		return TEST_ERROR;
	}

	return TEST_OK;
}

static TEST_RESULT_E directoryList (void)
{
	AZX_LOG_INFO("\n\n\r---------<><><><>directoryListing\r\n");

	int res;
	lfs2_dir_t dir;
	struct lfs2_info info;

	AZX_LOG_INFO("\n\rReading directory from root:\r\n");

	res = lfs2_dir_open(&lfs2, &dir, "/");
	if(res < 0)
	{
		AZX_LOG_ERROR("\n\rError opening directory\r\n");
		return TEST_ERROR;
	}

	res = lfs2_dir_read(&lfs2, &dir, &info);
	if(res < 0)
	{
		AZX_LOG_ERROR("\n\rError reading directory <.>\r\n");
	}
	AZX_LOG_INFO("\n\r%s\r\n", info.name);
	assert(info.type == LFS2_TYPE_DIR);
	assert(strcmp(info.name, ".") == 0);

	res = lfs2_dir_read(&lfs2, &dir, &info);
	if(res < 0)
	{
		AZX_LOG_ERROR("\n\rError reading directory <..>\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\n\r%s\r\n", info.name);
	assert(info.type == LFS2_TYPE_DIR);
	assert(strcmp(info.name, "..") == 0);

	do
	{
		res = lfs2_dir_read(&lfs2, &dir, &info);
		if(res < 0)
		{
			AZX_LOG_ERROR("\n\rError reading directory\r\n");
			return TEST_ERROR;
		}

		if(info.type == LFS2_TYPE_DIR)
		{
			AZX_LOG_INFO("\n\r%s\r\n", info.name);
		}

	}
	while(res != 0);
	lfs2_dir_close(&lfs2, &dir);

	return TEST_OK;
}

static TEST_RESULT_E directoryCreation	(const char *name)
{
	AZX_LOG_INFO("\n\n\r---------<><><><>directoryCreation\r\n");

	int res;

	res = lfs2_mkdir(&lfs2, name);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError creating directory: %s\r\n", name);
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\n\r directory created: %s\r\n", name);

	return TEST_OK;
}

static TEST_RESULT_E directoryRemove	(const char *name)
{
	AZX_LOG_INFO("\n\n\r---------<><><><>directoryRemove\r\n");

	int res;

	res = lfs2_remove(&lfs2, name);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError removing directory: %s\r\n", name);
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\n\r directory removed: %s\r\n", name);

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


	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}

	context = g_chip;

	// Configuration provided during initialization of the littlefs
	const struct lfs2_config cfg = {

		// Opaque user provided context that can be used to pass
		// information to the block device operations
		context,

		// Read a region in a block. Negative error codes are propogated
		// to the user.
		//int (*read)(const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size);
		user_provided_block_device_read,

		// Program a region in a block. The block must have previously
		// been erased. Negative error codes are propogated to the user.
		// May return LFS2_ERR_CORRUPT if the block should be considered bad.
		//int (*prog)(const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size);
		user_provided_block_device_prog,

		// Erase a block. A block must be erased before being programmed.
		// The state of an erased block is undefined. Negative error codes
		// are propogated to the user.
		// May return LFS2_ERR_CORRUPT if the block should be considered bad.
		//int (*erase)(const struct lfs2_config *c, lfs2_block_t block);
		user_provided_block_device_erase,

		// Sync the state of the underlying block device. Negative error codes
		// are propogated to the user.
		//int (*sync)(const struct lfs2_config *c);
		user_provided_block_device_sync,

		// Minimum size of a block read. All read operations will be a
		// multiple of this value.
		read_size,

		// Minimum size of a block program. All program operations will be a
		// multiple of this value.
		prog_size,

		// Size of an erasable block. This does not impact ram consumption and
		// may be larger than the physical erase size. However, non-inlined files
		// take up at minimum one block. Must be a multiple of the read
		// and program sizes.
		block_size,

		// Number of erasable blocks on the device.
		block_count,

		// Number of erase cycles before littlefs evicts metadata logs and moves
		// the metadata to another block. Suggested values are in the
		// range 100-1000, with large values having better performance at the cost
		// of less consistent wear distribution.
		//
		// Set to -1 to disable block-level wear-leveling.
		block_cycles,

		// Size of block caches. Each cache buffers a portion of a block in RAM.
		// The littlefs needs a read cache, a program cache, and one additional
		// cache per file. Larger caches can improve performance by storing more
		// data and reducing the number of disk accesses. Must be a multiple of
		// the read and program sizes, and a factor of the block size.
		cache_size,

		// Size of the lookahead buffer in bytes. A larger lookahead buffer
		// increases the number of blocks found during an allocation pass. The
		// lookahead buffer is stored as a compact bitmap, so each byte of RAM
		// can track 8 blocks. Must be a multiple of 8.
		lookahead_size,

		// Optional statically allocated read buffer. Must be cache_size.
		// By default lfs2_malloc is used to allocate this buffer.
		read_buffer,

		// Optional statically allocated program buffer. Must be cache_size.
		// By default lfs2_malloc is used to allocate this buffer.
		prog_buffer,

		// Optional statically allocated lookahead buffer. Must be lookahead_size
		// and aligned to a 32-bit boundary. By default lfs2_malloc is used to
		// allocate this buffer.
		lookahead_buffer,

		// Optional upper limit on length of file names in bytes. No downside for
		// larger names except the size of the info struct which is controlled by
		// the LFS2_NAME_MAX define. Defaults to LFS2_NAME_MAX when zero. Stored in
		// superblock and must be respected by other littlefs drivers.
		LFS2_NAME_MAX,

		// Optional upper limit on files in bytes. No downside for larger files
		// but must be <= LFS2_FILE_MAX. Defaults to LFS2_FILE_MAX when zero. Stored
		// in superblock and must be respected by other littlefs drivers.
		LFS2_FILE_MAX,

		// Optional upper limit on custom attributes in bytes. No downside for
		// larger attributes size but must be <= LFS2_ATTR_MAX. Defaults to
		// LFS2_ATTR_MAX when zero.
		LFS2_ATTR_MAX
	};


	//we need to allocate the resource because it must be available also out of this function
	lfs2_cfg = (struct lfs2_config*) m2mb_os_malloc(sizeof(struct lfs2_config));
	if (!lfs2_cfg)
	{
		AZX_LOG_ERROR("Error alloc config memory lfs!!\r\n");
		return TEST_ERROR;
	}
	*lfs2_cfg = cfg;

	//print information
	//lfs2_log_info(&lfs, lfs2_cfg);

	int err;


	AZX_LOG_INFO("\nFormatting... \r\n", err);
	err = lfs2_format(&lfs2, lfs2_cfg);
	if (err)
	{
		AZX_LOG_ERROR("\nError formatting memory: err = %d \r\n", err);
		return TEST_ERROR;
	}

	AZX_LOG_INFO("\nMounting partition...\r\n");
	// mount the filesystem
	err = lfs2_mount(&lfs2, lfs2_cfg);
	// reformat if we can't mount the filesystem
	// this should only happen on the first boot
	AZX_LOG_INFO("\nResult mounting err = %d \r\n", err);
	if (err)
	{
		AZX_LOG_ERROR("\nError mounting memory: err = %d \r\n", err);
		return TEST_ERROR;
	}

	if (fileList() != TEST_OK)
	{
		AZX_LOG_ERROR("fileList failed!!\r\n");
		return TEST_ERROR;
	}

	AZX_LOG_INFO("\nFormatting... \r\n", err);
	err = lfs2_format(&lfs2, lfs2_cfg);
	if (err)
	{
		AZX_LOG_ERROR("\nError formatting memory: err = %d \r\n", err);
		return TEST_ERROR;
	}

	AZX_LOG_INFO("\nMounting partition...\r\n");
	// mount the filesystem
	err = lfs2_mount(&lfs2, lfs2_cfg);
	AZX_LOG_INFO("\nResult mounting err = %d \r\n", err);
	if (err)
	{
		AZX_LOG_ERROR("\nError mounting memory: err = %d \r\n", err);
		return TEST_ERROR;
	}

	if (fileList() != TEST_OK)
	{
		AZX_LOG_ERROR("fileList failed!!\r\n");
		return TEST_ERROR;
	}

	char content [10] = {0};

	memset(content, 0x00, sizeof(content));
	strcat(content, "content000");
	if (fileCreation("file000.txt", content) != TEST_OK)
	{
		AZX_LOG_ERROR("fileCreation failed!!\r\n");
		return TEST_ERROR;
	}

	memset(content, 0x00, sizeof(content));
	strcat(content, "content001");
	if (fileCreation("file001.txt", content) != TEST_OK)
	{
		AZX_LOG_ERROR("fileCreation failed!!\r\n");
		return TEST_ERROR;
	}

	//GENERA L'ERRORE
	azx_sleep_ms(1000);
//	AZX_LOG_INFO("Start for Count!!\r\n");
//	for(int n=0; n < 500; n++)
//	{
//		AZX_LOG_INFO("Count!!\r\n");
//	}
//	AZX_LOG_INFO("End for Count!!\r\n");


	memset(content, 0x00, sizeof(content));
	strcat(content, "content002");
	if (fileCreation("file002.txt", content) != TEST_OK)
	{
		AZX_LOG_ERROR("fileCreation failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileList() != TEST_OK)
	{
		AZX_LOG_ERROR("fileList failed!!\r\n");
		return TEST_ERROR;
	}
	memset(content, 0x00, sizeof(content));
	strcat(content, "content003");
	if (fileCreation("file003.txt", content) != TEST_OK)
	{
		AZX_LOG_ERROR("fileCreation failed!!\r\n");
		return TEST_ERROR;
	}
	memset(content, 0x00, sizeof(content));
	strcat(content, "content004");
	if (fileCreation("file004.txt", content) != TEST_OK)
	{
		AZX_LOG_ERROR("fileCreation failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileList() != TEST_OK)
	{
		AZX_LOG_ERROR("fileList failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileRead("file000.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRead failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileRead("file004.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRead failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileRead("file002.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRead failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileRemove("file000.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRemove failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileRemove("file002.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRemove failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileRemove("file004.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRemove failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileList() != TEST_OK)
	{
		AZX_LOG_ERROR("fileList failed!!\r\n");
		return TEST_ERROR;
	}
#if 0
	memset(content, 0x00, sizeof(content));
	strcat(content, "content001");
	if (fileCreation("file001.txt", content) != TEST_OK)
	{
		AZX_LOG_ERROR("fileCreation failed!!\r\n");
		return TEST_ERROR;
	}
	memset(content, 0x00, sizeof(content));
	strcat(content, "content002");
	if (fileCreation("file002.txt", content) != TEST_OK)
	{
		AZX_LOG_ERROR("fileCreation failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileList() != TEST_OK)
	{
		AZX_LOG_ERROR("fileList failed!!\r\n");
		return TEST_ERROR;
	}

	if (fileRead("file000.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRead failed!!\r\n");
		return TEST_ERROR;
	}
	if (fileRead("file002.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRead failed!!\r\n");
		return TEST_ERROR;
	}


	if (fileList() != TEST_OK)
	{
		AZX_LOG_ERROR("fileList failed!!\r\n");
		return TEST_ERROR;
	}

	if (fileRemove("file000.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRemove failed!!\r\n");
		return TEST_ERROR;
	}


	if (fileRemove("file001.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRemove failed!!\r\n");
		return TEST_ERROR;
	}

	if (fileRemove("file002.txt") != TEST_OK)
	{
		AZX_LOG_ERROR("fileRemove failed!!\r\n");
		return TEST_ERROR;
	}


	if (fileList() != TEST_OK)
	{
		AZX_LOG_ERROR("fileList failed!!\r\n");
		return TEST_ERROR;
	}


	if (directoryList() != TEST_OK)
	{
		AZX_LOG_ERROR("directoryList failed!!\r\n");
		return TEST_ERROR;
	}
	if (directoryCreation("dir000") != TEST_OK)
	{
		AZX_LOG_ERROR("directoryCreation failed!!\r\n");
		return TEST_ERROR;
	}
	if (directoryCreation("dir001") != TEST_OK)
	{
		AZX_LOG_ERROR("directoryCreation failed!!\r\n");
		return TEST_ERROR;
	}
	if (directoryCreation("dir002") != TEST_OK)
	{
		AZX_LOG_ERROR("directoryCreation failed!!\r\n");
		return TEST_ERROR;
	}
	if (directoryList() != TEST_OK)
	{
		AZX_LOG_ERROR("directoryList failed!!\r\n");
		return TEST_ERROR;
	}
	if (directoryRemove("dir000") != TEST_OK)
	{
		AZX_LOG_ERROR("directoryRemove failed!!\r\n");
		return TEST_ERROR;
	}
	if (directoryRemove("dir001") != TEST_OK)
	{
		AZX_LOG_ERROR("directoryRemove failed!!\r\n");
		return TEST_ERROR;
	}
	if (directoryRemove("dir002") != TEST_OK)
	{
		AZX_LOG_ERROR("directoryRemove failed!!\r\n");
		return TEST_ERROR;
	}
	if (directoryList() != TEST_OK)
	{
		AZX_LOG_ERROR("directoryList failed!!\r\n");
		return TEST_ERROR;
	}
#endif


	if (azx_spi_flash_releaseResources(g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Release memory failed!!\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("Nand released\r\n");

	err = lfs2_unmount(&lfs2);
	if (err)
	{
		AZX_LOG_ERROR("\nError unmounting memory: err = %d \r\n", err);
		return TEST_ERROR;
	}
	AZX_LOG_INFO("Partition unmounted\r\n");

	if (lfs2_cfg)
	{
		err = m2mb_os_free(lfs2_cfg);
		if (err < 0)
		{
			AZX_LOG_ERROR("Release memory failed!!\r\n");
			return TEST_ERROR;
		}
	}
	AZX_LOG_INFO("Partition resources released\r\n");


	AZX_LOG_INFO("\n\rUnmounted process ended...\r\n");


	AZX_LOG_INFO("\n\rtestAllInOneFunction ended...\r\n");
	return TEST_OK;
}

TEST_RESULT_E runRawLfsUsage_TestGroup(void)
{
	//all in one function
	if (testAllInOneFunction() != TEST_OK) //if error
	{
		AZX_LOG_ERROR("setup failed!!\r\n");
		return TEST_ERROR;
	}

	return TEST_OK;
}


