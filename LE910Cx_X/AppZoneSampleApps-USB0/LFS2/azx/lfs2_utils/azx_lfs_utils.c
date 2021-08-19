/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
  	  azx_lfs_flash_utils.c

  @brief
  	  Project: SPI data flash

  @details

  @version
  	  1.0.1

  @note

  @author
  	  Norman Argiolas

  @dte
  	  File created on: Sep 10, 2020
*/

/* Include files =========================================================================================*/
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "m2mb_types.h"
#include "azx_utils.h"
#include "m2mb_os_api.h"
#include "azx_log.h"

#include "azx_lfs_utils.h"



/* Function definition ===================================================================================*/
static LFS2_ERROR_T getFlashByteSize(const AZX_LFS_CONTEXT_S *c, UINT32 *size);

/*******************************************************************************
                  isInitialize
*******************************************************************************/
//static BOOLEAN isReady(AZX_LFS_STATUS_UTILS *par)
//{
//	if (par == NULL ||
//		par->isInit != TRUE ||
//		par->isMount !=TRUE)
//	{
//		AZX_LOG_DEBUG("LFS not initialized!!!\r\n");
//		return FALSE;
//	}
//
//	return TRUE;
//}
/***************** isInitialize ***********************************************/


/*******************************************************************************
                  azx_ram_initialize
*******************************************************************************/
LFS2_ERROR_T azx_ram_initialize(AZX_RAM_DISK_S** ptr_ram,
								char*** disk,
								uint32_t read_size,
								uint32_t prog_size,
								uint32_t block_size,
								uint32_t block_count,
								int32_t  block_cycles,
								uint32_t cache_size)
{
	AZX_RAM_DISK_S *ram = NULL;
	ram = (AZX_RAM_DISK_S*) m2mb_os_malloc(sizeof(AZX_RAM_DISK_S));
	if (!ram)
	{
		AZX_LOG_ERROR("Error memory allocation!!\r\n");
		return LFS2_ERR_GENERIC;
	}
	ram->read_size = read_size;
	ram->prog_size = prog_size;
	ram->block_size = block_size;
	ram->block_count = block_count;
	ram->block_cycles = block_cycles;
	ram->cache_size = cache_size;

	*ptr_ram = ram;

	//todo check valid allocation
	const uint32_t rows = block_count;
	const uint32_t cols = block_size;


	/* defining a temp ptr, otherwise would have to use (*memory) everywhere ptr is used*/
	char** ptr;

	/* Each row should only contain an char*, not an char**,
	 * because each row will be an array of char */
	ptr = (char**) m2mb_os_malloc(rows * sizeof(char*));
	if (!ptr)
	{
		AZX_LOG_ERROR("Error rows memory allocation!!\r\n");
		return LFS2_ERR_GENERIC;
	}
	/* rows/cols are unsigned, so this should be too */
	uint32_t i;
	for(i = 0; i < rows; i++)
    {
		ptr[i] = (char*) m2mb_os_malloc(cols * sizeof(char));
		if (!ptr[i])
		{
			AZX_LOG_ERROR("Error cols memory allocation!!\r\n");
			uint32_t j;
			for(j = 0; j<i;j++)
			{
				if (m2mb_os_free(ptr[j]) < 0)
				{
					AZX_LOG_ERROR("Error release allocation!!\r\n");
				}
			}
			return LFS2_ERR_GENERIC;
		}
    }
	/* it worked so return ptr */
	*disk = ptr;

	AZX_LOG_DEBUG("Ram Memory allocated correctly from %p to %p!!\r\n", ptr[0], ptr[rows]);
	return LFS2_ERR_OK;
}

/***************** azx_ram_initialize *****************************************/


