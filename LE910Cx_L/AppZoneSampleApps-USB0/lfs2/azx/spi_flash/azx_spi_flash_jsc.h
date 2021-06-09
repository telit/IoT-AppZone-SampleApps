/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
 @file
 	 azx_spi_flash_jsc.h

 @brief
 	 Contains the Software Drivers for SPI NAND FLASH JSC JS28(P_U)xGQSxHG-83 family.

 @details
 	 The file contains interface, return codes and macro for
     the Software Drivers for SPI NAND FLASH JSC JS28(P_U)xGQSxHG-83 family.

 @version
 	 1.0.0

 @note

 @author
 	 Norman Argiolas
 	 Fabio Pintus

 @date
 	 File created on: Feb 24, 2020

 */


#ifndef AZX_SPI_FLASH_JSC_H_
#define AZX_SPI_FLASH_JSC_H_

/* Include files ================================================================================*/
#include "azx_spi_flash_hw_external.h"
#include "azx_spi_flash_types.h"


/* Global defines ================================================================================*/

/* set timeout to 10 seconds for waitForReady operation*/
#define SPI_FLASH_WAITING_TIMEOUT		10000

/*	set delay time to perform flash operation, in milliseconds*/
#define FLASH_DELAY_FOR_READY			150

/* Global typedef ==============================================================================*/
typedef JSC_HANDLE AZX_SPI_FLASH_JSC_HANDLE;

/* free oob descrption */
typedef struct {
	JSC_uint32	offset;
	JSC_uint32	length;
} AZX_SPI_FLASH_OOBFREE;

/* ECC layout description */
typedef struct {
	JSC_uint32				eccbytes;
    JSC_uint32      		eccpos[680];
    JSC_uint32      		oobavail;
    AZX_SPI_FLASH_OOBFREE 	oobfree[4];
} AZX_SPI_FLASH_ECCLAYOUT;

typedef enum
{
  AZX_SPI_FLASH_INITIALIZED = 1,
  AZX_SPI_FLASH_SPI_INITIALIZED,
  AZX_SPI_FLASH_GPIO_INITIALIZED,
  AZX_SPI_FLASH_RESETTED,
  AZX_SPI_FLASH_JSC_CONFIGURED
} AZX_SPI_FLASH_JSC_EVENT_E;

typedef void (*AZX_SPI_FLASH_CLBK_FUNCTION ) (AZX_SPI_FLASH_JSC_HANDLE h, AZX_SPI_FLASH_JSC_EVENT_E event, JSC_uint16 resp_size, void *resp_struct, void *userdata );

typedef struct {
	AZX_SPI_FLASH_JSC_HANDLE	  	clbkHndl;
	AZX_SPI_FLASH_CLBK_FUNCTION 	clbkFunc;
} AZX_SPI_FLASH_EVENT_CALLBACK;

typedef struct {
	AZX_SPI_FLASH_EVENT_CALLBACK   	clbk;	//event callback function
	AZX_SPI_FLASH_SPI_SLAVE  		spi; 	//TODO to delete
	AZX_SPI_FLASH_DESCRIPTOR 		fd;		//spi/gpio file descriptor
	JSC_bool 						init;	//if init=FALSE nand not initialized
} AZX_SPI_FLASH_MICRO_SPI_PARAMS;

typedef struct
{
  AZX_SPI_FLASH_CODE_RESULT_E  		result;
  AZX_SPI_FLASH_MICRO_SPI_PARAMS	opts;
} AZX_SPI_FLASH_JSC_EVENT_RES_T;


