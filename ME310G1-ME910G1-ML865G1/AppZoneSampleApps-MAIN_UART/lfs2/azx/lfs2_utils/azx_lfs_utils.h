/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
  	  azx_lfs_utils.h

  @brief
  	  Project: SPI data flash

  @details

  @version
  	  1.0.0

  @note

  @author
  	  Norman Argiolas

  @dte
  	  File created on: Sep 10, 2020
*/


#ifndef AZX_LFS_FLASH_UTILS_H_
#define AZX_LFS_FLASH_UTILS_H_

/* Include files ================================================================================*/
#include "m2mb_types.h"

#include "lfs2.h"
#include "azx_spi_flash_jsc.h"


/* Global variables definition =============================================================================*/

/**\name AZX_LFS_STATUS_UTILS struct
 * \brief Partition information status.
 *  @{ */
typedef struct
{
	BOOLEAN isInit;		/**<  LFS flash initialization state */
	BOOLEAN isMount;	/**<  LFS partition mounting state */
} AZX_LFS_STATUS_UTILS;
/** @} */


/**\name AZX_LFS_INFO_FILE_E struct
 * \brief File info list structure.
 *  @{ */
typedef struct AZX_LFS_INFO_FILE
{
	struct lfs2_info info;			/**<  LFS file info structure */
	struct AZX_LFS_INFO_FILE *next;     /**<  Pointer to the next element */

} AZX_LFS_INFO_FILE_E;
/** @} */


/**\name AZX_LFS_CONTEXT_TYPE_E enum
 * \brief LFS context type.
 *  @{ */
typedef enum
{
	AZX_LFSCONTEXT_FLASH	= 0,	/**< Flash file system */

	AZX_LFSCONTEXT_RAM 		= 1,	/**< RAM file system */

} AZX_LFS_CONTEXT_TYPE_E;
/** @} */


/**\name AZX_LFS_CONTEXT_E struct
 * \brief File system context structure.
 *  @{ */
typedef struct AZX_LFS_CONTEXT
{
	struct lfs2_config* context;			/**<  Configuration provided during initialization of the littlefs */
	AZX_LFS_CONTEXT_TYPE_E type;     	/**<  File system typey */

} AZX_LFS_CONTEXT_S;
/** @} */


/**\name AZX_RAM_DISK_S struct
 * \brief Ram disk property structure.
 *  @{ */
typedef struct AZX_RAM_DISK_S
{
	uint32_t read_size;
	uint32_t prog_size;
	uint32_t block_size;
	uint32_t block_count;
	int32_t  block_cycles;
	uint32_t cache_size;
} AZX_RAM_DISK_S;
/** @} */


/**\name AZX_LFSMOUNT_MODE_E enum
 * \brief Mount flags configuration.
 *  @{ */
typedef enum
{
	AZX_LFSMOUNT_MODE_MOUNT		 			= 0,	/**<  Mount and exit if error */

	AZX_LFSMOUNT_MODE_MOUNT_OR_FORMAT 		= 1,	/**<  Mount and format if error */

	AZX_LFSMOUNT_MODE_FORMAT_AND_MOUNT 		= 2,		/**<  Format and mount */

	AZX_LFSMOUNT_MODE_LOW_FORMAT_AND_MOUNT 	= 3		/**<  Format and mount */

} AZX_LFSMOUNT_MODE_E;
/** @} */


/**\name AZX_LFSWRITE_MODE_E enum
 * \brief Create file flags configuration.
 *  @{ */
typedef enum
{
	AZX_LFSWRITE_MODE_CREAT 	= 0,	/**<  Create a file if it does not exist or overwite if exists */

	AZX_LFSWRITE_MODE_EXCL 	= 1,		/**<  Fail if a file already exists */

	AZX_LFSWRITE_MODE_APPEND 	= 2		/**<  Move to end of file on every write */
} AZX_LFSWRITE_MODE_E;
/** @} */