/*******************************************************************************
                  azx_ram_initialize
*******************************************************************************/
LFS2_ERROR_T azx_ram_releaseResources(AZX_RAM_DISK_S* ptr_ram, char** disk)
{
	LFS2_ERROR_T res = LFS2_ERR_OK;

	uint32_t i;
	const uint32_t rows = ptr_ram->block_count;

	for(i = 0; i < rows; i++)
    {
		if (m2mb_os_free(disk[i]) != M2MB_OS_SUCCESS)
		{
			AZX_LOG_ERROR("Error releasing row %d!!\r\n", i);
			res = LFS2_ERR_GENERIC;
			//keep releasing the other
		}
    }
	if (m2mb_os_free(disk) != M2MB_OS_SUCCESS)
	{
		AZX_LOG_ERROR("Error releasing disk memory!!\r\n", i);
		res = LFS2_ERR_GENERIC;
	}

	if(m2mb_os_free(ptr_ram) != M2MB_OS_SUCCESS )
	{
		AZX_LOG_ERROR("\nError free struct ram memory!!\r\n");
		res = LFS2_ERR_GENERIC;
	}

	if (res == LFS2_ERR_OK)
	{
		AZX_LOG_DEBUG("Ram Memory released correctly!!\r\n");
	}
	return res;
}
/***************** azx_ram_initialize *****************************************/


/*******************************************************************************
                  azx_lfsMountByContext
*******************************************************************************/
LFS2_ERROR_T azx_lfsMountByContext (AZX_LFSMOUNT_MODE_E mode,
									const AZX_LFS_CONTEXT_S *c,
									lfs2_t *lfsObject)
{

	//check for file system context
	if (c == NULL)
	{
		AZX_LOG_ERROR("File System Context not initialized!!\r\n");
		return LFS2_ERR_GENERIC;
	}

	const struct lfs2_config* lfsContext = c->context;

	//check for valid mounting mode
	if (mode != AZX_LFSMOUNT_MODE_MOUNT &&
		mode != AZX_LFSMOUNT_MODE_MOUNT_OR_FORMAT &&
		mode != AZX_LFSMOUNT_MODE_FORMAT_AND_MOUNT &&
		mode != AZX_LFSMOUNT_MODE_LOW_FORMAT_AND_MOUNT)
	{
		AZX_LOG_ERROR("Mode not valid!!\r\n");
		return LFS2_ERR_GENERIC;
	}
	int err;
	switch (mode)
	{
	case AZX_LFSMOUNT_MODE_MOUNT: /*Mount and exit if error*/
	{
		AZX_LOG_INFO("\nMounting... \r\n");
		err = lfs2_mount(lfsObject, lfsContext);
		if (err)
		{
			AZX_LOG_ERROR("\nError mounting memory: err = %d \r\n", err);
			return LFS2_ERR_GENERIC;
		}
		break;
	}
	case AZX_LFSMOUNT_MODE_MOUNT_OR_FORMAT: /*Mount and format if error*/
	{
		AZX_LOG_INFO("\nMounting... \r\n");
		err = lfs2_mount(lfsObject, lfsContext);
		// reformat if we can't mount the filesystem
		// this should only happen on the first boot
		if (err)
		{
			AZX_LOG_INFO("\nFormatting... \r\n", err);
			err = lfs2_format(lfsObject, lfsContext);
			if (err)
			{
				AZX_LOG_ERROR("\nError formatting memory: err = %d \r\n", err);
				return LFS2_ERR_GENERIC;
			}

			AZX_LOG_INFO("\nMounting... \r\n", err);
			err = lfs2_mount(lfsObject, lfsContext);
			if (err)
			{
				AZX_LOG_ERROR("\nError mounting memory: err = %d \r\n", err);
				return LFS2_ERR_GENERIC;
			}
		}
		break;
	}
	case AZX_LFSMOUNT_MODE_FORMAT_AND_MOUNT: /*Format and mount*/
	{
		AZX_LOG_INFO("\nFormatting... \r\n");
		err = lfs2_format(lfsObject, lfsContext);
		if (err)
		{
			AZX_LOG_ERROR("\nError formatting memory: err = %d \r\n", err);
			return LFS2_ERR_GENERIC;
		}
		AZX_LOG_INFO("\nMounting... \r\n", err);
		err = lfs2_mount(lfsObject, lfsContext);
		if (err)
		{
			AZX_LOG_ERROR("\nError mounting memory: err = %d \r\n", err);
			return LFS2_ERR_GENERIC;
		}
		break;
	}
	case AZX_LFSMOUNT_MODE_LOW_FORMAT_AND_MOUNT: /*Low Level Format and mount*/
	{
		if (c->type != AZX_LFSCONTEXT_FLASH)
		{
			AZX_LOG_ERROR("\nContext type not valid!\r\n");
			return LFS2_ERR_GENERIC;
		}

		AZX_LOG_INFO("\nErasing... \r\n");
		AZX_SPI_FLASH_SPI_CHIP *spiFlashContext = (AZX_SPI_FLASH_SPI_CHIP*) lfsContext->context;

		if (flashLowLevelFormatByContext (spiFlashContext) != LFS2_ERR_OK)
		{
			AZX_LOG_ERROR("\nError low level format!\r\n");
			return LFS2_ERR_GENERIC;
		};

		AZX_LOG_INFO("\nFormatting... \r\n");
		err = lfs2_format(lfsObject, lfsContext);
		if (err)
		{
			AZX_LOG_ERROR("\nError formatting memory: err = %d \r\n", err);
			return LFS2_ERR_GENERIC;
		}
		AZX_LOG_INFO("\nMounting... \r\n", err);
		err = lfs2_mount(lfsObject, lfsContext);
		if (err)
		{
			AZX_LOG_ERROR("\nError mounting memory: err = %d \r\n", err);
			return LFS2_ERR_GENERIC;
		}
		break;
	}	
	}
	AZX_LOG_INFO("\n\rMounted partition...\r\n");

	return LFS2_ERR_OK;
}
/***************** azx_lfsMountByContext ***********************************************/