/**
 * struct spi_nand_chip - SPI-NAND Private Flash Chip Data
 * @name:                       name of the chip
 * @spi:                        [INTERN] point to spi device structure
 * @mfr_id:                     [BOARDSPECIFIC] manufacture id
 * @dev_id:                     [BOARDSPECIFIC] device id
 * @dev_id2:                    [BOARDSPECIFIC] device id2
 * @read_cache_op:              [REPLACEABLE] Opcode of read from cache
 * @write_cache_op:             [REPLACEABLE] Opcode of program load
 * @write_cache_rdm_op:         [REPLACEABLE] Opcode of program load random
 * @oobbuf:                     [INTERN] buffer for read/write oob
 * @size:                       [INTERN] the size of chip
 * @block_size:                 [INTERN] the size of eraseblock
 * @page_size:                  [INTERN] the size of page
 * @oob_size:                   [INTERN] the size of page oob size
 * @block_shift:                [INTERN] number of address bits in a eraseblock
 * @page_shift:                 [INTERN] number of address bits in a page (column address bits).
 * @pagemask:                   [INTERN] page number mask = number of (pages / chip) - 1
 * @ecc_strength:               [INTERN] ECC correctability
 * @refresh_threshold:          [INTERN] Bitflip threshold to return -EUCLEAN
 * @max_bad_blks_per_lun:       [INTERN] maximum number of invalid blocks
 * @otp_block:                  [INTERN] otp block address
 * @otp_page_start:             [INTERN] otp page start
 * @otp_page_number:            [INTERN] number of otp page
 * @ecclayout:                  [BOARDSPECIFIC] ECC layout control structure
 *                              See the defines for further explanation.
 * @onfi_params:                [INTERN] holds the ONFI page parameter
 */
typedef struct {
    char                        	*name;
    AZX_SPI_FLASH_SPI_SLAVE        	*spi;		//TODO change name becose is the handle of gpio and spi
    AZX_SPI_FLASH_MICRO_SPI_PARAMS	*opts;		//TODO remove one spi pointer or from SPI_NAND_CHIP or from MICRO_SPI_PARAMS

    JSC_bool                    vcc_33;
    JSC_uint8                   mfr_id;
    JSC_uint8                   dev_id;
    JSC_uint8                   dev_id2;
    JSC_uint8                   read_cache_op;
    JSC_uint8                   write_cache_op;
    JSC_uint8                   write_cache_rdm_op;

    JSC_uint8                   *datbuf;
    JSC_uint8                   *oobbuf;
    JSC_uint64                  size;
    JSC_uint32                  block_size;
    JSC_uint16                  page_size;
    JSC_uint16                  oob_size;
    JSC_uint8                   block_shift;
    JSC_uint8                   page_shift;
    JSC_uint16                  page_mask;
    JSC_uint32                  ecc_strength;
    JSC_uint8                   refresh_threshold;
    JSC_uint32                  max_bad_blks_per_lun;
    JSC_uint32                  otp_block;
    JSC_uint32                  otp_page_start;
    JSC_uint32                  otp_page_number;

    AZX_SPI_FLASH_ECCLAYOUT   		*ecclayout;
} AZX_SPI_FLASH_SPI_CHIP;


/**
 * oob operation modes
 *
 * @MTD_OPS_PLACE_OOB:  OOB data are placed at the given offset (default)
 * @MTD_OPS_AUTO_OOB:   OOB data are automatically placed at the free areas
 *                      which are defined by the internal ecclayout
 * @MTD_OPS_RAW:        data are transferred as-is, with no error correction;
 *                      this mode implies %MTD_OPS_PLACE_OOB
 */
enum {
    MTD_OPS_PLACE_OOB           = 0,
    MTD_OPS_AUTO_OOB            = 1,
    MTD_OPS_RAW                 = 2,
};


/**
 * struct MTD_OOB_OPS - oob operation operands
 * @mode:       operation mode
 * @len:        number of data bytes to write/read
 * @retlen:     number of data bytes written/read
 * @ooblen:     number of oob bytes to write/read
 * @oobretlen:  number of oob bytes written/read
 * @ooboffs:    offset of oob data in the oob area (only relevant when mode = MTD_OPS_PLACE_OOB or
 *              MTD_OPS_RAW)
 * @datbuf:     data buffer - if NULL only oob data are read/written
 * @oobbuf:     oob data buffer
 *
 * Note, it is allowed to read more than one OOB area at one go, but not write.
 * The interface assumes that the OOB write requests program only one page's
 * OOB area.
 */
typedef struct {
	JSC_uint32	mode;
	JSC_size_t	len;
    JSC_size_t  retlen;
    JSC_size_t  ooblen;
    JSC_size_t  oobretlen;
    JSC_uint32  ooboffs;
    JSC_uint8   *datbuf;
    JSC_uint8   *oobbuf;
} AZX_SPI_FLASH_MTD_OOB_OPS;