/**\name AZX_LFSDIR_MODE_E enum
 * \brief Create directory flags configuration.
 *  @{ */
typedef enum
{
	AZX_LFSDIR_MODE_CREAT 	= 0,	/**<  Create a directory if it does not exist or delete and override if exists */

	AZX_LFSDIR_MODE_EXCL 	= 1,	/**<  Fail if a directory already exists */

	AZX_LFSDIR_MODE_APPEND 	= 2	 	/**<  Create a directory, if it already exist returns success without override*/
} AZX_LFSDIR_MODE_E;
/** @} */

/**\name AZX_LFSRMDIR_MODE_E enum
 * \brief Remove directory flags configuration.
 *  @{ */
typedef enum
{
	AZX_LFSRMDIR_MODE_EMPTY = 0,	/**<  Remove a directory only if it is empty */

	AZX_LFSRMDIR_MODE_RM 	= 1,	/**<  Remove a directory olso if it is not empty*/
} AZX_LFSRMDIR_MODE_E;
/** @} */

/* Global function prototypes ====================================================================*/


#if 0
/* ====== Standalone functions ====== */
LFS2_ERROR_T azx_lfs_flashMount					(AZX_LFSMOUNT_MODE_E mode);
LFS2_ERROR_T azx_lfs_flashUmount				(void);
LFS2_ERROR_T azx_lfs_flashListsContent			(char *path, AZX_LFS_INFO_FILE_E **content_ptr);
LFS2_ERROR_T azx_lfs_flashWrite					(char *path, UINT32 size, UINT8 *source, UINT8 mode);
LFS2_ERROR_T azx_lfs_flashRead					(char *path, UINT32 size, UINT8 *buffer);
LFS2_ERROR_T azx_lfs_flashRmFile				(char *path);
/* ====== Context functions ====== */





AZX_SPI_FLASH_CODE_RESULT_E setFlashContextByContext(AZX_SPI_FLASH_MICRO_SPI_PARAMS *spiFlashOpts,
													 AZX_SPI_FLASH_SPI_CHIP **spiFlashContext);


LFS2_ERROR_T lfsInitByContext (AZX_SPI_FLASH_SPI_CHIP *spiFlashContext,
							   struct lfs2_config **lfsContext);


LFS2_ERROR_T azx_lfs_flashMountByContext(AZX_SPI_FLASH_MICRO_SPI_PARAMS *spiFlashOpts,
										 AZX_SPI_FLASH_SPI_CHIP *spiFlashContext,
										 struct lfs2_config *lfsContext,
										 lfs2_t *lfsObject,
										 AZX_LFSMOUNT_MODE_E mode,
										 AZX_LFS_STATUS_UTILS *lfsStatus);
#endif




LFS2_ERROR_T flashLowLevelFormatByContext (AZX_SPI_FLASH_SPI_CHIP *spiFlashContext);