/*******************************************************************************
                  getFlashByteSize
return:
	size in byte of the memory mounted on chip,
	-1 in case of error
*******************************************************************************/

static LFS2_ERROR_T getFlashByteSize(const AZX_LFS_CONTEXT_S *c, UINT32 *size)
{
	if (c != NULL)
	{
		switch (c->type)
		{
		case AZX_LFSCONTEXT_FLASH:
		{
			//type->page_size * type->pages_per_blk * type->blks_per_lun;
			AZX_SPI_FLASH_SPI_CHIP* flash = (AZX_SPI_FLASH_SPI_CHIP *) c->context->context;
			*size = flash->size;
			return LFS2_ERR_OK;
		}

		case AZX_LFSCONTEXT_RAM:
		{
			//TODO finish code here
			*size = 10000;
			AZX_LOG_INFO("RAM TYPE size: %d\r\n", *size);
			return LFS2_ERR_OK;
		}
		default:
			return LFS2_ERR_GENERIC;
		}
	}
	return LFS2_ERR_GENERIC;

}
/***************** getFlashByteSize ***********************************************/


/*******************************************************************************
                  azx_lfs_flashListsContentByContext
*******************************************************************************/

LFS2_ERROR_T azx_lfsListsContentByContext(char *path,
										  AZX_LFS_INFO_FILE_E **content_ptr,
										  lfs2_t *lfsObject)
{
	int res;
	struct lfs2_info info;
	lfs2_dir_t lfsDir;

	res = lfs2_dir_open(lfsObject, &lfsDir, path);
	if(res < 0)
	{
		AZX_LOG_ERROR("\n\rError opening directory\r\n");
		return LFS2_ERR_GENERIC;
	}
	res = lfs2_dir_read(lfsObject, &lfsDir, &info);
	if(res < 0)
	{
		AZX_LOG_ERROR("\n\rError reading directory <..>\r\n");
		return LFS2_ERR_GENERIC;
	}
	assert(info.type == LFS2_TYPE_DIR);
	assert(strcmp(info.name, ".") == 0);


	AZX_LFS_INFO_FILE_E *content;
	AZX_LFS_INFO_FILE_E *prev = NULL;

	content = (AZX_LFS_INFO_FILE_E*) m2mb_os_malloc(sizeof(AZX_LFS_INFO_FILE_E));
	if (!content)
	{
		AZX_LOG_ERROR("Error alloc memory\r\n");
		return LFS2_ERR_GENERIC;
	}
	//content->info = info;
	memcpy(&content->info, &info, sizeof(struct lfs2_info));
	content->next = NULL;
	*content_ptr = content; //first

	res = lfs2_dir_read(lfsObject, &lfsDir, &info);
	if(res < 0)
	{
		AZX_LOG_ERROR("\n\rError reading directory <..>\r\n");
		return LFS2_ERR_GENERIC;
	}
	assert(info.type == LFS2_TYPE_DIR);
	assert(strcmp(info.name, "..") == 0);

	prev = content;
	content = (AZX_LFS_INFO_FILE_E*) m2mb_os_malloc(sizeof(AZX_LFS_INFO_FILE_E));
	if (!content)
	{
		AZX_LOG_ERROR("Error alloc memory\r\n");
		return LFS2_ERR_GENERIC;
	}
	memcpy(&content->info, &info, sizeof(struct lfs2_info));
	content->next = NULL;
	prev->next = content;

	do
	{
		res = lfs2_dir_read(lfsObject, &lfsDir, &info);
		if(res < 0)
		{
			AZX_LOG_ERROR("\n\rError reading directory\r\n");
			return LFS2_ERR_GENERIC;
		}
		if (res != 0)
		{
			prev = content;
			content = (AZX_LFS_INFO_FILE_E*) m2mb_os_malloc(sizeof(AZX_LFS_INFO_FILE_E));
			if (!content)
			{
				AZX_LOG_ERROR("Error alloc memory\r\n");
				return LFS2_ERR_GENERIC;
			}
			AZX_LOG_TRACE("____INSIDE --->%s, %u, %u\r\n",info.name, info.size, info.type);

			memcpy(&content->info, &info, sizeof(struct lfs2_info));
			content->next = NULL;
			prev->next = content;
		}
	}
	while(res != 0);
	lfs2_dir_close(lfsObject, &lfsDir);

	return LFS2_ERR_OK;
}
/***************** azx_lfs_flashListsContentByContext ***********************************************/


