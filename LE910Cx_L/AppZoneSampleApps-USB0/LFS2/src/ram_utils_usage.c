/*
 * ram_utils_usage.c
 *
 *  Created on: Oct 13, 2020
 *      Author: NormanAr
 */

/* Include files ================================================================================*/
#include <string.h>

#include "m2mb_os_api.h"

#include "azx_utils.h"
#include "azx_log.h"
#include "lfs2.h"

#include "azx_lfs_utils.h"

#include "lfs_usage.h"


/* Local variables definition =============================================================================*/

static AZX_RAM_DISK_S* g_ram;

// variables used by the filesystem
static AZX_LFS_CONTEXT_S g_context;
static struct lfs2_config lfsContext;
static lfs2_t lfs2;



static void *read_buffer;
static void *prog_buffer;
static void *lookahead_buffer;
static lfs2_size_t read_size = 2048;	   		 //16;
static lfs2_size_t prog_size = 2048; 			 //2048;
static int32_t block_cycles = 500; 				 //1000;
static lfs2_size_t cache_size = 2048;    		 //131072/32=4096
static lfs2_size_t lookahead_size = 16;			 //16;
static const lfs2_size_t block_size = 16384;	 //131072;
static const lfs2_size_t block_count = 16;		 //128;

static char** RAM_Disk;

/* Local function prototypes ====================================================================*/
static int user_provided_block_device_read  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size);
static int user_provided_block_device_prog  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size);
static int user_provided_block_device_erase (const struct lfs2_config *c, lfs2_block_t block);
static int user_provided_block_device_sync  (const struct lfs2_config *c);

static TEST_RESULT_E fileListUtils (void);


/* Local function definition ====================================================================*/

// Read a region in a block. Negative error codes are propogated
// to the user.
static int user_provided_block_device_read  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size)
{
	(void) c;

	if (size > block_size - off)
	{
    	//an error occur
    	return LFS2_ERR_IO;
	}
	memcpy(buffer, &RAM_Disk[block][off], size);
	return LFS2_ERR_OK;
}

static int user_provided_block_device_prog  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size)
{
	(void) c;

	if (size > block_size - off)
	{
    	//an error occur
    	return LFS2_ERR_IO;
	}
	//&RAM_Disk[block][off] --> &RAM_Disk[0][0] + block << block_shift + off
	//off -> page * page_shift
	memcpy(&RAM_Disk[block][off], buffer, size);
	return LFS2_ERR_OK;
}

static int user_provided_block_device_erase (const struct lfs2_config *c, lfs2_block_t block)
{
	(void) c;

	memset(&RAM_Disk[block][0], 0, block_size);
	return LFS2_ERR_OK;
}

static int user_provided_block_device_sync  (const struct lfs2_config *c)
{
	(void) c;

	AZX_LOG_TRACE("user_provided_block_device_sync \r\n");

	azx_sleep_ms(500);
	return LFS2_ERR_OK;
}


static TEST_RESULT_E fileListUtils (void)
{
	AZX_LOG_INFO("\r\n<><><><>fileListUtils\r\n");

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

TEST_RESULT_E ramDiskDemo (void)
{
	AZX_LOG_INFO("\r\n>>>>>>> Starting RAMDiskDemo ...\r\n");

	//ram initialization with memory allocation
	if (azx_ram_initialize(&g_ram, &RAM_Disk,
			read_size, prog_size, block_size, block_count, block_cycles, cache_size) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("Ram Initialization failed!!\r\n");
		return TEST_ERROR;
	}

	//	struct lfs2_config lfsContext;
	lfsContext.context = g_ram;
	lfsContext.read = user_provided_block_device_read;
	lfsContext.prog = user_provided_block_device_prog;
	lfsContext.erase =user_provided_block_device_erase;
	lfsContext.sync = user_provided_block_device_sync;

	lfsContext.read_size = g_ram->read_size;
	lfsContext.prog_size = g_ram->prog_size;
	lfsContext.block_size = g_ram->block_size;
	lfsContext.block_count = g_ram->block_count;
	lfsContext.block_cycles = g_ram->block_cycles;
	lfsContext.cache_size = g_ram->cache_size;

	lfsContext.lookahead_size = lookahead_size;
	lfsContext.read_buffer = read_buffer;
	lfsContext.prog_buffer = prog_buffer;
	lfsContext.lookahead_buffer = lookahead_buffer;

	lfsContext.name_max = LFS2_NAME_MAX;
	lfsContext.file_max = LFS2_FILE_MAX;
	lfsContext.attr_max = LFS2_ATTR_MAX;

	//set file system context
	g_context.context = &lfsContext;
	g_context.type = AZX_LFSCONTEXT_RAM;


	LFS2_ERROR_T ret;
	int err = 0;

	AZX_LOG_INFO("\r\nMounting partition...\r\n");
	// mount the filesystem
	ret = azx_lfsMountByContext(AZX_LFSMOUNT_MODE_FORMAT_AND_MOUNT,
								&g_context,
								&lfs2);
	if (ret != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("\r\nError lfsMountByContext memory: err = %d \r\n", err);
		return TEST_ERROR;
	}

	if (fileListUtils() != TEST_OK)
	{
		AZX_LOG_ERROR("fileListUtils failed!!\r\n");
		return TEST_ERROR;
	}


	char content [32] = {0};

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


	if (fileListUtils() != TEST_OK)
	{
		AZX_LOG_ERROR("fileListUtils failed!!\r\n");
		return TEST_ERROR;
	}

	AZX_LOG_INFO("------>File reading\r\n");
	char buffer[11];
	unsigned int size = 10;

	memset(buffer, 0x00, sizeof(buffer));
	if (azx_lfsReadByContext("file000.txt",
			  	  	  	  	buffer,
							size,
							&lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfsReadByContext failed!!\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\r\nFile: %s, Size: %d, Buffer: %s\r\n\n", "file000.txt", size, buffer);


	AZX_LOG_INFO("RAMDisk released\r\n");
	err = lfs2_unmount(&lfs2);
	if (err)
	{
		AZX_LOG_ERROR("\r\nError unmounting memory: err = %d \r\n", err);
		return TEST_ERROR;
	}
	AZX_LOG_INFO("Partition unmounted\r\n");

	//free memory
	if (azx_ram_releaseResources(g_ram, RAM_Disk) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("Release Ram memory failed!!\r\n");
		return TEST_ERROR;
	}

	return TEST_OK;
}