/*-----------------------------------------------------------------------------------------------*/
/*!
  @brief
	azx_lfsRmFileByContext removes a file

  @details
	Removes the file from the selected path.

  @note

  @param[in] lfsObject
	lfs2_t *lfsObject: pointer to LFS object

  @param[in] path
    char *path: the path where list the contents complete of the path and the file name

  @return
	azx_lfsRmFileByContext() returns LFS2_ERR_OK if there isn't error.
	Otherwise it returns AZX_LFS_ERROR status.

  <b>Sample usage</b>
  @code

	//---Include files here---
	...

	//--- Local function prototypes
	static void nandLFSCallback 			(void *h, AZX_SPI_FLASH_JSC_EVENT_E event, UINT16 resp_size, void *resp_struct, void *userdata );

	static int user_provided_block_device_read  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size);
	static int user_provided_block_device_prog  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size);
	static int user_provided_block_device_erase (const struct lfs2_config *c, lfs2_block_t block);
	static int user_provided_block_device_sync  (const struct lfs2_config *c);

	...

	//---Variables definition
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_JSC_HANDLE  		clbkNandJscHndl;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	static void *context;
	static void *read_buffer;
	static void *prog_buffer;
	static void *lookahead_buffer;
	static lfs2_size_t lookahead_size = 16;
	static int32_t block_cycles 		 = 500;
	static lfs2_size_t block_size 	 = 131072;
	static lfs2_size_t cache_size 	 = 2048;
	static lfs2_size_t prog_size 	 = 2048;
	static lfs2_size_t block_count 	 = 1024;
	static lfs2_size_t read_size 	 = 2048;

	//---Set the context
	opts.clbk.clbkHndl = &clbkNandJscHndl;
	opts.clbk.clbkFunc = nandLFSCallback;

	//--- Ability of SPI Controllers : choose one of x1/x2/x4
	opts.spi.op_mode_rx = SPI_OPM_RX_SING;
	opts.spi.op_mode_tx = SPI_OPM_TX_SING;
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	lfsContext.context = g_chip;
	lfsContext.read = user_provided_block_device_read;
	lfsContext.prog = user_provided_block_device_prog;
	lfsContext.erase =user_provided_block_device_erase;
	lfsContext.sync = user_provided_block_device_sync;
	lfsContext.read_size = read_size;
	lfsContext.prog_size = prog_size;
	lfsContext.block_size = block_size;
	lfsContext.block_count = block_count;
	lfsContext.block_cycles = block_cycles;
	lfsContext.cache_size = cache_size;
	lfsContext.lookahead_size = lookahead_size;
	lfsContext.read_buffer = read_buffer;
	lfsContext.prog_buffer = prog_buffer;
	lfsContext.lookahead_buffer = lookahead_buffer;
	lfsContext.name_max = LFS2_NAME_MAX;
	lfsContext.file_max = LFS2_FILE_MAX;
	lfsContext.attr_max = LFS2_ATTR_MAX;
	lfs2_cfg = &lfsContext;

	//---Functions call
	LFS2_ERROR_T ret;
	ret = lfsMountByContext(AZX_LFSMOUNT_MODE_MOUNT,
							&lfsContext,
							&lfs2);
	if (ret != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("\nError lfsMountByContext memory: err = %d \r\n", ret);
		return TEST_ERROR;
	}
	...

	if (azx_lfsRmFileByContext(&lfs2, "file001.txt") != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfsRmFileByContext failed!!\r\n");
		...
	}

 @endcode
 @ingroup azxLfsUtils
*/
/*-----------------------------------------------------------------------------------------------*/
LFS2_ERROR_T azx_lfsRmFileByContext(lfs2_t *lfsObject, const char *path);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_lfsMountByContext mounts and/or format the partition

 @details
    The function azx_lfsMountByContext provides the filesystem operations. With AZX_LFSMOUNT_MODE_E mode selects if format the partition before mounting it

 @note
 	 Once mounted, the littlefs provides a full set of POSIX-like file and directory functions, with the deviation .
 	 that the allocation of filesystem structures must be provided by the user.
  	 All POSIX operations, such as remove and rename, are atomic, even in event of power-loss. Additionally,
  	 file updates are not actually committed to the filesystem until sync or close is called on the file.

 @param[in] mode
      AZX_LFSMOUNT_MODE_E mode: mode mounting type

 @param[in] c
      const static AZX_LFS_CONTEXT_E *c: pointer to file system context

 @param[in] lfsObject
      lfs2_t *lfsObject: pointer to LFS object


 @return
	lfsMountByContext() returns LFS2_ERR_OK if there isn't error.
	Otherwise it returns AZX_LFS_ERROR status. The function is able to format or/and mount the partition picked out by lfsObject, lfsContext

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

 	//--- Local function prototypes
 	static void nandLFSCallback (void *h, AZX_SPI_FLASH_JSC_EVENT_E event, UINT16 resp_size, void *resp_struct, void *userdata );

	static int user_provided_block_device_read  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size);
	static int user_provided_block_device_prog  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size);
	static int user_provided_block_device_erase (const struct lfs2_config *c, lfs2_block_t block);
	static int user_provided_block_device_sync  (const struct lfs2_config *c);

 	...

	//---Variables definition
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_JSC_HANDLE  		clbkNandJscHndl;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	static void *context;
	static void *read_buffer;
	static void *prog_buffer;
	static void *lookahead_buffer;
	static lfs2_size_t lookahead_size = 16;
	static int32_t block_cycles 		 = 500;
	static lfs2_size_t block_size 	 = 131072;
	static lfs2_size_t cache_size 	 = 2048;
	static lfs2_size_t prog_size 	 = 2048;
	static lfs2_size_t block_count 	 = 1024;
	static lfs2_size_t read_size 	 = 2048;

	//---Set the context
	opts.clbk.clbkHndl = &clbkNandJscHndl;
	opts.clbk.clbkFunc = nandLFSCallback;

	//--- Ability of SPI Controllers : choose one of x1/x2/x4
	opts.spi.op_mode_rx = SPI_OPM_RX_SING;
	opts.spi.op_mode_tx = SPI_OPM_TX_SING;
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	lfsContext.context = g_chip;
	lfsContext.read = user_provided_block_device_read;
	lfsContext.prog = user_provided_block_device_prog;
	lfsContext.erase =user_provided_block_device_erase;
	lfsContext.sync = user_provided_block_device_sync;
	lfsContext.read_size = read_size;
	lfsContext.prog_size = prog_size;
	lfsContext.block_size = block_size;
	lfsContext.block_count = block_count;
	lfsContext.block_cycles = block_cycles;
	lfsContext.cache_size = cache_size;
	lfsContext.lookahead_size = lookahead_size;
	lfsContext.read_buffer = read_buffer;
	lfsContext.prog_buffer = prog_buffer;
	lfsContext.lookahead_buffer = lookahead_buffer;
	lfsContext.name_max = LFS2_NAME_MAX;
	lfsContext.file_max = LFS2_FILE_MAX;
	lfsContext.attr_max = LFS2_ATTR_MAX;
	lfs2_cfg = &lfsContext;

	//set file system context
	static AZX_LFS_CONTEXT_E g_context;
	g_context.context = lfsContext;
	g_context.type = AZX_LFSCONTEXT_FLASH;

	//---Functions call
	LFS2_ERROR_T ret;
	ret = azx_lfsMountByContext(AZX_LFSMOUNT_MODE_MOUNT,
								&g_context,
								&lfs2);
	if (ret != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("\nError lfsMountByContext memory: err = %d \r\n", ret);
		return TEST_ERROR;
	}

	...

 @endcode
 @ingroup azxLfsUtils