/*******************************************************************************
                  azx_lfs_flashRmFileByContext
*******************************************************************************/
LFS2_ERROR_T azx_lfsRmFileByContext(lfs2_t *lfsObject,
								   	const char *path)
{
		AZX_LOG_INFO("%s<<<<<<<\r\n", path);
		int res;
		//lfs2_dir_t g_lfsDir;

//	    res = lfs2_dir_open(&g_lfsObject, &g_lfsDir, "/");
//		if(res < 0)
//		{
//			AZX_LOG_ERROR("\n\rError opening root directory\r\n");
//			return LFS2_ERR_GENERIC;
//		}

		res = lfs2_remove(lfsObject, path);
		if(res < 0)
		{
			AZX_LOG_ERROR("\n\rError removing file\r\n");
			return LFS2_ERR_GENERIC;
		}
		AZX_LOG_INFO("\n\rFile removed: %s\r\n", path);

//		res = lfs2_dir_close(&g_lfsObject, &g_lfsDir);
//		if(res < 0)
//		{
//			AZX_LOG_ERROR("\n\rError closing root directory\r\n");
//			return LFS2_ERR_GENERIC;
//		}

		return LFS2_ERR_OK;
}
/***************** azx_lfs_flashRmFileByContext ***********************************************/


/*******************************************************************************
                  azx_lfsWriteByContext
*******************************************************************************/
LFS2_ERROR_T azx_lfsWriteByContext(char *path,
										 UINT32 size,
										 UINT8 *source,
										 AZX_LFSWRITE_MODE_E mode,
										 lfs2_t *lfsObject,
										 const AZX_LFS_CONTEXT_S *context)
{
		AZX_LOG_INFO("file_name: %s\r\n", path);
		AZX_LOG_INFO("size: %d\r\n", size);
		AZX_LOG_INFO("buffer: %s\r\n", source);
		AZX_LOG_INFO("mode: %d\r\n", mode);
//
//		AZX_LOG_INFO("memory size: %ld MB \r\n", getFlashByteSize(g_spiFalshContext));
//		AZX_LOG_INFO("LFS max size: %ld MB \r\n", LFS2_FILE_MAX);


		lfs2_file_t lfsFileObject;

		//check for valid file size
		UINT32 flashSize = 0;
		if (getFlashByteSize(context, &flashSize) == LFS2_ERR_OK)
		{
			if (size > LFS2_FILE_MAX ||
				size > flashSize)
			{
				AZX_LOG_ERROR("File size not valid!!\r\n");
				return LFS2_ERR_GENERIC;
			}
		}
		else
		{
			AZX_LOG_ERROR("File size with spiFlashContext not valid!!\r\n");
			return LFS2_ERR_GENERIC;
		}



		int res;
		switch (mode)
		{
		case AZX_LFSWRITE_MODE_CREAT:

			//create the file
			res = lfs2_file_open(lfsObject, &lfsFileObject, path, LFS2_O_CREAT);
	        if (res < 0)
	        {
	        	AZX_LOG_ERROR("\n\rError creating file: %s\r\n", path);
	        	return LFS2_ERR_GENERIC;
	        }


	        //write the content
	    	res = lfs2_file_write(lfsObject, &lfsFileObject, source, size);
			if (res < 0)
			{
				AZX_LOG_ERROR("\n\rError writing the file: %s\r\n", source);
				return LFS2_ERR_GENERIC;
			}

	        //close the file
	        res = lfs2_file_close(lfsObject, &lfsFileObject);
	        if (res < 0)
	        {
	        	AZX_LOG_ERROR("\n\rError closing file: %s\r\n", path);
	        	return LFS2_ERR_GENERIC;
	        }
			AZX_LOG_INFO("\n\rFile created and closed: %s\r\n\n", path);

			break;
		case AZX_LFSWRITE_MODE_EXCL:
			//todo
			AZX_LOG_ERROR("\n\rNot implemented yet\r\n");

			break;
		case AZX_LFSWRITE_MODE_APPEND:
			//todo
			AZX_LOG_ERROR("\n\rNot implemented yet\r\n");

			break;
		default:
			AZX_LOG_ERROR("Mode not valid!!\r\n");
			return LFS2_ERR_GENERIC;
			break;
		}

		return LFS2_ERR_OK;
}
/***************** azx_lfsWriteByContext ***********************************************/