typedef struct __attribute__((packed)) {
    /* rev info and features block */
    /* 'O' 'N' 'F' 'I'  */
	JSC_uint8		sig[4];                     /*<0-3*/
	JSC_uint16      revision;                   /*4-5*/
	JSC_uint16      features;                   /*6-7*/
	JSC_uint16      opt_cmd;                    /*8-9*/
    JSC_uint8       reserved0[22];              /*10-31*/

    /* manufacturer information block */
    char            manufacturer[12];           /*32-43*/
    char            model[20];                  /*44-63*/
    JSC_uint8       mfr_id;                     /*64*/
    JSC_uint16      date_code;                  /*65-66*/
    JSC_uint8       reserved1[13];              /*67-79*/

    /* memory organization block */
    JSC_uint32      byte_per_page;              /*80-83*/
    JSC_uint16      spare_bytes_per_page;       /*84*85*/
    JSC_uint32      data_bytes_per_ppage;       /*86-89*/
    JSC_uint16      spare_bytes_per_ppage;      /*90-91*/
    JSC_uint32      pages_per_block;            /*92-95*/
    JSC_uint32      blocks_per_lun;             /*96-99*/
    JSC_uint8       lun_count;                  /*100*/
    JSC_uint8       addr_cycles;                /*101*/
    JSC_uint8       bits_per_cell;              /*102*/
    JSC_uint16      bb_per_lun;                 /*103-104*/
    JSC_uint16      block_endurance;            /*105-106*/
    JSC_uint8       guaranteed_good_blocks;     /*107*/
    JSC_uint16      guaranteed_block_endurance; /*108-109*/
    JSC_uint8       programs_per_page;          /*110*/
    JSC_uint8       page_attr;                  /*111*/
    JSC_uint8       ecc_bits;                   /*112*/
    JSC_uint8       interleaved_bits;           /*113*/
    JSC_uint8       interleaved_ops;            /*114*/
    JSC_uint8       reserved2[13];              /*115-127*/

    /* electrical parameter block */
    JSC_uint8       io_pin_capacitance_max;     /*128*/
    JSC_uint16      timing_mode;                /*129-130*/
    JSC_uint16      program_cache_timing_mode;  /*131-132*/
    JSC_uint16      t_prog;                     /*133-134*/
    JSC_uint16      t_bers;                     /*135-136*/
    JSC_uint16      t_r;                        /*137-138*/
    JSC_uint16      t_ccs;                      /*139-140*/
    JSC_uint8       reserved3[23];   /*141-163*/

    /* vendor */
    JSC_uint16      vendor_specific_revision;   /*164-165*/
    JSC_uint8       vendor_specific[88];        /*166-253*/

    JSC_uint16      crc;                        /*254-255*/
} AZX_SPI_FLASH_ONFI_PARAMS;



/* block lock register defines
* Register Bit Address
*   | 7    | 6        | 5   | 4   | 3   | 2   | 1   | 0        |
*   |----------------------------------------------------------|
*   | BRWD | Reversed | BP2 | BP1 | BP0 | INV | CMP | Reversed |
*
* Block Lock Register Block Protection Bits
*/
#define LOCK_CMP                                    (0x02)
#define LOCK_INV                                    (0x04)
#define LOCK_BP0                                    (0x08)
#define LOCK_BP1                                    (0x10)
#define LOCK_BP2                                    (0x20)

                                                                                    /* CMP INV BP2 BP1 BP0  Protected Portion*/