*/
/*-----------------------------------------------------------------------------------------------*/
LFS2_ERROR_T azx_lfsMountByContext (AZX_LFSMOUNT_MODE_E mode,
									const AZX_LFS_CONTEXT_S *c,
								    lfs2_t *lfsObject);


/*-----------------------------------------------------------------------------------------------*/
/*!
  @brief
	azx_lfs_flashListsContentByContext List the content of the selected path

  @details
	Lists the content of the selected path specifying the type of the content and the size

  @note

  @param[in] path
	  char *path: the path where list the contents

  @param[in] content_ptr
	  AZX_LFS_INFO_FILE_E **content_ptr: content info struct list

  @param[in] lfsObject
	  lfs2_t *lfsObject: pointer to LFS object

  @return
	azx_lfs_flashListsContentByContext() returns LFS2_ERR_OK if there isn't error.
	Otherwise it returns AZX_LFS_ERROR status.

  <b>Sample usage</b>
  @code

	//---Include files here---
	...

	//--- Local function prototypes
	static void nandLFSCallback 			(void *h, AZX_SPI_FLASH_JSC_EVENT_E event, UINT16 resp_size, void *resp_struct, void *userdata );

	static int user_provided_block_device_read  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size);
	static int user_provided_block_device_prog  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size);
	static int user_provided_block_device_erase (const struct lfs2_config *c, lfs2_block_t block);
	static int user_provided_block_device_sync  (const struct lfs2_config *c);

	...

	//---Variables definition
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_JSC_HANDLE  		clbkNandJscHndl;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	static void *context;
	static void *read_buffer;
	static void *prog_buffer;
	static void *lookahead_buffer;
	static lfs2_size_t lookahead_size = 16;
	static int32_t block_cycles 		 = 500;
	static lfs2_size_t block_size 	 = 131072;
	static lfs2_size_t cache_size 	 = 2048;
	static lfs2_size_t prog_size 	 = 2048;
	static lfs2_size_t block_count 	 = 1024;
	static lfs2_size_t read_size 	 = 2048;

	//---Set the context
	opts.clbk.clbkHndl = &clbkNandJscHndl;
	opts.clbk.clbkFunc = nandLFSCallback;

	//--- Ability of SPI Controllers : choose one of x1/x2/x4
	opts.spi.op_mode_rx = SPI_OPM_RX_SING;
	opts.spi.op_mode_tx = SPI_OPM_TX_SING;
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	lfsContext.context = g_chip;
	lfsContext.read = user_provided_block_device_read;
	lfsContext.prog = user_provided_block_device_prog;
	lfsContext.erase =user_provided_block_device_erase;
	lfsContext.sync = user_provided_block_device_sync;
	lfsContext.read_size = read_size;
	lfsContext.prog_size = prog_size;
	lfsContext.block_size = block_size;
	lfsContext.block_count = block_count;
	lfsContext.block_cycles = block_cycles;
	lfsContext.cache_size = cache_size;
	lfsContext.lookahead_size = lookahead_size;
	lfsContext.read_buffer = read_buffer;
	lfsContext.prog_buffer = prog_buffer;
	lfsContext.lookahead_buffer = lookahead_buffer;
	lfsContext.name_max = LFS2_NAME_MAX;
	lfsContext.file_max = LFS2_FILE_MAX;
	lfsContext.attr_max = LFS2_ATTR_MAX;
	lfs2_cfg = &lfsContext;

	//---Functions call
	LFS2_ERROR_T ret;
	ret = lfsMountByContext(AZX_LFSMOUNT_MODE_MOUNT,
							&lfsContext,
							&lfs2);
	if (ret != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("\nError lfsMountByContext memory: err = %d \r\n", ret);
		return TEST_ERROR;
	}
	...

	AZX_LFS_INFO_FILE_E *listContent = NULL;
	lfs2_dir_t dir;

	if (azx_lfs_flashListsContentByContext("/",
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

		AZX_LOG_INFO("\n\rList:\n\r");
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

 @endcode
 @ingroup azxLfsUtils
*/
/*-----------------------------------------------------------------------------------------------*/
LFS2_ERROR_T azx_lfsListsContentByContext(char *path,
										  AZX_LFS_INFO_FILE_E **content_ptr,
										  lfs2_t *lfsObject);


 /*-----------------------------------------------------------------------------------------------*/
 /*!
   @brief
 	azx_lfsWriteByContext writes a file

   @details
 	Writes the file and its content into the selected path.

   @note

   @param[in] path
 	  char *path: the path where list the contents complete of the path and the file name

   @param[in] size
 	  UINT32 size: size in byte of the content to write

   @param[in] source
 	  UINT8 *source: content to write

   @param[in] mode
 	  UINT8 mode: writing configuration options

  @param[in] lfsObject
	  lfs2_t *lfsObject: pointer to LFS object

  @param[in] lfsFileObject
	  lfs2_file_t *lfsFileObject: lfs file object

  @param[in] context
	  const AZX_LFS_CONTEXT_E *context: context object

   @return
 	azx_lfs_flashListsContentByContext() returns LFS2_ERR_OK if there isn't error.
 	Otherwise it returns AZX_LFS_ERROR status.

   <b>Sample usage</b>
   @code

 	//---Include files here---
 	...

 	//--- Local function prototypes
 	static void nandLFSCallback 			(void *h, AZX_SPI_FLASH_JSC_EVENT_E event, UINT16 resp_size, void *resp_struct, void *userdata );

 	static int user_provided_block_device_read  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size);
 	static int user_provided_block_device_prog  (const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size);
 	static int user_provided_block_device_erase (const struct lfs2_config *c, lfs2_block_t block);
 	static int user_provided_block_device_sync  (const struct lfs2_config *c);

 	...

 	//---Variables definition
 	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
 	static AZX_SPI_FLASH_JSC_HANDLE  		clbkNandJscHndl;
 	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
 	static void *context;
 	static void *read_buffer;
 	static void *prog_buffer;
 	static void *lookahead_buffer;
 	static lfs2_size_t lookahead_size = 16;
 	static int32_t block_cycles 		 = 500;
 	static lfs2_size_t block_size 	 = 131072;
 	static lfs2_size_t cache_size 	 = 2048;
 	static lfs2_size_t prog_size 	 = 2048;
 	static lfs2_size_t block_count 	 = 1024;
 	static lfs2_size_t read_size 	 = 2048;

 	//---Set the context
 	opts.clbk.clbkHndl = &clbkNandJscHndl;
 	opts.clbk.clbkFunc = nandLFSCallback;

 	//--- Ability of SPI Controllers : choose one of x1/x2/x4
 	opts.spi.op_mode_rx = SPI_OPM_RX_SING;
 	opts.spi.op_mode_tx = SPI_OPM_TX_SING;
 	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
 	{
 		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
 		return TEST_ERROR;
 	}
 	lfsContext.context = g_chip;
 	lfsContext.read = user_provided_block_device_read;
 	lfsContext.prog = user_provided_block_device_prog;
 	lfsContext.erase =user_provided_block_device_erase;
 	lfsContext.sync = user_provided_block_device_sync;
 	lfsContext.read_size = read_size;
 	lfsContext.prog_size = prog_size;
 	lfsContext.block_size = block_size;
 	lfsContext.block_count = block_count;
 	lfsContext.block_cycles = block_cycles;
 	lfsContext.cache_size = cache_size;
 	lfsContext.lookahead_size = lookahead_size;
 	lfsContext.read_buffer = read_buffer;
 	lfsContext.prog_buffer = prog_buffer;
 	lfsContext.lookahead_buffer = lookahead_buffer;
 	lfsContext.name_max = LFS2_NAME_MAX;
 	lfsContext.file_max = LFS2_FILE_MAX;
 	lfsContext.attr_max = LFS2_ATTR_MAX;
 	lfs2_cfg = &lfsContext;

 	AZX_LFS_CONTEXT_E g_context;
 	//set file system context
	g_context.context = lfsContext;
	g_context.type = AZX_LFSCONTEXT_FLASH;


 	//---Functions call
 	LFS2_ERROR_T ret;
 	ret = lfsMountByContext(AZX_LFSMOUNT_MODE_MOUNT,
 							&lfsContext,
 							&lfs2);
 	if (ret != LFS2_ERR_OK)
 	{
 		AZX_LOG_ERROR("\nError lfsMountByContext memory: err = %d \r\n", ret);
 		return TEST_ERROR;
 	}
 	...

	char content [10] = {0};
	memset(content, 0x00, sizeof(content));
	strcat(content, "content000");

	if (azx_lfsWriteByContext("file000.txt",
									strlen(content),
									(UINT8*) content,
									AZX_LFSWRITE_MODE_CREAT,
									&lfs2,
									&g_context
									) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("fileCreationUtils failed!!\r\n");
		...
	}

  @endcode
  @ingroup azxLfsUtils
 */
 /*-----------------------------------------------------------------------------------------------*/