/*******************************************************************************
                  azx_lfsReadByContext
*******************************************************************************/
LFS2_ERROR_T azx_lfsReadByContext(const char *name,
								  char* buffer,
								  unsigned int size,
								  lfs2_t *lfsObject)
{
	int res;
	lfs2_file_t p_file;

	res = lfs2_file_open(lfsObject, &p_file, name, LFS2_O_RDONLY);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError opening file: %s\r\n", name);
		return LFS2_ERR_GENERIC;
	}

	res = lfs2_file_read(lfsObject, &p_file, buffer, size);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError reading file: %s\r\n", name);
		return LFS2_ERR_GENERIC;
	}

	res = lfs2_file_close(lfsObject, &p_file);
	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError closing file: %s\r\n", name);
		return LFS2_ERR_GENERIC;
	}

	return LFS2_ERR_OK;
}
/***************** azx_lfsReadByContext ***********************************************/


/*******************************************************************************
                  azx_lfsDirCreation
*******************************************************************************/
LFS2_ERROR_T azx_lfsDirCreationByContext(const char *dir,
								AZX_LFSDIR_MODE_E mode,
								lfs2_t *lfsObject)
{
	int res = LFS2_ERR_GENERIC;

	switch (mode)
	{
	case AZX_LFSDIR_MODE_CREAT:
		res = lfs2_mkdir(lfsObject, dir);
		if (res == LFS2_ERR_EXIST)
		{
			res = lfs2_remove(lfsObject, dir);
			if (res < 0)
			{
				AZX_LOG_ERROR("\n\rError removing directory: %s\r\n", dir);
				return LFS2_ERR_GENERIC;
			}
			AZX_LOG_DEBUG("Directory removed: %s!!\r\n", dir);
			res = lfs2_mkdir(lfsObject, dir);
		}
		break;
	case AZX_LFSDIR_MODE_EXCL:
		res = lfs2_mkdir(lfsObject, dir);
		break;
	case AZX_LFSDIR_MODE_APPEND:
		res = lfs2_mkdir(lfsObject, dir);
		if (res == LFS2_ERR_EXIST)
		{
			AZX_LOG_DEBUG("Directory already exists: %s!!\r\n", dir);
			return LFS2_ERR_OK;
		}

		break;
	default:
		AZX_LOG_ERROR("Mode not valid!!\r\n");
		return LFS2_ERR_GENERIC;
		break;
	}

	if (res < 0)
	{
		AZX_LOG_ERROR("\n\rError creating directory: %s\r\n", dir);
		return LFS2_ERR_GENERIC;
	}

	AZX_LOG_DEBUG("Directory created: %s!!\r\n", dir);
	return LFS2_ERR_OK;
}
/***************** azx_lfsDirCreation ***********************************************/