#define BL_ALL_UNLOCKED     (0x00)                                                  /* x   x   0   0   0    All unlocked */
#define BL_U_1_64_LOCKED    (LOCK_BP0)                                              /* 0   0   0   0   1    Upper 1/64 locked */
#define BL_U_1_32_LOCKED    (LOCK_BP1)                                              /* 0   0   0   1   0    Upper 1/32 locked */
#define BL_U_1_16_LOCKED    (LOCK_BP1 | LOCK_BP0)                                   /* 0   0   0   1   1    Upper 1/16 locked */
#define BL_U_1_8_LOCKED     (LOCK_BP2)                                              /* 0   0   1   0   0    Upper 1/8 locked */
#define BL_U_1_4_LOCKED     (LOCK_BP2 | LOCK_BP0)                                   /* 0   0   1   0   1    Upper 1/4 locked */
#define BL_U_1_2_LOCKED     (LOCK_BP2 | LOCK_BP1)                                   /* 0   0   1   1   0    Upper 1/2 locked */
#define BL_ALL_LOCKED       (LOCK_CMP | LOCK_INV | LOCK_BP2 | LOCK_BP1 | LOCK_BP0)  /* x   x   1   1   1    All Locked (default) */
#define BL_L_1_64_LOCKED    (LOCK_INV | LOCK_BP0)                                   /* 0   1   0   0   1    Lower 1/64 locked */
#define BL_L_1_32_LOCKED    (LOCK_INV | LOCK_BP1)                                   /* 0   1   0   1   0    Lower 1/32 locked */
#define BL_L_1_16_LOCKED    (LOCK_INV | LOCK_BP1 | LOCK_BP0)                        /* 0   1   0   1   1    Lower 1/16 locked */
#define BL_L_1_8_LOCKED     (LOCK_INV | LOCK_BP2)                                   /* 0   1   1   0   0    Lower 1/8 locked */
#define BL_L_1_4_LOCKED     (LOCK_INV | LOCK_BP2 | LOCK_BP0)                        /* 0   1   1   0   1    Lower 1/4 locked */
#define BL_L_1_2_LOCKED     (LOCK_INV | LOCK_BP2 | LOCK_BP1)                        /* 0   1   1   1   0    Lower 1/2 locked */
#define BL_L_63_64_LOCKED   (LOCK_CMP | LOCK_BP0)                                   /* 1   0   0   0   1    Lower 63/64 locked */
#define BL_L_31_32_LOCKED   (LOCK_CMP | LOCK_BP1)                                   /* 1   0   0   1   0    Lower 31/32 locked */
#define BL_L_15_16_LOCKED   (LOCK_CMP | LOCK_BP1 | LOCK_BP0)                        /* 1   0   0   1   1    Lower 15/16 locked */
#define BL_L_7_8_LOCKED     (LOCK_CMP | LOCK_BP2)                                   /* 1   0   1   0   0    Lower 7/8 locked */
#define BL_L_3_4_LOCKED     (LOCK_CMP | LOCK_BP2 | LOCK_BP0)                        /* 1   0   1   0   1    Lower 3/4 locked */
#define BL_BLOCK0_LOCKED    (LOCK_CMP | LOCK_BP2 | LOCK_BP1)                        /* 1   0   1   1   0    Block 0 locked */
#define BL_U_63_64_LOCKED   (LOCK_CMP | LOCK_INV | LOCK_BP0)                        /* 1   1   0   0   1    Upper 63/64 locked */
#define BL_U_31_32_LOCKED   (LOCK_CMP | LOCK_INV | LOCK_BP1)                        /* 1   1   0   1   0    Upper 31/32 locked */
#define BL_U_15_16_LOCKED   (LOCK_CMP | LOCK_INV | LOCK_BP1 | LOCK_BP0)             /* 1   1   0   1   1    Upper 15/16 locked */
#define BL_U_7_8_LOCKED     (LOCK_CMP | LOCK_INV | LOCK_BP2)                        /* 1   1   1   0   0    Upper 7/8 locked */
#define BL_U_3_4_LOCKED     (LOCK_CMP | LOCK_INV | LOCK_BP2 | LOCK_BP0)             /* 1   1   1   0   1    Upper 3/4 locked */
//#define BL_BLOCK0_LOCKED    (LOCK_CMP | LOCK_INV | LOCK_BP2 | LOCK_BP1)             /* 1   1   1   1   0    Block 0 locked */




/****************************************************************************
 	 	 	 	 Memory Tecnology Devise (MTD)
 	 	 	 	 Hardware independent function
 ****************************************************************************/