LFS2_ERROR_T azx_lfsWriteByContext(char *path,
										 UINT32 size,
										 UINT8 *source,
										 AZX_LFSWRITE_MODE_E mode,
										 lfs2_t *lfsObject,
										 const AZX_LFS_CONTEXT_S *context);



/*-----------------------------------------------------------------------------------------------*/
/*!
    @brief
   	 azx_ram_initialize inizializes ram memory

    @details
   	 Inizializes the memory struct and disk for the Ram memory file system.

     @note

     @param[in] ram
   	  AZX_RAM_DISK_S* ram: contains ram struct information

     @param[in] disk
   	  char*** disk: contains the real disk memory space

	 @param[in] read_size
	  uint32_t read_size: read_size

	 @param[in] prog_size
	  uint32_t prog_size: prog_size

	 @param[in] block_size
	  uint32_t block_size: block_size

	 @param[in] block_count
 	  uint32_t block_count: block_count

	 @param[in] block_cycles
	  int32_t  block_cycles: block_cycles

	 @param[in] cache_size
	  uint32_t cache_size: cache_size

     @return
   	  azx_ram_initialize() returns LFS2_ERR_OK if there isn't error.
   	  Otherwise it returns AZX_LFS_ERROR status.

     <b>Sample usage</b>
     @code

   	//---Include files here---
   	...

   	static AZX_RAM_DISK_S* g_ram;
   	static char** RAM_Disk;

   	...
   	static void *read_buffer;
	static void *prog_buffer;
	static void *lookahead_buffer;
	static lfs2_size_t read_size = 2048;
	static lfs2_size_t prog_size = 2048;
	static int32_t block_cycles = 500;
	static lfs2_size_t cache_size = 2048;
	static lfs2_size_t lookahead_size = 16;
	static const lfs2_size_t block_size = 16384;
	static const lfs2_size_t block_count = 16
	...

   	//ram initialization with memory allocation
  	if (azx_ram_initialize(&g_ram, &RAM_Disk,
  			read_size, prog_size, block_size, block_count, block_cycles, cache_size) != LFS2_ERR_OK)
  	{
  		AZX_LOG_ERROR("Ram Initialization failed!!\r\n");
  		return TEST_ERROR;
  	}

  	...
   	//free memory
  	if (azx_ram_releaseResources(g_ram, RAM_Disk) != LFS2_ERR_OK)
  	{
  		AZX_LOG_ERROR("Release Ram memory failed!!\r\n");
  		return TEST_ERROR;
  	}

    @endcode
    @ingroup azxLfsUtils
*/
/*-----------------------------------------------------------------------------------------------*/