/*******************************************************************************
                  azx_lfsDirRmByContext
*******************************************************************************/
LFS2_ERROR_T azx_lfsDirRmByContext(const char *dir,
								   AZX_LFSRMDIR_MODE_E mode,
								   lfs2_t *lfsObject)
{
	int res;

	switch (mode)
	{
	case AZX_LFSRMDIR_MODE_EMPTY:
		res = lfs2_remove(lfsObject, dir);
		if (res < 0)
		{
			AZX_LOG_ERROR("\n\rError removing directory: %s\r\n", dir);
			return LFS2_ERR_GENERIC;
		}
		break;

	case AZX_LFSRMDIR_MODE_RM:
		//todo delete the directory content and retry to remove the directory
		AZX_LOG_ERROR("\n Nor implemented yet!!\n\r");

		break;
	default:
		AZX_LOG_ERROR("Mode not valid!!\r\n");
		return LFS2_ERR_GENERIC;
		break;
	}

	return LFS2_ERR_OK;
}
/***************** azx_lfsDirRmByContext **************************************/


/*******************************************************************************
                  azx_lfs_flashLowLevelFormat
*******************************************************************************/

LFS2_ERROR_T flashLowLevelFormatByContext(AZX_SPI_FLASH_SPI_CHIP *spiFlashContext)
{
	//Number of the blocks
	int counter = (spiFlashContext->size >> spiFlashContext->block_shift);

	AZX_LOG_INFO("Number of the blocks: %d \r\n", counter );
	AZX_LOG_INFO("block_shift: %u, size: %llu  \r\n", spiFlashContext->block_shift, spiFlashContext->size);

	AZX_SPI_FLASH_CODE_RESULT_E ret;

	//Starting from block 0
	int blk_index = 0;

	//remove this line
	counter = 10;

	int n = 1;

	JSC_loff_t block_addr;
	for (blk_index = 0; blk_index < counter; blk_index++)
	{
		AZX_LOG_INFO("----inside for---\r\n");

		//just for test--------
		if (blk_index >  (n * (counter/10)))
		{
			AZX_LOG_INFO("%d of 10\r\n", n);
			n++;
		}
		///////////------------

		//calculate the address of the block to erase
		block_addr = blk_index << spiFlashContext->block_shift;

//			AZX_LOG_INFO("Address of the blocks: %" PRId64 "\r\n", block_addr);
//			AZX_LOG_INFO("Address of the blocks: %lld \r\n", block_addr);


		ret = azx_spi_flash_erase(spiFlashContext, block_addr, spiFlashContext->block_size);
		if (ret != AZX_SPI_FLASH_CODE_PASS)
		{
			if (ret == AZX_SPI_FLASH_CODE_EIO)
			{
				/*Erase protected block*/
				AZX_LOG_INFO("Block %lld : runtime erase FAIL, skip program\r\n", blk_index);
				continue;
			}
			else
			{
				AZX_LOG_ERROR("Block %lld : erase operation ERROR, test finished\r\n", blk_index);
				return LFS2_ERR_GENERIC;
			}
		}
	}


	return LFS2_ERR_OK;

}
/***************** azx_lfs_flashLowLevelFormat ***********************************************/