/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_initialize initializes SPI-FLASH service

 @details
   Initializes the AZX_SPI_FLASH_SPI_CHIP struct before the usage of SPI-NAND device driver and the
   handles for SPI and GPIO configuration

 @note

 @param[in] CS_GPIO_pin
    const JSC_uint8 CS_GPIO_pin

 @param[in] opts
     AZX_SPI_FLASH_MICRO_SPI_PARAMS *opts

 @param[in] chip_ptr
     AZX_SPI_FLASH_SPI_CHIP **chip_ptr


 @return
	azx_spi_flash_initialize() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}

	...
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_initialize(const JSC_uint8 CS_GPIO_pin,
													 AZX_SPI_FLASH_MICRO_SPI_PARAMS *opts,
													 AZX_SPI_FLASH_SPI_CHIP **chip_ptr);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_releaseResources releases resources used with azx_spi_flash_initialize

 @details
   Releases all resources used during the SPI FLASH initialization.
   Releases the SPI and GPIO handles and the memory allocated with the AZX_SPI_FLASH_SPI_CHIP struct.

 @note

 @param[in] CS_GPIO_pin
    const JSC_uint8 CS_GPIO_pin

 @param[in] chip
     AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @return
	azx_spi_flash_releaseResources() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}

	...

	if (azx_spi_flash_releaseResources(g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Release memory failed!!\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("Nand released\r\n");

 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_releaseResources (AZX_SPI_FLASH_SPI_CHIP *chip);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_configure releases resources used with azx_spi_flash_initialize

 @details
	Initializes the handles for SPI and GPIO configuration

 @note
 	TOBE IMPLEMENTED!!!
	The main idea is to decouple the allocation of the memory during the initialization phase and the configuration for SPI and GPIO handles.

 @param[in] opts
    AZX_SPI_FLASH_MICRO_SPI_PARAMS *opts

 @param[in] chip_ptr
     AZX_SPI_FLASH_SPI_CHIP **chip_ptr

 @return
	azx_spi_flash_releaseResources() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E

 <b>Sample usage</b>
 @code

 	//work in progress

 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_configure (AZX_SPI_FLASH_MICRO_SPI_PARAMS *opts,
													 AZX_SPI_FLASH_SPI_CHIP **chip_ptr);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_readID reads the ID of the mounted device

 @details
   Reads the unique ID used to identify the device

 @note

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @param[in] buf
     JSC_uint8 *buf

 @return
	azx_spi_flash_readID() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E. The read ID is stored into the buf variable.


 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	UINT8 id[3] = {0};
	if (azx_spi_flash_readID(g_chip, id) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand ReadID failed!!\r\n");
		return TEST_ERROR;
	}
	AZX_LOG_INFO("SPI-NAND type mfr_id: %x, dev_id: %x, dev_id2: %x is not in id table.\r\n",
		id[0], id[1], id[2]);
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_readID(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *buf);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_lockBlock locks the write operations

 @details
   Locks the selected portion of memory

 @note

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @param[in] lock
     JSC_uint8 lock

 @return
	azx_spi_flash_readID() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E.


 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

    //--- Block protection---
    JSC_uint8 protect_reg;
    for (lp = 0; lp < 25; lp++)
	{
		protect_reg = lp ==  0 ? BL_ALL_UNLOCKED   :
					  lp ==  1 ? BL_U_1_64_LOCKED  :
					  lp ==  2 ? BL_U_1_32_LOCKED  :
					  lp ==  3 ? BL_U_1_16_LOCKED  :
					  lp ==  4 ? BL_U_1_8_LOCKED   :
					  lp ==  5 ? BL_U_1_4_LOCKED   :
					  lp ==  6 ? BL_U_1_2_LOCKED   :
					  lp ==  7 ? BL_ALL_LOCKED     :
					  lp ==  8 ? BL_L_1_64_LOCKED  :
					  lp ==  9 ? BL_L_1_32_LOCKED  :
					  lp == 10 ? BL_L_1_16_LOCKED  :
					  lp == 11 ? BL_L_1_8_LOCKED   :
					  lp == 12 ? BL_L_1_4_LOCKED   :
					  lp == 13 ? BL_L_1_2_LOCKED   :
					  lp == 14 ? BL_L_63_64_LOCKED :
					  lp == 15 ? BL_L_31_32_LOCKED :
					  lp == 16 ? BL_L_15_16_LOCKED :
					  lp == 17 ? BL_L_7_8_LOCKED   :
					  lp == 18 ? BL_L_3_4_LOCKED   :
					  lp == 19 ? BL_BLOCK0_LOCKED  :
					  lp == 20 ? BL_U_63_64_LOCKED :
					  lp == 21 ? BL_U_31_32_LOCKED :
					  lp == 22 ? BL_U_15_16_LOCKED :
					  lp == 23 ? BL_U_7_8_LOCKED   :
							  	 BL_U_3_4_LOCKED;
	AZX_SPI_FLASH_CODE_RESULT_E ret;
    ret = azx_spi_flash_lockBlock(chip, protect_reg);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	AZX_LOG_ERROR(" Invalid lock block protection %x\r\n", protection);
        return AZX_NAND_CODE_FAIL;
    }
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_lockBlock(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 lock);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_enableEcc enables internal ECC.

 @details
   	Enables internal ECC. There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 	Enable chip internal ECC, set the bit to 1
    Disable chip internal ECC, clear the bit to 0

 @note

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @return
	azx_spi_flash_enableEcc() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E.


 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	if (azx_spi_flash_enableEcc(g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand anable ECC failed!!\r\n");
		return TEST_ERROR;
	}
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_enableEcc(AZX_SPI_FLASH_SPI_CHIP *chip);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_disableEcc disables internal ECC.

 @details
   	Disables internal ECC. There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 	Enable chip internal ECC, set the bit to 1
    Disable chip internal ECC, clear the bit to 0

 @note

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @return
	azx_spi_flash_disableEcc() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E.


 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	if (azx_spi_flash_disableEcc(g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand disable ECC failed!!\r\n");
		return TEST_ERROR;
	}
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_disableEcc(AZX_SPI_FLASH_SPI_CHIP *chip);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_callbak calls callback function.

 @details
   	Calls callback function with a specific event

 @note

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @param[in] chip
    AZX_SPI_FLASH_JSC_EVENT_E event

 @return
	azx_spi_flash_callbak() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E.

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	if (azx_spi_flash_callbak(g_chip, AZX_SPI_FLASH_INITIALIZED) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand callback failed!!\r\n");
		return TEST_ERROR;
	}
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_callbak(AZX_SPI_FLASH_SPI_CHIP *chip, AZX_SPI_FLASH_JSC_EVENT_E event);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_blockIsBad checks block is bad or not

 @details
   	Checks block is bad or not via read

 @note
	bad block mark(the first two byte in oob area of the first page in the block)
	For a block, read the first page's first two byte of oob data, if data is all
	0xFF, the block is a good block, otherwise it a bad block.

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @param[in] offs
    JSC_loff_t offs: offset from device start

 @param[in] result
    JSC_bool *result: true if is bad block, false otherwise

 @return
	azx_spi_flash_blockIsBad() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E. If success result is true if is bad block, false otherwise

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	int isBad = false;
	JSC_loff_t block_addr;	//set here the address of the block
	...

	//---Scan good block
	ret = azx_spi_flash_blockIsBad(g_chip, block_addr, &isBad);
	if (ret != AZX_SPI_FLASH_CODE_PASS)
	{
		if (ret == AZX_SPI_FLASH_CODE_EBADMSG && isBad == true)
		{
			AZX_LOG_INFO("Block %d : initial bad block, skip erase\r\n", blk);
			//...
		}
		else
		{
			AZX_LOG_ERROR("Checking bad block failed!!\r\n");
			//...
		}
	}
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_blockIsBad(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t offs, JSC_bool *result);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_erase erase block(s)

 @details
   	Erase one ore more blocks of the SPI FLASH memory

 @note
	Erase one ore more blocks
	The command sequence for the BLOCK ERASE operation is as follows:
	       06h (WRITE ENBALE command)
	       D8h (BLOCK ERASE command)
	       0Fh (GET FEATURES command to read the status register)

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @param[in] addr
    JSC_uint64 addr: address that erase start with, should be blocksize aligned

 @param[in] len
    JSC_uint64 len: length that want to be erased, should be blocksize aligned

 @return
	azx_spi_flash_blockIsBad() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E. If success result is true if is bad block, false otherwise

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	int isBad = false;
	JSC_loff_t block_addr;	//set here the address of the block
	...
	JSC_uint64 len; //set here the block size
	...

	//---Scan good block
	ret = ret = azx_spi_flash_erase(chip, block_addr, len);
	if (ret != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand erase failed!!\r\n");
		//...
	}
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_erase(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint64 addr, JSC_uint64 len);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_writeOob write data

 @details
    Writes data and/or out-of-band

 @note

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @param[in] to
    JSC_loff_t to: offset to write to

 @param[in] ops
    AZX_SPI_FLASH_MTD_OOB_OPS *ops: oob operation description structure

 @return
	azx_spi_flash_writeOob() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E.

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	int isBad = false;
	JSC_loff_t block_addr;	//set here the address of the block
	...
	AZX_SPI_FLASH_MTD_OOB_OPS ops;
	UINT8 write_buf_oob[256];
	//set here the configuration
	memset(&ops, 0xFF, sizeof(ops));
    ops.mode        = MTD_OPS_AUTO_OOB;
    ops.len         = size;
    ops.ooblen      = chip->ecclayout->oobavail;
    ops.ooboffs     = 0;
    ops.datbuf      = (JSC_uint8 *) buffer;
    ops.oobbuf      = write_buf_oob;
	...

	//---Scan good block
	ret = ret = azx_spi_flash_writeOob(chip, block_addr, ops);
	if (ret != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand write failed!!\r\n");
		//...
	}
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_writeOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t to, AZX_SPI_FLASH_MTD_OOB_OPS *ops);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_readOob read data

 @details
    Reads data and/or out-of-band

 @note

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @param[in] from
    JSC_loff_t from: offset to read from

 @param[in] ops
    AZX_SPI_FLASH_MTD_OOB_OPS *ops: oob operation description structure

 @return
	azx_spi_flash_readOob() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E.

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	int isBad = false;
	JSC_loff_t block_addr;	//set here the address of the block
	...
	AZX_SPI_FLASH_MTD_OOB_OPS ops;
	UINT8 read_buf_oob[256];
	//set here the configuration
	memset(&ops, 0xFF, sizeof(ops));
	ops.mode		= MTD_OPS_AUTO_OOB;
	ops.len  		= size;
	ops.ooblen      = g_chip->ecclayout->oobavail;
	ops.ooboffs     = 0;
	ops.datbuf      = (JSC_uint8 *) buffer;
	ops.oobbuf      = read_buf_oob;
	...

	//---Scan good block
	ret = ret = azx_spi_flash_readOob(chip, block_addr, ops);
	if (ret != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand write failed!!\r\n");
		//...
	}
 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_readOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t from, AZX_SPI_FLASH_MTD_OOB_OPS *ops);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_readUIDpage read UID page

 @details
    Read UID page from flash to buffer

 @note

 @param[in] chip
    AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @param[in] column
    JSC_uint32 column: column address

 @param[in] buf
    JSC_uint8 *buf: data buffer

 @param[in] len
    JSC_size_t len: data length to read

 @return
	azx_spi_flash_readUIDpage() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E.

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}

	...

	static UINT8 read_buf[4096];
	JSC_uint8 xorJSC;
	memset(read_buf, 0xFF, 4096);

    //---Read UID
	AZX_LOG_INFO("Read UID...\r\n");
    ret = azx_spi_flash_readUIDpage(g_chip, 0, read_buf, g_chip->page_size);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	AZX_LOG_ERROR("UID page: read operation ERROR, test finished\r\n");
        //...
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
        	ret = AZX_NAND_CODE_FAIL;
        }
    }
    AZX_LOG_INFO("\r\n");

    if (ret != AZX_NAND_CODE_PASS)
    {
    	AZX_LOG_ERROR(" unique ID invalid\r\n");
    }
    else
    {
    	AZX_LOG_INFO(" unique ID valid\r\n");
    }

 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_readUIDpage(AZX_SPI_FLASH_SPI_CHIP *chip,
													  JSC_uint32 column,
													  JSC_uint8 *buf,
													  JSC_size_t len);


 /*-----------------------------------------------------------------------------------------------*/
 /*!
  @brief
 	azx_spi_flash_readParameterPage read parameter page

  @details
     Reads parameter page from flash to buffer

  @note

  @param[in] chip
     AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

  @param[in] column
     JSC_uint32 column: column address

  @param[in] buf
    JSC_uint8 *buf: data buffer

  @param[in] len
    JSC_size_t len: data length to read

  @return
 	azx_spi_flash_readParameterPage() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
 	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E.

  <b>Sample usage</b>
  @code

  	//---Include files here---
  	...

 	//---Variables definition
  	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
 	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
 	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
 	...

 	//---Functions call
 	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
 	{
 		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
 		return TEST_ERROR;
 	}
 	...

	UINT8 read_buf[4096];
 	//---Read parameter
 	ret = ret = azx_spi_flash_readParameterPage(g_chip, 0, read_buf, g_chip->page_size);
 	if (ret != AZX_SPI_FLASH_CODE_PASS)
 	{
 		AZX_LOG_ERROR("Nand reading parameters failed!!\r\n");
 		//...
 	}


  @endcode
  @ingroup azxSpiFlash
  */
 /*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_readParameterPage(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 column, JSC_uint8 *buf, JSC_size_t len);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_checkParameter reads information

 @details
    Reads information from the flash memory when we can not get info from id table.

 @note

 @param[in] p
    AZX_SPI_FLASH_ONFI_PARAMS *p

 @return
	azx_spi_flash_checkParameter() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E.

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	UINT8 read_buf[4096];
	//---Read parameter
	ret = ret = azx_spi_flash_readParameterPage(g_chip, 0, read_buf, g_chip->page_size);
	if (ret != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand reading parameters failed!!\r\n");
		//...
	}
	//...

	AZX_NAND_SPI_ONFI_PARAMS *p;
	p = (AZX_SPI_FLASH_ONFI_PARAMS *) &read_buf[0];
  	if (azx_spi_flash_checkParameter(p) != AZX_SPI_FLASH_CODE_PASS)
  	{
  		AZX_LOG_ERROR("parameter page : data invalid, test finished\r\n");
	//...
   	}

 @endcode
 @ingroup azxSpiFlash
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_checkParameter(AZX_SPI_FLASH_ONFI_PARAMS *p);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
	azx_spi_flash_waitForReady wait until the command is done

 @details
    Wait until the command is done

 @note

  @param[in] chip
     AZX_SPI_FLASH_SPI_CHIP *chip: SPI-NAND device structure

 @param[in] s
    JSC_uint8 *s: buffer to store status register(can be NULL)

 @return
	azx_spi_flash_waitForReady() returns AZX_SPI_FLASH_CODE_PASS if there isn't error.
	Otherwise it returns AZX_SPI_FLASH_CODE_RESULT_E. The function save into s the register status

 <b>Sample usage</b>
 @code

 	//---Include files here---
 	...

	//---Variables definition
 	 #define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2 //select the right pin number taking into account own device configuration
	static AZX_SPI_FLASH_MICRO_SPI_PARAMS 	opts;
	static AZX_SPI_FLASH_SPI_CHIP 			*g_chip;
	...

	//---Functions call
	if (azx_spi_flash_initialize(SPI_FLASH_JS28P1GQSCAHG_CS_GPIO, &opts, &g_chip) != AZX_SPI_FLASH_CODE_PASS)
	{
		AZX_LOG_ERROR("Nand Initialization failed!!\r\n");
		return TEST_ERROR;
	}
	...

	JSC_uint8 status;
	ret = azx_spi_flash_waitForReady(chip, &status);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	//an error occur
    	//...
    }
    else
    {
    	//no error occur
    	//...
    }

 @endcode
 @ingroup azxSpiFlash
*/
/*-----------------------------------------------------------------------------------------------*/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_waitForReady(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *s);



/*************** Memory Tecnology Devise (MTD) ***************/


#endif //AZX_SPI_FLASH_JSC_H_