LFS2_ERROR_T azx_ram_initialize(AZX_RAM_DISK_S** ram,
								char*** disk,
								uint32_t read_size,
								uint32_t prog_size,
								uint32_t block_size,
								uint32_t block_count,
								int32_t  block_cycles,
								uint32_t cache_size);

/*-----------------------------------------------------------------------------------------------*/
 /*!
   @brief
 	azx_ram_releaseResources release memory

   @details
 	Release the memory allocated during the initialization.

   @note

   @param[in] ptr_ram
 	  AZX_RAM_DISK_S* ptr_ram: contains ram struct information

   @param[in] disk
 	  char** disk: contains the real disk memory space

   @return
 	azx_ram_releaseResources() returns LFS2_ERR_OK if there isn't error.
 	Otherwise it returns AZX_LFS_ERROR status.

   <b>Sample usage</b>
   @code

 	//---Include files here---
 	...

 	static AZX_RAM_DISK_S* g_ram;
 	static char** RAM_Disk;
 	...

 	//ram initialization with memory allocation
	if (azx_ram_initialize(&g_ram, &RAM_Disk,
			read_size, prog_size, block_size, block_count, block_cycles, cache_size) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("Ram Initialization failed!!\r\n");
		return TEST_ERROR;
	}

	...
 	//free memory
	if (azx_ram_releaseResources(g_ram, RAM_Disk) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("Release Ram memory failed!!\r\n");
		return TEST_ERROR;
	}

  @endcode
  @ingroup azxLfsUtils
 */
 /*-----------------------------------------------------------------------------------------------*/
LFS2_ERROR_T azx_ram_releaseResources(AZX_RAM_DISK_S* ptr_ram, char** disk);


/*-----------------------------------------------------------------------------------------------*/
 /*!
   @brief
 	azx_lfsReadByContext read a file

   @details
 	Read a specific file.

   @note

   @param[in] name
 	  const char *name: the name of the file to read

   @param[in] buffer
 	  char* buffer: contains the bytes read

   @param[in] size
 	  unsigned int size: number of bytes to read

   @param[in] lfsObject
   	  lfs2_t *lfsObject: file system object

   @return
 	azx_lfsReadByContext() returns LFS2_ERR_OK if there isn't error.
 	Otherwise it returns AZX_LFS_ERROR status.

   <b>Sample usage</b>
   @code

 	//---Include files here---

 	...
	//---Initialization
	static lfs2_t lfs2;
	...

 	AZX_LOG_INFO("------>File reading\n\r");
	char buffer[11];
	unsigned int size = 10;
 	memset(buffer, 0x00, sizeof(buffer));
	if (azx_lfsReadByContext("file002.txt",
			  	  	  	  	buffer,
							size,
							&lfs2) != LFS2_ERR_OK)
	{
		AZX_LOG_ERROR("azx_lfsReadByContext failed!!\r\n\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("\nFile: %s, Size: %d, Buffer: %s\r\n", "file002.txt", size, buffer);


  @endcode
  @ingroup azxLfsUtils
 */
 /*-----------------------------------------------------------------------------------------------*/

LFS2_ERROR_T azx_lfsReadByContext(const char *name,
								  char* buffer,
								  unsigned int size,
								  lfs2_t *lfsObject);

/*-----------------------------------------------------------------------------------------------*/
 /*!
   @brief
 	azx_lfsDirCreationByContext create a directory

   @details
 	Create a directory.

   @note

   @param[in] dir
 	  const char *dir: the name of the directory to create

   @param[in] mode
 	  AZX_LFSDIR_MODE_E mode: creation mode, see AZX_LFSDIR_MODE_E

   @param[in] size
 	  unsigned int size: number of bytes to read

   @param[in] lfsObject
   	  lfs2_t *lfsObject: file system object

   @return
 	azx_lfsReadByContext() returns LFS2_ERR_OK if there isn't error.
 	Otherwise it returns AZX_LFS_ERROR status.

   <b>Sample usage</b>
   @code

 	//---Include files here---

 	...
	//---Initialization
	static lfs2_t lfs2;
	...

 	if (azx_lfsDirCreationByContext("dir000",
									AZX_LFSDIR_MODE_APPEND,
									&lfs2) != TEST_OK)
	{
		AZX_LOG_ERROR("directoryCreation failed!!\r\n");
		return TEST_ERROR;
	}

  @endcode
  @ingroup azxLfsUtils
 */
 /*-----------------------------------------------------------------------------------------------*/
LFS2_ERROR_T azx_lfsDirCreationByContext(const char *dir,
										 AZX_LFSDIR_MODE_E mode,
										 lfs2_t *lfsObject);


//todo doxy doc
LFS2_ERROR_T azx_lfsDirRmByContext(const char *dir,
								   AZX_LFSRMDIR_MODE_E mode,
								   lfs2_t *lfsObject);


#endif /* AZX_LFS_FLASH_UTILS_H_ */
