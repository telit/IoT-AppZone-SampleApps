/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
 @file
   azx_spi_flash_jsc.c

 @brief
 	 Contains the implementation functions for SPI NAND FLASH JSC JS28(P_U)xGQSxHG-83 family.

 @details

 @version
   1.0.1

 @note

 @author
   Norman Argiolas

 @date
 	 File created on: Feb 13, 2020
 */

/* Include files ================================================================================*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "string.h"


#include "azx_spi_flash_jsc.h"

#include "azx_spi_flash_hw.h"
#include "azx_spi_flash_types.h"
#include "azx_spi_flash_strings.h"

#ifndef STM32F446xx
#include "azx_log.h"
#endif
/* Local typedef ==============================================================================*/

/* Local defines ================================================================================*/

#define le16_to_cpu(x)              (x)

#define min(a,b)                    (((a)<(b))?(a):(b))
#define max(a,b)                    (((a)>(b))?(a):(b))

/*FabioPi removed designated initializers for RCVT*/
#define SPI_FLASH_INFO(nm, vcc33, mid, did, did2, pagesz, oobsz, pg_per_blk,                                  \
                      blk_per_lun, max_bad_blk_per_lun, ecc_stren,                                           \
                      OTP_block, OTP_page_start, OTP_page_number)                                            \
                      { (char*)(nm),  (vcc33), (mid), (did), (did2),\
                      (pagesz), (oobsz),                                            \
                      (pg_per_blk), (blk_per_lun),                          \
                      (max_bad_blk_per_lun),                                         \
                      (ecc_stren),                                                           \
                      (OTP_block), (OTP_page_start), (OTP_page_number),}


struct SPI_FLASH {
    char			*name;
    JSC_bool        vcc_33;
    JSC_uint8       mfr_id;
    JSC_uint8       dev_id;
    JSC_uint8       dev_id2;
    JSC_uint32      page_size;
    JSC_uint32      oob_size;
    JSC_uint32      pages_per_blk;
    JSC_uint32      blks_per_lun;
    JSC_uint32      max_bad_blks_per_lun;
    JSC_uint32      ecc_strength;
    JSC_uint32      otp_block;
    JSC_uint32      otp_page_start;
    JSC_uint32      otp_page_number;
};

/****************************************************************************
 	 	 	    Hardware JSC SPI NAND Part Number Information
 ****************************************************************************/
/*| .name | .vcc_33 | .mfr_id | .dev_id | .dev_id2 | .page_size | .oob_size | .pages_per_blk | .blks_per_lun | .max_bad_blks_per_lun | .ecc_strength | .otp_block | .otp_page_start | .otp_page_number|*/
static struct SPI_FLASH spi_flash_table[] = {
    SPI_FLASH_INFO("JS28P1GQSCAHG", 0, 0xBF, 0x01, 0x00,    2048,  64, 64, 1024,  20, 4, 0, 16, 32),
    SPI_FLASH_INFO("JS28U1GQSCAHG", 1, 0xBF, 0x21, 0x00,    2048,  64, 64, 1024,  20, 4, 0, 16, 32),
    SPI_FLASH_INFO("JS28P2GQSDAHG", 0, 0xBF, 0x4A, 0x00,    2048, 128, 64, 2048,  40, 4, 0, 16, 32),
    SPI_FLASH_INFO("JS28U2GQSDAHG", 1, 0xBF, 0x6A, 0x00,    2048, 128, 64, 2048,  40, 4, 0, 16, 32),
    SPI_FLASH_INFO("JS28U4GQSJAHG", 1, 0xBF, 0xAE, 0x80,    2048, 128, 64, 4096,  80, 4, 6, 16, 32),
    SPI_FLASH_INFO("JS28U4GQSKAHG", 1, 0xBF, 0xBD, 0x80,    4096, 256, 64, 2048,  40, 4, 3, 16, 32),
    SPI_FLASH_INFO("JS28U8GQSJAHG", 1, 0xBF, 0xEF, 0x80,    2048, 128, 64, 8192, 160, 4, 6, 16, 32),
    SPI_FLASH_INFO("JS28U8GQSKAHG", 1, 0xBF, 0xFE, 0x80,    4096, 256, 64, 4096,  80, 4, 3, 16, 32),
	/*FabioPi removed designated initializers for RCVT*/
    {/*.name = */(char*) NULL, 		   0, 0,       0,    0,       0,   0,  0,    0,   0, 0, 0,  0, 0},
};
/************** Hardware JSC SPI NAND Part Number Information ****************/


/****************************************************************************
 	 	 	    Page layout information on ECC protected
 ****************************************************************************/
//The following structures show the page layout including information
//on ECC protected main and spare area (pages 43/44/45 of JSC datasheet).
/*FabioPi removed designated initializers for RVCT*/
static AZX_SPI_FLASH_ECCLAYOUT ecc_layout_64 = {
    /*.eccbytes = */32,
    /*.eccpos = */{
        8, 9, 10, 11, 12, 13, 14, 15,
        24, 25, 26, 27, 28, 29, 30, 21,
        40, 41, 42, 43, 44, 45, 46, 47,
        56, 57, 58, 59, 60, 61, 62, 63},
    /*.oobavail =*/ 30,
    /*.oobfree =*/ {
        {
        	/*.offset =*/ 2,
			/*.length = */30
        }
    }
};
/*FabioPi removed designated initializers for RVCT*/
static AZX_SPI_FLASH_ECCLAYOUT ecc_layout_128 = {
    /*.eccbytes = */64,
    /*.eccpos = */{
        64, 65, 66, 67, 68, 69, 70, 71,
        72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87,
        88, 89, 90, 91, 92, 93, 94, 95,
        96, 97, 98, 99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127},
    /*.oobavail = */62,
    /*.oobfree =*/ {
        {
        	/*.offset = */2,
			/*.length =*/ 62
        }
    }
};

static AZX_SPI_FLASH_ECCLAYOUT ecc_layout_256 = {
    /*.eccbytes = */128,
    /*.eccpos = */{
        128, 129, 130, 131, 132, 133, 134, 135,
        136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151,
        152, 153, 154, 155, 156, 157, 158, 159,
        160, 161, 162, 163, 164, 165, 166, 167,
        168, 169, 170, 171, 172, 173, 174, 175,
        176, 177, 178, 179, 180, 181, 182, 183,
        184, 185, 186, 187, 188, 189, 190, 191,
        192, 193, 194, 195, 196, 197, 198, 199,
        200, 201, 202, 203, 204, 205, 206, 207,
        208, 209, 210, 211, 212, 213, 214, 215,
        216, 217, 218, 219, 220, 221, 222, 223,
        224, 225, 226, 227, 228, 229, 230, 231,
        232, 233, 234, 235, 236, 237, 238, 239,
        240, 241, 242, 243, 244, 245, 246, 247,
        248, 249, 250, 251, 252, 253, 254, 255},
    /*.oobavail = */126,
    /*.oobfree = */{
        {
        	/*.offset =*/ 2,
			/*.length =*/ 126
        }
    }
};
/************** Page layout information on ECC protected *********************/


/****************************************************************************
 	 	 	    Hardware JSC SPI NAND Command Definitions
 ****************************************************************************/
#define OPCODE_CMD_WRITE_ENABLE               		(0x06)  /* write enable  */
#define OPCODE_CMD_WRITE_DISABLE              		(0x04)  /* write disable */
#define OPCODE_CMD_GET_FEATURES               		(0x0F)  /* get features  */
#define OPCODE_CMD_SET_FEATURES               		(0x1F)  /* set features  */
#define OPCODE_CMD_READ_ID		              		(0x9F)  /* ID used to identify the device */
#define OPCODE_CMD_READ_UID		              		(0xED)  /* UID used to identify the device */
#define OPCODE_CMD_READ_PARAMETER_PAGE		  		(0xEC)  /* Read Parameter page*/
#define OPCODE_CMD_READ_PAGE	 	 	  	  		(0x13)  /* Read Page */
#define OPCODE_CMD_READ_FROM_CACHE	 	  	  		(0x03)  /* Read From Cache */
#define OPCODE_CMD_READ_FROM_CACHE_FAST	  	  		(0x0B)  /* Read From Cache Fast  */
#define OPCODE_CMD_READ_FROM_CACHE_X2         		(0x3B)  /* Read From Cache x 2 */
#define OPCODE_CMD_READ_FROM_CACHE_X4         		(0x6B)  /* Read From Cache x 4 */
#define OPCODE_CMD_READ_FROM_CACHE_DUAL_IO    		(0xBB)  /* Read From Cache Dual I/O */
#define OPCODE_CMD_READ_FROM_CACHE_QUAD_IO    		(0xEB)  /* Read From Cache Quad I/O */
#define OPCODE_CMD_PROGRAM_LOAD 	 	  	  		(0x02)  /* Program Load */
#define OPCODE_CMD_PROGRAM_LOAD_X4               	(0x32)  /* Program Load x 4 */
#define OPCODE_CMD_PROGRAM_LOAD_RND_DATA	   		(0x84)  /* Program Load Random Data */
#define OPCODE_CMD_PROGRAM_LOAD_RDM_DATA_X4      	(0x34)  /* Program Load Random Data x 4*/
#define OPCODE_CMD_PROGRAM_LOAD_RDM_DATA_QUAD_IO 	(0x72)  /* Program Load Random Data Quad I/O */
#define OPCODE_CMD_PROGRAM_EXECUTE	 	  	  		(0x10)  /* Program Execute */
#define OPCODE_CMD_BLOCK_ERASE  	          		(0xD8)  /* block erase   */
#define OPCODE_CMD_RESET	 	  	          		(0xFF)  /* reset         */
#define OPCODE_CMD_END                        		(0x00)

/***************** Hardware JSC SPI NAND Command Definitions ******************/


#define SPIFLASH_MAX_ADDR_LEN                        (4)
#define SPIFLASH_MAX_ID_LEN                          (3)


/****************************************************************************
 	 	 	    Hardware JSC SPI NAND Feature Registers Description
****************************************************************************/
/*Page 38 of datasheet: feature registers */
#define REGISTER_BLOCK_LOCK                0xA0 	 /* Protection */
#define REGISTER_CFG		               0xB0 	 /* Feature 0 */
#define REGISTER_STATUS                    0xC0 	 /* Status */
#define REGISTER_DRIVE_STRENGTH            0xD0 	 /* Feature 1 */

/* configuration register defines */
#define CFG_QE_MASK                       (0x01)
#define CFG_QE_ENABLE                     (0x01)
#define CFG_ECC_MASK                      (0X10)
#define CFG_ECC_ENABLE                    (0x10)
#define CFG_OTP_MASK                      (0xc2)
#define CFG_OTP_ENTER                     (0x40)
#define CFG_OTP_EXIT                      (0x00)
#define CFG_OTP_PROTECT                   (0xc0)


/* Operation In Progress:
 * This bit is set (OIP = 1) when a Program Execute, Page Read, Block Erase or
 * Reset command is executing, indicating the device is busy. When the bit is 0,
 * the interface is in the ready state. */
#define STATUS_OIP_MASK		0x01
#define STATUS_CRBSY_MASK  (0x80)
#define STATUS_READY       (0 << 0) //0000 0000
#define STATUS_BUSY        (1 << 0) //0000 0001

#define STATUS_E_FAIL_MASK                          (0x04)
#define STATUS_E_FAIL                               (1 << 2)

#define STATUS_P_FAIL_MASK                          (0x08)
#define STATUS_P_FAIL                               (1 << 3)

/* ECC status */
#define ECC_MASK                                    (0x70)
#define ECC_0_BIT                                   (0x00)
#define ECC_1_BIT                                   (0x10)
#define ECC_2_BIT                                   (0x20)
#define ECC_3_BIT                                   (0x30)
#define ECC_4_BIT                                   (0x40)
#define ECC_UNCORR                                  (0x70)

#define ONFI_CRC_BASE                               (0x4F4E)


//TODO check for memory deallocation
#define EXIT_IF_ERROR(err, msg) { 					\
    if (err < 0) { 									\
        AZX_SPI_FLASH_LOG_ERROR("%s error: %d\n", msg, err);  \
        return err; 								\
    } 												\
}


#define JSC_LOG_ERROR_MSG(err_code) \
		AZX_SPI_FLASH_LOG_ERROR("%s ERROR CODE: %d\n", getMessage(err_code), err_code);


//TODO finish here
#define EMIT_EVENT(chip, event) {							\
	chip->opts->clbk.clbkFunc(								\
			NULL, 											\
			(AZX_SPI_FLASH_JSC_EVENT_E) event,				\
			sizeof(AZX_SPI_FLASH_JSC_EVENT_RES_T), NULL,	\
			NULL);											\
}


typedef struct {
	JSC_uint8		cmd;
	JSC_uint8       n_addr;                     /* Number of address */
	JSC_uint8       addr[SPIFLASH_MAX_ADDR_LEN]; /* Reg Offset */
	JSC_uint32      n_tx;                       /* Number of tx bytes */
    JSC_uint8       *tx_buf;                    /* Tx buf */
    JSC_uint32      n_rx;                       /* Number of rx bytes */
    JSC_uint8       *rx_buf;                    /* Rx buf */
} SPI_FLASH_CMD;

typedef struct {
	JSC_uint8			opcode;
	JSC_uint8           addr_bytes;
	JSC_uint8           addr_bits;
	JSC_uint8           dummy_bytes;
	JSC_uint8           data_bits;
} SPI_FLASH_CMD_CFG;


/*
 #####################################################################
 | opcode | addr_bytes | addr_bits | dummy_bytes | data_nbits |
 */

static SPI_FLASH_CMD_CFG jsc_cmd_cfg_table[] = {
    {OPCODE_CMD_GET_FEATURES,                       1, 1, 0, 1},
    {OPCODE_CMD_SET_FEATURES,                       1, 1, 0, 1},
    {OPCODE_CMD_READ_PAGE,                          3, 1, 0, 0},
    {OPCODE_CMD_READ_FROM_CACHE,                    2, 1, 1, 1},
    {OPCODE_CMD_READ_FROM_CACHE_FAST,               2, 1, 1, 1},
    {OPCODE_CMD_READ_FROM_CACHE_X2,                 2, 1, 1, 2},
    {OPCODE_CMD_READ_FROM_CACHE_DUAL_IO,            2, 2, 1, 2},
    {OPCODE_CMD_READ_FROM_CACHE_X4,                 2, 1, 1, 4},
    {OPCODE_CMD_READ_FROM_CACHE_QUAD_IO,            2, 4, 1, 4},
    {OPCODE_CMD_BLOCK_ERASE,                        3, 1, 0, 0},
    {OPCODE_CMD_PROGRAM_EXECUTE,                    3, 1, 0, 0},
    {OPCODE_CMD_PROGRAM_LOAD,                       2, 1, 0, 1},
    {OPCODE_CMD_PROGRAM_LOAD_RND_DATA,              2, 1, 0, 1},
    {OPCODE_CMD_PROGRAM_LOAD_X4,                    2, 1, 0, 4},
    {OPCODE_CMD_PROGRAM_LOAD_RDM_DATA_X4,           2, 1, 0, 4},
    {OPCODE_CMD_PROGRAM_LOAD_RDM_DATA_QUAD_IO,      2, 4, 0, 4},
    {OPCODE_CMD_WRITE_ENABLE,                       0, 0, 0, 0},
    {OPCODE_CMD_WRITE_DISABLE,                      0, 0, 0, 0},
    {OPCODE_CMD_READ_ID,                            0, 0, 1, 1},
    {OPCODE_CMD_READ_UID,                           1, 1, 0, 0},
    {OPCODE_CMD_RESET,                              0, 0, 0, 0},
    {OPCODE_CMD_READ_PARAMETER_PAGE,                1, 1, 0, 0},
    {OPCODE_CMD_END,								0, 0, 0, 0}
};

/* Local function prototypes ====================================================================*/

static AZX_SPI_FLASH_CODE_RESULT_E waitForReady(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *s);
static void 			  setRdWrMode(AZX_SPI_FLASH_SPI_CHIP *chip);
static AZX_SPI_FLASH_CODE_RESULT_E setEccProtection(AZX_SPI_FLASH_SPI_CHIP *chip);
static JSC_uint8 ilog2(JSC_uint32 v);

static AZX_SPI_FLASH_CODE_RESULT_E spiReset(AZX_SPI_FLASH_SPI_CHIP *chip);
static AZX_SPI_FLASH_CODE_RESULT_E spiIssueCmd(AZX_SPI_FLASH_SPI_CHIP *chip, SPI_FLASH_CMD *cmd);
static SPI_FLASH_CMD_CFG * spiLookupCmdCfgTable(JSC_uint8 opcode,
											  SPI_FLASH_CMD_CFG *table);
static AZX_SPI_FLASH_CODE_RESULT_E spiTxRx(AZX_SPI_FLASH_SPI_SLAVE *slave, JSC_uint32 bitlen,
								  JSC_uint8 bits, void *dout,
								  void *din, JSC_uint64 flags);
static AZX_SPI_FLASH_CODE_RESULT_E spiReadStatus(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *status);
static AZX_SPI_FLASH_CODE_RESULT_E spiReadReg(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 reg, JSC_uint8 *buf);
static JSC_bool isInitialize(AZX_SPI_FLASH_SPI_CHIP *chip);

static AZX_SPI_FLASH_CODE_RESULT_E spiReleaseMemory(AZX_SPI_FLASH_SPI_CHIP *chip);
static AZX_SPI_FLASH_CODE_RESULT_E spiReleaseSPI(AZX_SPI_FLASH_SPI_CHIP *chip);
static AZX_SPI_FLASH_CODE_RESULT_E spiReleaseGPIO(AZX_SPI_FLASH_SPI_CHIP *chip);


static AZX_SPI_FLASH_CODE_RESULT_E spiIdTable(AZX_SPI_FLASH_SPI_CHIP *chip, const JSC_uint8 *id);

static AZX_SPI_FLASH_CODE_RESULT_E spiReadID(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *buf);
static AZX_SPI_FLASH_CODE_RESULT_E spiLockBlock(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 lock);
static AZX_SPI_FLASH_CODE_RESULT_E spiWriteReg(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 reg, JSC_uint8 *buf);

static AZX_SPI_FLASH_CODE_RESULT_E spiGetCfg(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *cfg);
static AZX_SPI_FLASH_CODE_RESULT_E spiSetCfg(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *cfg);

static AZX_SPI_FLASH_CODE_RESULT_E spiSetEcc(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_bool ecc);
static AZX_SPI_FLASH_CODE_RESULT_E spiEnableEcc(AZX_SPI_FLASH_SPI_CHIP *chip);
static AZX_SPI_FLASH_CODE_RESULT_E spiDisableEcc(AZX_SPI_FLASH_SPI_CHIP *chip);

static AZX_SPI_FLASH_CODE_RESULT_E spiSetQuad(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_bool quad);
//static NAND_CODE_RESULT_E spiQuadDisable(SPI_NAND_CHIP *chip);
static AZX_SPI_FLASH_CODE_RESULT_E spiQuadEnable(AZX_SPI_FLASH_SPI_CHIP *chip);

static AZX_SPI_FLASH_CODE_RESULT_E spiBlockIsBad(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t offs, JSC_bool *result);
static AZX_SPI_FLASH_CODE_RESULT_E spiDoReadOps(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t from, AZX_SPI_FLASH_MTD_OOB_OPS *ops);

static AZX_SPI_FLASH_CODE_RESULT_E spiDoReadPage(AZX_SPI_FLASH_SPI_CHIP *chip,
										JSC_uint32 page_addr,
										JSC_uint32 column,
										JSC_bool ecc_off,
										JSC_uint32 *corrected,
										JSC_uint8 *buf,
										JSC_size_t len);

static AZX_SPI_FLASH_CODE_RESULT_E spiReadPageToCache(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 page_addr);

static void spiEccStatus(JSC_uint32 status, JSC_uint32 *corrected, JSC_uint32 *ecc_error);

static AZX_SPI_FLASH_CODE_RESULT_E spiReadFromCache(AZX_SPI_FLASH_SPI_CHIP *chip,
										   JSC_uint32 column,
										   JSC_size_t len,
										   JSC_uint8 *rbuf);


static AZX_SPI_FLASH_CODE_RESULT_E spiTransferAutoOob(AZX_SPI_FLASH_SPI_CHIP *chip,
											 JSC_uint8 *oob,
											 JSC_size_t len,
											 JSC_uint32 oob_offset);

static AZX_SPI_FLASH_CODE_RESULT_E spiWriteEnable(AZX_SPI_FLASH_SPI_CHIP *chip);
static AZX_SPI_FLASH_CODE_RESULT_E spiErase(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint64 addr, JSC_uint64 len);
static AZX_SPI_FLASH_CODE_RESULT_E spiEraseBlock(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 page_addr);
static AZX_SPI_FLASH_CODE_RESULT_E spiWriteOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t to, AZX_SPI_FLASH_MTD_OOB_OPS *ops);
static AZX_SPI_FLASH_CODE_RESULT_E spiCheckOps(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t offs, AZX_SPI_FLASH_MTD_OOB_OPS *ops);
static AZX_SPI_FLASH_CODE_RESULT_E spiDoWriteOps(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t offs, AZX_SPI_FLASH_MTD_OOB_OPS *ops);
static AZX_SPI_FLASH_CODE_RESULT_E spiFillAutoOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *oob, JSC_size_t len, JSC_uint32 oob_offset);
static AZX_SPI_FLASH_CODE_RESULT_E spiDoWritePage(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 page_addr, JSC_uint32 column, JSC_uint8 *buf, JSC_size_t len, JSC_bool clr_cache);
static AZX_SPI_FLASH_CODE_RESULT_E spiProgramDataToCache(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 column, JSC_size_t len, JSC_uint8 *wbuf, JSC_bool clr_cache);
static AZX_SPI_FLASH_CODE_RESULT_E spiProgramExecute(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 page_addr);

static AZX_SPI_FLASH_CODE_RESULT_E spiReadOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t from, AZX_SPI_FLASH_MTD_OOB_OPS *ops);
static AZX_SPI_FLASH_CODE_RESULT_E spiReadUIDpage(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 column, JSC_uint8 *buf, JSC_size_t len);
static AZX_SPI_FLASH_CODE_RESULT_E spiReadUIDPageToCache(AZX_SPI_FLASH_SPI_CHIP *chip);
static AZX_SPI_FLASH_CODE_RESULT_E spiReadParameterPage(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 column, JSC_uint8 *buf, JSC_size_t len);
static AZX_SPI_FLASH_CODE_RESULT_E spiReadParameterPageToCache(AZX_SPI_FLASH_SPI_CHIP *chip);
static AZX_SPI_FLASH_CODE_RESULT_E spiCheckParameter(AZX_SPI_FLASH_ONFI_PARAMS *p);
static JSC_uint16 onfiCrc16(JSC_uint16 crc, JSC_uint8 *p, JSC_size_t len);

/* Local functions definition ===================================================================*/

/*******************************************************************************
                  SPI_FLASH_logFormatted

Function:
Arguments:
Description:
*******************************************************************************/
void SPI_FLASH_logFormatted(SPI_FLASH_LOG_HOOK_LEVELS_E level,
						const char* function,
						const char* file,
						int line, const char *fmt, ... )
{
	char buf[512];
	int bufSize = sizeof(buf);
	va_list arg;
	INT32   offset = 0;
	unsigned int now = azx_spi_flash_get_uptime();

	//CHAR taskName[32];

	memset(buf,0,bufSize);

	switch(level)
	{
	case SPI_FLASH_LOG_LEVEL_ERROR:
		offset = sprintf(buf, "%s %u.%03u %8s:%d - %8s: ",
				"[ERROR]",
				now / 1000, now % 1000,
				file,
				line,
				function
		);
		break;
	case SPI_FLASH_LOG_LEVEL_INFO:
		break;
	case SPI_FLASH_LOG_LEVEL_DEBUG:
		offset = sprintf(buf, "%s %u.%03u %8s:%d - %8s: ",
				"[DEBUG]",
				now / 1000, now % 1000,
				file,
				line,
				function
		);
		break;
	default:
		break;
	}
	va_start(arg, fmt);
	vsnprintf(buf + offset, bufSize-offset, fmt, arg);
	va_end(arg);

	AZX_LOG_INFO("%s\r\n", buf);  //This is a generic function, like printf

}
/***************** SPI_FLASH_logFormatted ***********************************************/


/******************************************************************************
                  spiLookupCmdCfgTable

Function:       SPI_NAND_CMD_CFG *spiLookupCmdCfgTable(JSC_uint8 opcode, SPI_NAND_CMD_CFG *table)
Arguments:
Return Value:
Description:

******************************************************************************/
static SPI_FLASH_CMD_CFG *spiLookupCmdCfgTable(JSC_uint8 opcode, SPI_FLASH_CMD_CFG *table)
{
    SPI_FLASH_CMD_CFG *index = table;

    for (; index->opcode != OPCODE_CMD_END; index++) {
        if (index->opcode == opcode)
            return index;
    }

    AZX_SPI_FLASH_LOG_ERROR("Invalid spi nand opcode %x\n", opcode);
    return NULL;
}
/**************  spiLookupCmdCfgTable ***************/


/******************************************************************************
                  spiTxRx
Function:
Arguments:
Return Value:
Description: Sets the I/O channel with the relative bit of flags
******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiTxRx(AZX_SPI_FLASH_SPI_SLAVE *slave, JSC_uint32 bitlen,
		JSC_uint8 bits, void *dout, void *din, JSC_uint64 flags)
{
    if (bits == 4)
        flags |= SPI_XFER_QUAD;
    else if (bits == 2)
        flags |= SPI_XFER_DUAL;

    return azx_spi_flash_spi_xfer(slave, bitlen, dout, din, flags);
}
/**************  spiTxRx *******************************************/


/******************************************************************************
                  ilog2
Function:
Arguments:	 number to calculate the log2
Return Value:
Description: Return log base 2 of v
******************************************************************************/
static JSC_uint8 ilog2(JSC_uint32 v)
{
	JSC_uint8 l = 0;
	//1UL cast 1 to UNSIGNED LONG
    while ((1UL << l) < v)
        l++;
    return l;
}
/**************  ilog2 *******************************************/


/******************************************************************************
                  spiIdTable
Function:
Arguments:
 	 	  	 @chip: SPI-NAND device structure
 	 	 	 @id: point to manufacture id and device id
Return Value:
Description: Scan chip info in id table.
			 If found in id table, config chip with table information.
******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiIdTable(AZX_SPI_FLASH_SPI_CHIP *chip, const JSC_uint8 *id)
{
    struct SPI_FLASH *type = spi_flash_table;

    //Scorre la lista spi_nand_table fino a quando type->name != NULL
    for (JSC_uint8 index = 0; type->name; index++, type++)
    {
        AZX_LOG_INFO("\r\n");
    	AZX_LOG_INFO("table id[0] = %d\r\n", id[0]);
    	AZX_LOG_INFO("table id[1] = %d\r\n", id[1]);
    	AZX_LOG_INFO("table id[2] = %d\r\n", id[2]);
    	AZX_LOG_INFO("\r\n");

    	//Check the right match
        if (id[0] == type->mfr_id &&
        	id[1] == type->dev_id &&
			id[2] == type->dev_id2)
        {
        	chip->vcc_33                = type->vcc_33;
			chip->mfr_id                = type->mfr_id;
			chip->dev_id                = type->dev_id;
			chip->dev_id2               = type->dev_id2;
			chip->name                  = type->name;
			chip->size                  = type->page_size * type->pages_per_blk * type->blks_per_lun;
			chip->block_size            = type->page_size * type->pages_per_blk;
			chip->page_size             = type->page_size;
			chip->oob_size              = type->oob_size;
			chip->block_shift           = ilog2(chip->block_size);
			chip->page_shift            = ilog2(chip->page_size);
			chip->page_mask             = chip->page_size - 1;
			chip->ecc_strength          = type->ecc_strength;
			chip->max_bad_blks_per_lun  = type->max_bad_blks_per_lun;
			chip->otp_block             = type->otp_block;
			chip->otp_page_start        = type->otp_page_start;
			chip->otp_page_number       = type->otp_page_number;
			return AZX_SPI_FLASH_CODE_PASS;
        }
    }
    return AZX_SPI_FLASH_CODE_INVALID_MEMORY_MODEL;
}
/**************  spiIdTable *******************************************/


/******************************************************************************
                  spiReadID
Function:
Arguments:
Return Value:
Description:
******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReadID(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *buf)
{
	AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_RESET_FAIL;

	SPI_FLASH_CMD cmd;
	azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
	cmd.cmd = OPCODE_CMD_READ_ID;
	cmd.n_rx = 3;
	cmd.rx_buf = buf;

	ret = spiIssueCmd(chip, &cmd);
	if (ret != AZX_SPI_FLASH_CODE_PASS) {
		AZX_SPI_FLASH_LOG_ERROR("spi_nand read ID failed!\r\n");
		return ret;
	}

	return ret;
}
/**************  spiReadID *******************************************/


/******************************************************************************
                  spiIssueCmd

Function:       NAND_CODE_RESULT_E spiIssueCmd(SPI_NAND_CHIP *chip, SPI_NAND_CMD *cmd)
Arguments:		spi: spi device structure
				cmd: command structure
Return Value:
Description:	to process a command to send to the SPI-NAND
				Set up the command buffer to send to the SPI controller.
				The command buffer has to initialized to 0.

******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiIssueCmd(AZX_SPI_FLASH_SPI_CHIP *chip, SPI_FLASH_CMD *cmd)
{
	AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_FAIL;
	SPI_FLASH_CMD_CFG *cmd_cfg = NULL;
	AZX_SPI_FLASH_SPI_SLAVE *spi = chip->spi;
	JSC_uint32 buf[SPIFLASH_MAX_ADDR_LEN];
	int flags = SPI_XFER_BEGIN; //0000 0001
	JSC_int32 bitlen;

	cmd_cfg = spiLookupCmdCfgTable(cmd->cmd, jsc_cmd_cfg_table);

	//Check if command is valid
	if (cmd_cfg == NULL)
    {
		AZX_SPI_FLASH_LOG_ERROR("SPI ret : 0x%08X\n", AZX_SPI_FLASH_CODE_INVALID_ARGUMENT);
        return AZX_SPI_FLASH_CODE_INVALID_ARGUMENT;
    }

	//If the operation is atomic, the command performs the operations starting with CS = LOW and at the end of the operations SPI CS = HIGH
    if (cmd->n_tx == 0 &&
    	cmd->n_rx == 0 &&
		cmd_cfg->addr_bytes == 0
		&& cmd_cfg->dummy_bytes == 0)
    {
    	//flags = flags | SPI_XFER_END;
    	//flags = 0000 0001 | 0000 0010
    	//flags = 0000 0011
        flags |= SPI_XFER_END;
    }

    //Write the command in to the SPI
    ret = spiTxRx(spi, 8, 1, &cmd->cmd, NULL, flags);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	AZX_SPI_FLASH_LOG_ERROR("SPI ret : 0x%08X\n", ret);
    	return ret;
    }

    if (cmd_cfg->addr_bytes || cmd_cfg->dummy_bytes)
    {
        flags = 0;
        if (cmd->n_tx == 0 &&
        	cmd->n_rx == 0)
        {
            flags |= SPI_XFER_END;
        }

        //TODO verify better
        if (cmd_cfg->addr_bytes > cmd->n_addr)
        {
            azx_spi_flash_memcpy(buf, cmd->addr, cmd->n_addr);
            azx_spi_flash_memset(cmd->addr, 0, cmd->n_addr);
            azx_spi_flash_memcpy(cmd->addr + cmd_cfg->addr_bytes - cmd->n_addr, buf, cmd->n_addr);
        }

        bitlen = (cmd_cfg->addr_bytes + cmd_cfg->dummy_bytes) * 8;
        ret = spiTxRx (spi, bitlen, cmd_cfg->addr_bits, cmd->addr, NULL, flags);
        if (ret != AZX_SPI_FLASH_CODE_PASS)
        {
        	AZX_SPI_FLASH_LOG_ERROR("SPI ret : 0x%08X\n", ret);
        	return ret;
        }
    }
    if (cmd->n_tx)
    {
    	ret = spiTxRx(spi, cmd->n_tx * 8, cmd_cfg->data_bits, cmd->tx_buf, NULL, SPI_XFER_END);
        if (ret != AZX_SPI_FLASH_CODE_PASS)
        {
        	AZX_SPI_FLASH_LOG_ERROR("SPI ret : 0x%08X\n", ret);
        	return ret;
        }
    }
    else if (cmd->n_rx) {
    	ret = spiTxRx(spi, cmd->n_rx * 8, cmd_cfg->data_bits, NULL, cmd->rx_buf, SPI_XFER_END);
        if (ret != AZX_SPI_FLASH_CODE_PASS)
        {
        	AZX_SPI_FLASH_LOG_ERROR("SPI ret : 0x%08X\n", ret);
        	return ret;
        }
    }

    if (ret != AZX_SPI_FLASH_CODE_PASS) {
    	AZX_SPI_FLASH_LOG_ERROR("SPI ret : 0x%08X\n", ret);
        return ret;
    }

    return AZX_SPI_FLASH_CODE_PASS;
}
/**************  spiIssueCmd ***************/


/*******************************************************************************
                  setRdWrMode

Function:       void setRdWrMode(SPI_NAND_CHIP *chip)
Arguments:
		chip: 	SPI-NAND device structure
Return Value:   void
Description:    Chose the best read write command. Chose the
				fastest r/w command according to spi controller's ability.
*******************************************************************************/
void setRdWrMode(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	AZX_SPI_FLASH_SPI_SLAVE *spi = chip->spi;

	if(spi->op_mode_rx & SPI_OPM_RX_QUAD)
	{
		chip->read_cache_op =  OPCODE_CMD_READ_FROM_CACHE_QUAD_IO;
	}
    else if (spi->op_mode_rx & SPI_OPM_RX_DUAL)
    {
    	chip->read_cache_op = OPCODE_CMD_READ_FROM_CACHE_DUAL_IO;
    }
    else
    {
        chip->read_cache_op = OPCODE_CMD_READ_FROM_CACHE_FAST; //TODO OPCODE_CMD_READ_FROM_CACHE
    }

    if (spi->op_mode_tx & SPI_OPM_TX_QUAD)
    {
        chip->write_cache_op = OPCODE_CMD_PROGRAM_LOAD_X4;
        chip->write_cache_rdm_op = OPCODE_CMD_PROGRAM_LOAD_RDM_DATA_X4;
    }
    else
    {
        chip->write_cache_op = OPCODE_CMD_PROGRAM_LOAD;
        chip->write_cache_rdm_op = OPCODE_CMD_PROGRAM_LOAD_RND_DATA;
    }
}
/***************** setRdWrMode ***********************************************/


/*******************************************************************************
                  spiReadStatus

Function:       int spiReadStatus(SPI_NAND_CHIP *chip, JSC_uint8 *status);
Arguments:
				chip: 	SPI-NAND device structure
				status: buffer to store value
Return Value:
Description:	get status register value
 	 	 	 	After read, write, or erase, the Nand device is expected to set the
 	 	 	 	busy status. This function is to allow reading the status of the
 	 	 	 	command: read, write, and erase.
 	 	 	 	Once the status turns to be ready, the other status bits also are
 	 	 	 	valid status bits.
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReadStatus(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *status)
{
	 return spiReadReg(chip, REGISTER_STATUS, status);
}
/**************  spiReadStatus **********************************************/


/*******************************************************************************
                  spiReadReg

Function:		NAND_CODE_RESULT_E spiReadReg(SPI_NAND_CHIP *chip, JSC_uint8 reg, JSC_uint8 *buf)
Arguments:
				chip: SPI-NAND device structure
				reg: register to read
				buf: buffer to store value
Return Value:
Description: 	send command 0Fh to read register
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReadReg(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 reg, JSC_uint8 *buf)
{
	 SPI_FLASH_CMD cmd;
	 AZX_SPI_FLASH_CODE_RESULT_E ret;

	 azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
	 cmd.cmd = OPCODE_CMD_GET_FEATURES;
	 cmd.n_addr = 1;
	 cmd.addr[0] = reg;
	 cmd.n_rx = 1;
	 cmd.rx_buf = buf;

	ret = spiIssueCmd(chip, &cmd);
	if (ret < 0)
	{
		AZX_SPI_FLASH_LOG_ERROR("err: %d read register %d\n", ret, reg);
	}

	return ret;
}
/**************  spiReadReg **********************************************/


/*******************************************************************************
                  waitForReady

Function:       NAND_CODE_RESULT_E waitForReady(SPI_NAND_CHIP *chip, JSC_uint8 *s)
Arguments:
Return Value:   Return the status of the status register
Description:    This function is called after an operation on the NAND is
                asserted. The function wait until the NAND is ready.
		        NOTE: after return the NAND is lived in status register mode
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E waitForReady(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *s)
{
	JSC_uint64 start = azx_spi_flash_get_ticks();
	JSC_uint8 status = OPCODE_CMD_RESET;

	/* set timeout to 1 second */
	JSC_uint64 timeout = start + SPI_FLASH_WAITING_TIMEOUT;
	AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_TIMEDOUT;

    while (azx_spi_flash_get_ticks() < timeout)
    {
    	spiReadStatus(chip, &status);
        if ((status & STATUS_OIP_MASK) == STATUS_READY)
        {
//        	AZX_SPI_FLASH_LOG_INFO("Status ready!\r\n");
			ret = AZX_SPI_FLASH_CODE_PASS;
			break;
        }
        else
        {
        	AZX_SPI_FLASH_LOG_INFO("Busy!\r\n");
			azx_spi_flash_delay(FLASH_DELAY_FOR_READY); //300
        }
    }
    if(azx_spi_flash_get_ticks() >= timeout)
	{
		ret = AZX_SPI_FLASH_CODE_TIMEDOUT;
	}

    if (s)
    {
    	*s = status;
    }
    return ret;

}
/***************** waitForReady ***********************************************/


/******************************************************************************
                      spiReset

NAND_CODE_RESULT_E spiReset(SPI_NAND_CHIP *chip)
Arguments:        na
Return Value:     na
Description:      The Reset command (FFh) terminates the current internal operation
				  and initializes NAND devices. Then, the first page is automatically
				  loaded into the cache register again. To ensure that the current
				  operation is terminated and the re-initialization is completed
				  correctly, a host should start to check the OIP status bit at least
				  300ns later after sending the Reset command and wait for tRST until
				  the bit is cleared.
******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReset(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_RESET_FAIL;
	SPI_FLASH_CMD cmd;

	azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
	cmd.cmd = OPCODE_CMD_RESET;

	ret = spiIssueCmd(chip, &cmd); //NAND_CommandInput
	if (ret != AZX_SPI_FLASH_CODE_PASS) {
		AZX_SPI_FLASH_LOG_ERROR("spi_nand reset failed!\r\n");
		return ret;
	}

	ret = waitForReady(chip, NULL);
	if (ret != AZX_SPI_FLASH_CODE_PASS) {
		AZX_SPI_FLASH_LOG_ERROR("error %d waiting reset\r\n", ret);
		return ret;
	}

	return ret;
}
/**************  spiReset ***************/


/*******************************************************************************
                  isInitialize

Function:       JSC_bool isInitialize(SPI_NAND_CHIP *chip)
Arguments:
Return Value:   Return TRUE if NAND operations are initialized FALSE otherwise
Description:    Check if NAND operations are initialized
*******************************************************************************/
static JSC_bool isInitialize(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	if (chip->opts->init != 1)
	{
		AZX_SPI_FLASH_LOG_ERROR("JSC NAND not initialized!!!");
	}
	return (chip->opts->init);
}
/***************** NAND_initialize ***********************************************/


/*******************************************************************************
                  setEccProtection
Function:
Arguments:
Return Value:
Description:	According with the table of page 43 of the datasheet.
The device has an embedded 4-bit ECC engine which can be
configured to be enabled or disabled by setting the ECC_EN bit in the
feature 1 register. By default, it is enabled to protect the 1st page
that is automatically loaded into the cache register after power-up.
*******************************************************************************/

static AZX_SPI_FLASH_CODE_RESULT_E setEccProtection(AZX_SPI_FLASH_SPI_CHIP *chip)
{
    chip->datbuf = (JSC_uint8*) azx_spi_flash_malloc(chip->page_size + chip->oob_size);
    if (!chip->datbuf)
    {
		AZX_SPI_FLASH_LOG_ERROR("------>ECC NAND_malloc FAIL\r\n");
        return AZX_SPI_FLASH_CODE_MALLOC_FAIL;
    }

    chip->oobbuf = chip->datbuf + chip->page_size;
    if (chip->oob_size == 64)
        chip->ecclayout = &ecc_layout_64;
    else if (chip->oob_size == 128)
        chip->ecclayout = &ecc_layout_128;
    else if (chip->oob_size == 256)
        chip->ecclayout = &ecc_layout_256;

    //The I/O drive strength of Quad SPI interface is described in page 38/39
    //of the JSC datasheet. Users can configure the strength
    //considering SPI frequency and board environment
    chip->refresh_threshold = (chip->ecc_strength * 3 + 3) / 4;

    return AZX_SPI_FLASH_CODE_PASS;
}
/***************** setEccProtection ***********************************************/


/*******************************************************************************
                  spiLockBlock
Function:
Arguments:
Return Value:
Description:
*******************************************************************************/
/**
 * spiLockBlock - [Interface] write block lock register to
 * lock/unlock device
 * @spi: spi device structure
 * @lock: value to set to block lock register
 * Description:
 *   After power up, all the Nand blocks are locked.  This function allows
 *   one to unlock the blocks, and so it can be written or erased.
 *
 * Register Bit Address Table 5. Feature Registers Description page 38
 *   | 7    | 6        | 5   | 4   | 3   | 2   | 1   | 0        |
 *   |----------------------------------------------------------|
 *   | BRWD | Reversed | BP2 | BP1 | BP0 | INV | CMP | Reversed |
 *
 * Block Lock Register Block Protection Bits
 *
 * CMP INV BP2 BP1 BP0 Protected Portion
 * X   X   0   0   0   All unlocked
 * 0   0   0   0   1   Upper 1/64 locked
 * 0   0   0   1   0   Upper 1/32 locked
 * 0   0   0   1   1   Upper 1/16 locked
 * 0   0   1   0   0   Upper 1/8 locked
 * 0   0   1   0   1   Upper 1/4 locked
 * 0   0   1   1   0   Upper 1/2 locked
 * X   X   1   1   1   All Locked (default)
 * 0   1   0   0   1   Lower 1/64 locked
 * 0   1   0   1   0   Lower 1/32 locked
 * 0   1   0   1   1   Lower 1/16 locked
 * 0   1   1   0   0   Lower 1/8 locked
 * 0   1   1   0   1   Lower 1/4 locked
 * 0   1   1   1   0   Lower 1/2 locked
 * 1   0   0   0   1   Lower 63/64 locked
 * 1   0   0   1   0   Lower 31/32 locked
 * 1   0   0   1   1   Lower 15/16 locked
 * 1   0   1   0   0   Lower 7/8 locked
 * 1   0   1   0   1   Lower 3/4 locked
 * 1   0   1   1   0   Block 0 locked
 * 1   1   0   0   1   Upper 63/64 locked
 * 1   1   0   1   0   Upper 31/32 locked
 * 1   1   0   1   1   Upper 15/16 locked
 * 1   1   1   0   0   Upper 7/8 locked
 * 1   1   1   0   1   Upper 3/4 locked
 * 1   1   1   1   0   Block 0 locked
 */
static AZX_SPI_FLASH_CODE_RESULT_E spiLockBlock(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 lock)
{
	return spiWriteReg(chip, REGISTER_BLOCK_LOCK, &lock);
}
/***************** spiLockBlock ***********************************************/


/*******************************************************************************
                  spiWriteReg
Function:
Arguments:
  	  @chip: SPI-NAND device structure
  	  @reg; register to write
 	  @buf: buffer stored value
Return Value:
Description: send command 1Fh to write register
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiWriteReg(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 reg, JSC_uint8 *buf)
{
    SPI_FLASH_CMD cmd;
    AZX_SPI_FLASH_CODE_RESULT_E ret;

    azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
    cmd.cmd = OPCODE_CMD_SET_FEATURES;
    cmd.n_addr = 1;
    cmd.addr[0] = reg;
    cmd.n_tx = 1;
    cmd.tx_buf = buf;

    ret = spiIssueCmd(chip, &cmd);
    if (ret < 0)
    {
        AZX_SPI_FLASH_LOG_ERROR("err: %d write register %d\n", ret, reg);
    }

	return ret;
}
/***************** spiWriteReg ***********************************************/


/*******************************************************************************
                  spiEnableEcc
Function:
Arguments:
  	  @chip: SPI-NAND device structure
Return Value:
Description: enable internal ECC.
There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 *   Enable chip internal ECC, set the bit to 1
 *   Disable chip internal ECC, clear the bit to 0
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiEnableEcc(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	return spiSetEcc(chip, true);
}
/***************** spiEnableEcc ***********************************************/


/*******************************************************************************
                  spiDisableEcc
Function:
Arguments:
  	  @chip: SPI-NAND device structure
Return Value:
Description: disable internal ECC.
There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 *   Enable chip internal ECC, set the bit to 1
 *   Disable chip internal ECC, clear the bit to 0
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiDisableEcc(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	return spiSetEcc(chip, false);
}
/***************** spiDisableEcc ***********************************************/


/*******************************************************************************
                  spiSetEcc
Function:
Arguments:
  	  @chip: SPI-NAND device structure
  	  @ecc: if ecc == TRUE then spiEnableEcc
  	  	  	if ecc == FALSE then spiEnableEcc
Return Value:
Description:
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiSetEcc(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_bool ecc)
{
	JSC_uint8 cfg = 0;
	AZX_SPI_FLASH_CODE_RESULT_E ret;

	ret = spiGetCfg(chip, &cfg);
	EXIT_IF_ERROR(ret, "NAND_getCfg FAILS!");

    if ((cfg & CFG_ECC_MASK) == CFG_ECC_ENABLE) //check if it is already enable
    {
        return AZX_SPI_FLASH_CODE_PASS;
    }

    //sets the bit taking into account the value of the ecc variable
    if (ecc == true)
    {
        cfg |= CFG_ECC_ENABLE;
    }
    else
    {
        cfg &= ~CFG_ECC_ENABLE;
    }

    ret = spiSetCfg(chip, &cfg);
	EXIT_IF_ERROR(ret, "NAND_spiSetCfg FAILS!");

    return ret;
}
/***************** spiSetEcc ***********************************************/


/*******************************************************************************
                  spiGetCfg
Function:
Arguments:
 	 	 @chip: SPI-NAND device structure
 	 	 @cfg: buffer to store value
Return Value:
Description:	get configuration register value.
Configuration register includes OTP config, Lock Tight enable/disable
and Internal ECC enable/disable.
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E spiGetCfg(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *cfg)
{
    return spiReadReg(chip, REGISTER_CFG, cfg);
}
/***************** spiGetCfg ***********************************************/


/*******************************************************************************
                  spiSetCfg
Function:
Arguments:
 	 	 @chip: SPI-NAND device structure
 	 	 @cfg: buffer stored value
Return Value:
Description:	set value to configuration register
Configuration register includes OTP config, Lock Tight enable/disable
and Internal ECC enable/disable.

*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E spiSetCfg(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *cfg)
{
    return spiWriteReg(chip, REGISTER_CFG, cfg);
}
/***************** spiSetCfg ***********************************************/


/*******************************************************************************
                  spiQuadEnable
Function:
Arguments:
 	 	 @chip: SPI-NAND device structure
Return Value:
Description:	enable QUAD mode for some chips
Some SPI NAND chips need to enable QUAD mode before X4/QUAD transfer.
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiQuadEnable(AZX_SPI_FLASH_SPI_CHIP *chip)
{
    return spiSetQuad(chip, true);
}
/***************** spiQuadEnable ***********************************************/


/*******************************************************************************
                  spiSetQuad
Function:
Arguments:
 	 	 @chip: SPI-NAND device structure
Return Value:
Description:	disable QUAD mode for some chips
Some SPI NAND chips need to enable QUAD mode before X4/QUAD transfer.
*******************************************************************************/
//static NAND_CODE_RESULT_E spiQuadDisable(SPI_NAND_CHIP *chip)
//{
//    return spiSetQuad(chip, false);
//}
/***************** spiQuadDisable ***********************************************/


/*******************************************************************************
                  spiSetQuad
Function:
Arguments:
  	  @chip: SPI-NAND device structure
  	  @ecc: if quad == TRUE then spiQuadEnable
  	  	  	if quad == FALSE then spiQuadDisable
Return Value:
Description:
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiSetQuad(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_bool quad)
{
	JSC_uint8 cfg = 0;
	AZX_SPI_FLASH_CODE_RESULT_E ret;

    ret = spiGetCfg(chip, &cfg);
	EXIT_IF_ERROR(ret, "NAND_spiSetCfg FAILS!");

    if ((cfg & CFG_QE_MASK) == CFG_QE_ENABLE)
    {
        return AZX_SPI_FLASH_CODE_PASS;
    }

    if (quad == true)
    {
        cfg |= CFG_QE_ENABLE;
    }
    else
    {
    	cfg &= ~CFG_ECC_ENABLE;
    }

    ret = spiSetCfg(chip, &cfg);
	EXIT_IF_ERROR(ret, "NAND_spiSetCfg FAILS!");

    return AZX_SPI_FLASH_CODE_PASS;
}
/***************** spiSetQuad ***********************************************/



/*******************************************************************************
                  nandReleaseSPI
Function:
Arguments:
  	  @chip: SPI-NAND device structure
Return Value:
Description: [Interface] Release SPI handler
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReleaseSPI(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	AZX_SPI_FLASH_DESCRIPTOR *fd = (AZX_SPI_FLASH_DESCRIPTOR*) chip->spi->hspi;
	return azx_spi_flash_spi_close(&fd->spi);
}
/***************** nandReleaseSPI ***********************************************/


/*******************************************************************************
	                  nandReleaseGPIO
Function:
Arguments:
	  @chip: SPI-NAND device structure
Return Value:
Description: [Interface] Release GPIO handler
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReleaseGPIO(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	AZX_SPI_FLASH_DESCRIPTOR *fd = (AZX_SPI_FLASH_DESCRIPTOR*) chip->spi->hspi;
	return azx_spi_flash_gpio_close(&fd->gpio);
}
/***************** nandReleaseGPIO ***********************************************/


/*******************************************************************************
                  spiReleaseMemory
Function:
Arguments:
  	  @chip: SPI-NAND device structure
Return Value: TRUE if azx_nand_free succeeded
Description: [Interface] Release function
Release all allocate memory. If a pointer is NULL automatically is skipped
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReleaseMemory(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	AZX_SPI_FLASH_CODE_RESULT_E ret;

	if (chip)
	{
		//release oll pointer inside AZX_NAND_SPI_CHIP allocated with azx_nand_malloc

		//*spi
		if (chip->spi)
		{
			ret = azx_spi_flash_free(chip->spi);
			if (ret < 0)
			{
				JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_FREE_FAIL);
				return AZX_SPI_FLASH_CODE_FREE_FAIL;
			}
		}
		else
		{
        	AZX_SPI_FLASH_LOG_DEBUG("chip->spi should not be NULL \r\n");
		}

		//*opts
		if (chip->opts)
		{
			ret = azx_spi_flash_free(chip->opts);
			if (ret < 0)
			{
				JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_FREE_FAIL);
				return AZX_SPI_FLASH_CODE_FREE_FAIL;
			}
		}
		else
		{
        	AZX_SPI_FLASH_LOG_DEBUG("chip->opts should not be NULL \r\n");
		}

		//*datbuf
		if (chip->datbuf)
		{
			ret = azx_spi_flash_free(chip->datbuf);
			if (ret < 0)
			{
				JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_FREE_FAIL);
				return AZX_SPI_FLASH_CODE_FREE_FAIL;
			}
		}
		else
		{
        	AZX_SPI_FLASH_LOG_DEBUG("chip->datbuf should not be NULL \r\n");
		}

		//*chip
		ret = azx_spi_flash_free(chip);
	    if (ret < 0)
	    {
	    	JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_FREE_FAIL);
	        return AZX_SPI_FLASH_CODE_FREE_FAIL;
	    }
	    return AZX_SPI_FLASH_CODE_PASS;
	}

	AZX_SPI_FLASH_LOG_DEBUG("chip should not be NULL \r\n");
	return AZX_SPI_FLASH_CODE_PASS;
}
/***************** spiReleaseMemory ***********************************************/


/*******************************************************************************
                  spiBlockIsBad
Function:
Arguments:
  	  @chip: 	SPI-NAND device structure
  	  @offs: 	offset from device start
  	  @result: 	true if is bad block, false otherwise
Return Value:
Description: [Interface] check block is bad or not via read
bad block mark(the first two byte in oob area of the first page in the block)
For a block, read the first page's first two byte of oob data, if data is all
0xFF, the block is a good block, otherwise it a bad block.
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiBlockIsBad(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t offs, JSC_bool *result)
{
	AZX_SPI_FLASH_MTD_OOB_OPS ops = {0,0,0,0,0,0,0,0};
	JSC_uint32 block_addr;
	JSC_uint8 bad[2] = {0, 0};
	AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_FAIL;

	//offs contains the block address
	//obtain from the address the nubmer of the block
	block_addr = offs >> chip->block_shift;
	ops.mode = MTD_OPS_RAW;
	ops.ooblen = 2;
	ops.oobbuf = bad;

	*result = false;
	JSC_loff_t page_addr = block_addr << chip->block_shift;


    ret = spiDoReadOps(chip, page_addr, &ops);
	if (ret != AZX_SPI_FLASH_CODE_PASS && ret != AZX_SPI_FLASH_CODE_EBADMSG)
	{
		EXIT_IF_ERROR(ret, "spiDoReadOps chip error!");
		/* log error message */
		JSC_LOG_ERROR_MSG(ret);
		return ret;
	}


    if (bad[0] != 0xFF || bad[1] != 0xFF)
    {
		JSC_LOG_ERROR_MSG(ret);
        *result = true;
    }

	return ret;
}
/***************** spiBlockIsBad ***********************************************/


/*******************************************************************************
                  spiDoReadOps
Function:	spiDoReadOps(SPI_NAND_CHIP *chip, JSC_loff_t from, MTD_OOB_OPS *ops)
Arguments:
		@chip: spi nand device structure
 	 	@from: offset to read from
 	    @ops: oob ops structure
Return Value:
		Return NAND_CODE_EBADMSG when internal ecc can not correct bitflips.
Description: read data from flash to buffer
		Disable internal ECC before reading when MTD_OPS_RAW set.
		Normal read function, read one page to buffer before issue
		another. Return NAND_CODE_EUCLEAN when bitflip is over threshold.

*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiDoReadOps(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t from, AZX_SPI_FLASH_MTD_OOB_OPS *ops)
{
	JSC_int32 page_addr, page_offset, size, oobsize;
	JSC_int32 readlen = ops->len;
	JSC_int32 oobreadlen = ops->ooblen;
	AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_FAIL;

	JSC_int32 ooblen = (ops->mode == MTD_OPS_AUTO_OOB) ?
					   chip->ecclayout->oobavail : chip->oob_size;

	JSC_int32 oob_offset = ops->ooboffs;

	JSC_bool ecc_off = ops->mode == MTD_OPS_RAW;

	JSC_uint32 corrected = 0;
	JSC_uint32 max_bitflip = 0;
	JSC_uint32 failed = 0;

    page_addr = from >> chip->page_shift;
    page_offset = from & chip->page_mask;

    if (ecc_off)
    {
        ret = spiDisableEcc(chip);
        if (ret != AZX_SPI_FLASH_CODE_PASS)
        {
        	AZX_SPI_FLASH_LOG_ERROR("error spiDisableEcc \r\n");
        	return ret;
        }
    }

    while (readlen || oobreadlen)
    {

		size = 0;
		oobsize = 0;

		if (readlen)
		{
			size = min(readlen, chip->page_size - page_offset);
		}
		if (oobreadlen)
		{
			oobsize = min(oobreadlen, ooblen - oob_offset);
		}

		JSC_uint32 column = size ? 0 : chip->page_size;
		JSC_size_t len = (size ? chip->page_size : 0) + (oobsize ? chip->oob_size : 0);
		JSC_uint8 *buf = size ? chip->datbuf : chip->oobbuf;

        ret = spiDoReadPage(chip, page_addr,
        					column,
							ecc_off, &corrected,
							buf, len);

        if (ret != AZX_SPI_FLASH_CODE_PASS)
        {
            if (ret == AZX_SPI_FLASH_CODE_EBADMSG)
            {
                failed++;
            }
            else
            {
            	AZX_SPI_FLASH_LOG_ERROR("error %d reading page 0x%x\r\n", ret, page_addr);
            	break;
            }


        }

        max_bitflip = max(corrected, max_bitflip);

        if (readlen)
        {
            azx_spi_flash_memcpy(ops->datbuf + ops->retlen,
            			chip->datbuf + page_offset,
						size);
        }

        if (oobreadlen)
        {
        	oobsize = min(oobreadlen, ooblen - oob_offset);
            if (ops->mode == MTD_OPS_AUTO_OOB)
            {
                spiTransferAutoOob(chip, ops->oobbuf + ops->oobretlen,
                						   oobsize, oob_offset);
            }
            else
            {
                azx_spi_flash_memcpy(ops->oobbuf + ops->oobretlen,
                			chip->oobbuf + oob_offset, oobsize);
            }
        }
        ops->oobretlen += oobsize;
        oobreadlen -= oobsize;

        ops->retlen += size;
        readlen -= size;

        page_offset = 0;
        oob_offset = 0;
        page_addr++;

    } //while (readlen || oobreadlen)


    if (max_bitflip > chip->refresh_threshold)
    {
        ret = AZX_SPI_FLASH_CODE_EUCLEAN;
        AZX_SPI_FLASH_LOG_ERROR("error %d : error bit %d over refresh threshold %d\r\n",
        		ret, max_bitflip, chip->refresh_threshold);
    }

    if (failed) //TODO if failed is true, ret is NAND_CODE_EBADMSG
    {
        ret = AZX_SPI_FLASH_CODE_EBADMSG;
    }

    if (ecc_off)
    {
        spiEnableEcc(chip);
    }

    return ret;
}
/***************** spiDoReadOps ***********************************************/


/*******************************************************************************
                  spiTransferAutoOob
Function:
Arguments:
 	 	 @chip: SPI-NAND device structure
 	 	 @oob:  oob destination address
 	 	 @len:  size of oob to transfer
 	 	 @offs: offset of oob data in the oob area (only used for MTD_OPS_AUTO_OOB)
Return Value:
Description: transfer oob to client buffer
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiTransferAutoOob(AZX_SPI_FLASH_SPI_CHIP *chip,
											 JSC_uint8 *oob,
											 JSC_size_t len,
											 JSC_uint32 oob_offset)
{
	AZX_SPI_FLASH_OOBFREE *free = chip->ecclayout->oobfree;
	JSC_uint32 boffs = 0, roffs = oob_offset;
	JSC_size_t bytes = 0;

//	NAND_CODE_RESULT_E ret; //TODO not used

	for (; free->length && len; free++, len -= bytes) {
		/* Read request not from offset 0? */
		if (roffs)
		{
			if (roffs >= free->length)
			{
				roffs -= free->length;
				continue;
			}
			boffs = free->offset + roffs;
			bytes = min(len, (free->length - roffs));
			roffs = 0;
		}
		else
		{
			bytes = min(len, free->length);
			boffs = free->offset;
		}
		azx_spi_flash_memcpy(oob, chip->oobbuf + boffs, bytes);
		oob += bytes;
	}

    return AZX_SPI_FLASH_CODE_PASS;
}
/***************** spiTransferAutoOob ***********************************************/


/*******************************************************************************
                  spiDoReadPage
Function:
Arguments:
		@chip: spi nand chip structure
 	 	@page_addr: page address/raw address
 	 	@column: column address
 	 	@ecc_off: without ecc or not
 	 	@corrected: how many bit error corrected
		@buf: data buffer
 	 	@len: data length to read
Return Value:
		EBADMSG when internal ecc can not correct bitflips.
Description: read page from flash to buffer
 *   The command sequence to transfer data from NAND array to output is
 *   follows:
 *      13h (PAGE READ to cache register)
 *      0Fh (GET FEATURES command to read the status)
 *      0Bh/03h/3Bh/6Bh (Read from Cache Xn); or BBh/EBh (Read From
 *      Cache Dual/Quad IO)

*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiDoReadPage(AZX_SPI_FLASH_SPI_CHIP *chip,
										JSC_uint32 page_addr,
										JSC_uint32 column,
										JSC_bool ecc_off,
										JSC_uint32 *corrected,
										JSC_uint8 *buf,
										JSC_size_t len)
{
	AZX_SPI_FLASH_CODE_RESULT_E	ret, ret_rfc;
	JSC_uint32 ecc_error;
	JSC_uint8 status;

	ret_rfc = spiReadPageToCache(chip, page_addr);
	EXIT_IF_ERROR(ret_rfc, "spiReadPageToCache error!");

    ret = waitForReady(chip, &status);
	EXIT_IF_ERROR(ret, "waitForReady error!");

    if (!ecc_off)
    {
        spiEccStatus(status, corrected, &ecc_error);
        if (ecc_error)
        {
        	AZX_SPI_FLASH_LOG_INFO("Internal ECC error reading page 0x%x\n", page_addr);
            ret = AZX_SPI_FLASH_CODE_EBADMSG;
        }
    }

    ret_rfc = spiReadFromCache(chip, column, len, buf);
    EXIT_IF_ERROR(ret_rfc, "spiReadFromCache error!");

    return ret;
}
/***************** spiDoReadPage ***********************************************/


/*******************************************************************************
                  spiReadFromCache
Function:
Arguments:
		@chip: SPI-NAND device structure
		@column: the location to read from the cache
 	 	@len: number of bytes to read
 	 	@rbuf: buffer held @len bytes
Return Value:
Description: read data out from cache register
 *   Command can be 03h, 0Bh, 3Bh, 6Bh, BBh, EBh
 *   The read can specify 1 to (page size + spare size) bytes of data read at
 *   the corresponding locations.
 *   No tRd delay.
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReadFromCache(AZX_SPI_FLASH_SPI_CHIP *chip,
										   JSC_uint32 column,
										   JSC_size_t len,
										   JSC_uint8 *rbuf)
{
    SPI_FLASH_CMD cmd;

    azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
    cmd.cmd = chip->read_cache_op;
    cmd.n_addr = 2;
    cmd.addr[0] = (JSC_uint8)(column >> 8);
    cmd.addr[1] = (JSC_uint8)(column);
    cmd.n_rx = len;
    cmd.rx_buf = rbuf;

    return spiIssueCmd(chip, &cmd);
}
/***************** spiReadFromCache ***********************************************/


/*******************************************************************************
                  spiEccStatus
Function:
Arguments:
		@status: status register value to decode
		@corrected: bitflip count that ecc corrected
		@ecc_error: uncorrected bitflip happen or not
Return Value:
Description: decode status regisger to get ecc info
*******************************************************************************/
static void spiEccStatus(JSC_uint32 status, JSC_uint32 *corrected, JSC_uint32 *ecc_error)
{
	JSC_uint32 ecc_status;

	ecc_status = status & ECC_MASK;
	*ecc_error = (ecc_status == ECC_UNCORR);
	*corrected = (ecc_status == ECC_0_BIT) ? 0 :
				 (ecc_status == ECC_1_BIT) ? 1 :
				 (ecc_status == ECC_2_BIT) ? 2 :
				 (ecc_status == ECC_3_BIT) ? 3 :
				 (ecc_status == ECC_4_BIT) ? 4 : 0;
}
/***************** spiEccStatus ***********************************************/


/*******************************************************************************
                  spiReadPageToCache
Function:
Arguments:
		@chip: SPI-NAND device structure
		@page_addr: page to read
Return Value:
Description: send command 13h to read data from Nand to cache
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReadPageToCache(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 page_addr)
{
    SPI_FLASH_CMD cmd;
    azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));

    cmd.cmd = OPCODE_CMD_READ_PAGE;
    cmd.n_addr = 3;
    cmd.addr[0] = (JSC_uint8)(page_addr >> 16);
    cmd.addr[1] = (JSC_uint8)(page_addr >> 8);
    cmd.addr[2] = (JSC_uint8)(page_addr);

    return spiIssueCmd(chip, &cmd);

}
/***************** spiReadPageToCache ***********************************************/


/*******************************************************************************
                  spiWriteEnable
Function:
Arguments:
		@chip: SPI-NAND device structure
Return Value:
Description: send command 06h to enable write or erase the Nand cells
 *   Before write and erase the Nand cells, the write enable has to be set.
 *   After the write or erase, the write enable bit is automatically
 *   cleared (status register bit 2)
 *   Set the bit 2 of the status register has the same effect
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiWriteEnable(AZX_SPI_FLASH_SPI_CHIP *chip)
{
    SPI_FLASH_CMD cmd;

    azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
    cmd.cmd = OPCODE_CMD_WRITE_ENABLE;

    return spiIssueCmd(chip, &cmd);
}
/***************** spiWriteEnable ***********************************************/


/*******************************************************************************
                  spiEraseBlock
Function:
Arguments:
		@chip: SPI-NAND device structure
		@page_addr: the page to erase.
Return Value:
Description: send command D8h to erase a block
Need to wait for tERS.
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiEraseBlock(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 page_addr)
{
	SPI_FLASH_CMD cmd;

	azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
	cmd.cmd = OPCODE_CMD_BLOCK_ERASE;
	cmd.n_addr = 3;
	cmd.addr[0] = (JSC_uint8)(page_addr >> 16);
	cmd.addr[1] = (JSC_uint8)(page_addr >> 8);
	cmd.addr[2] = (JSC_uint8)(page_addr);

	return spiIssueCmd(chip, &cmd);
}
/***************** spiEraseBlock ***********************************************/


/*******************************************************************************
                  spiErase
Function:
Arguments:
 	 @chip: spi nand device structure
 	 @addr: address that erase start with, should be blocksize aligned
 	 @len: length that want to be erased, should be blocksize aligned
Return Value:
Description: [Interface] erase block(s)
Erase one ore more blocks
 *   The command sequence for the BLOCK ERASE operation is as follows:
 *       06h (WRITE ENBALE command)
 *       D8h (BLOCK ERASE command)
 *       0Fh (GET FEATURES command to read the status register)
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiErase(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint64 addr, JSC_uint64 len)
{

	//len = chip->block_size
	JSC_int32 page_addr, pages_per_block;
	JSC_uint8 status;
	AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_FAIL;
	JSC_int32 isBad = false;


//    AZX_SPI_FLASH_LOG_INFO("%s: address = 0x%012llx, len = %llu\r\n", __func__, addr, len);
    AZX_SPI_FLASH_LOG_INFO("%s: address = %llu, len = %llu\r\n", __func__, addr, len);


    /* check address align on block boundary */
    if (addr & (chip->block_size - 1))
    {
    	AZX_SPI_FLASH_LOG_ERROR("%s: Unaligned address\n", __func__);
    	JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_EINVAL);
        return AZX_SPI_FLASH_CODE_EINVAL;
    }

    if (len & (chip->block_size - 1))
    {
    	AZX_SPI_FLASH_LOG_ERROR("%s: Length not block aligned\n", __func__);
    	JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_EINVAL);
        return AZX_SPI_FLASH_CODE_EINVAL;
    }

    /* Do not allow erase past end of device */
    if ((len + addr) > chip->size)
    {
    	AZX_SPI_FLASH_LOG_ERROR("%s: Erase past end of device\n", __func__);
    	JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_EINVAL);
        return AZX_SPI_FLASH_CODE_EINVAL;
    }

    pages_per_block = 1 << (chip->block_shift - chip->page_shift);

    //obtain fro the address the number of the pages
    page_addr = addr >> chip->page_shift;

    while (len)
    {
        /* Check if we have a bad block, we do not erase bad blocks! */
    	JSC_loff_t addressOfPage = ((JSC_loff_t) page_addr) << chip->page_shift;
        ret = spiBlockIsBad(chip, addressOfPage, &isBad);
    	if (ret != AZX_SPI_FLASH_CODE_PASS)
        {
      		if (ret == AZX_SPI_FLASH_CODE_EBADMSG && isBad == true)
      		{
    			AZX_SPI_FLASH_LOG_ERROR("%s: attempt to erase a bad block at 0x%012llx\n",
    						  __func__, addressOfPage);
    			break;
      		}
    		else
			{
            	AZX_SPI_FLASH_LOG_ERROR("$s: spiBlockIsBad error \r\n", __func__);
                break;
			}
        }

    	ret = spiWriteEnable(chip);
    	if (ret != AZX_SPI_FLASH_CODE_PASS)
		{
			AZX_SPI_FLASH_LOG_ERROR("%s: attempt to enable write bit\n");
			   break;
		}

    	ret = spiEraseBlock(chip, page_addr);
    	if (ret != AZX_SPI_FLASH_CODE_PASS)
		{
    		AZX_SPI_FLASH_LOG_ERROR("%s: attempt to erase a block at 0x%012llx\n",
    		        	  __func__, addressOfPage);
    		break;
		}

    	ret = waitForReady(chip, &status);
        if (ret != AZX_SPI_FLASH_CODE_PASS)
        {
        	AZX_SPI_FLASH_LOG_ERROR("block erase command wait failed\n");
            break;
        }

        if ((status & STATUS_E_FAIL_MASK) == STATUS_E_FAIL)
        {
        	AZX_SPI_FLASH_LOG_ERROR("erase block 0x%012llx failed\n", addressOfPage);
            ret = AZX_SPI_FLASH_CODE_EIO;
            break;
        }

        /* Increment page address and decrement length */
        len -= (1ULL << chip->block_shift);
        page_addr += pages_per_block;

    }

    return ret;
}
/***************** spiErase ***********************************************/


/*******************************************************************************
                  spiCheckOps
Function:
Arguments:
 	 * @chip: spi nand device structure
 	 * @offs: offset to access
 	 * @ops: oob operations description structure
Return Value:
Description: check oob structure

*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiCheckOps(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t offs, AZX_SPI_FLASH_MTD_OOB_OPS *ops)
{
	int ooblen;// = (ops->mode == MTD_OPS_AUTO_OOB) ? chip->ecclayout->oobavail : chip->oob_size;

	if (ops->mode == MTD_OPS_AUTO_OOB)
	{
		ooblen = chip->ecclayout->oobavail;
	}
	else
	{
		ooblen = chip->oob_size;
	}

	ops->retlen = 0;
	ops->oobretlen = 0;

    /* Do not allow null for databuf & oobbuf */
    if (!ops->datbuf && !ops->oobbuf)
    {
        AZX_SPI_FLASH_LOG_ERROR("%s: null buffer\r\n", __func__);
        return AZX_SPI_FLASH_CODE_EINVAL;
    }

    if (ops->mode > MTD_OPS_RAW) {
    	AZX_SPI_FLASH_LOG_ERROR("%s: not supported mode\r\n", __func__);
        return AZX_SPI_FLASH_CODE_ENOTSUPP;
    }

    /*
    * Some users are setting ->datbuf or ->oobbuf to NULL, but are leaving
    * ->len or ->ooblen uninitialized. Force ->len and ->ooblen to 0 in
    *  this case.
    */
    if (!ops->datbuf)
    {
        ops->len = 0;
    }

    if (!ops->oobbuf)
    {
        ops->ooblen = 0;
    }

//    JSC_LOG_INFO("%s: offs = 0x%012llx, len = %i, ooblen = %i\r\n",
//        __func__, offs, ops->len, ops->ooblen);

    /* Do not allow writes past end of device */
    if (offs < 0 || (((JSC_size_t) offs) + ops->len) > chip->size)
    {
        AZX_SPI_FLASH_LOG_ERROR("%s: attempt to access beyond end of device\r\n",
            __func__);
        return AZX_SPI_FLASH_CODE_EINVAL;
    }

    if (ooblen > 0)
    {
        if (ops->ooboffs >= (JSC_uint32) ooblen)
        {
        	AZX_SPI_FLASH_LOG_ERROR("%s: attempt to access write outside oob\n",
                __func__);
            return AZX_SPI_FLASH_CODE_EINVAL;
        }
        if (ops->ooboffs + ooblen >
            ((chip->size >> chip->page_shift) - (offs >> chip->page_shift))
            * ooblen)
        {
        	AZX_SPI_FLASH_LOG_ERROR("%s: attempt to access beyond end of device\n",
                __func__);
            return AZX_SPI_FLASH_CODE_EINVAL;
        }
    }

    return AZX_SPI_FLASH_CODE_PASS;

}
/***************** spiCheckOps ***********************************************/


/*******************************************************************************
                  spiDoWriteOps
Function:
Arguments:
 	 * @chip: spi nand device structure
 	 * @to: offset to write to
 	 * @ops: oob operations description structure
Return Value:
Description:
	   Disable internal ECC before writing when MTD_OPS_RAW set.

*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiDoWriteOps(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t to, AZX_SPI_FLASH_MTD_OOB_OPS *ops)
{
    int page_addr, page_offset, size, oobsize;
    int writelen = ops->len;
    int oobwritelen = ops->ooblen;
    AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_FAIL;
    int ooblen = (ops->mode == MTD_OPS_AUTO_OOB) ?
        chip->ecclayout->oobavail : chip->oob_size;
    int oob_offset = ops->ooboffs;
    JSC_bool ecc_off = ops->mode == MTD_OPS_RAW;

    page_addr = to >> chip->page_shift;
    page_offset = to & chip->page_mask;

    if (ecc_off)
    {
        spiDisableEcc(chip);
    }

    while (writelen || oobwritelen)
    {
        size = 0;
        oobsize = 0;

        if (writelen)
        {
            size = min(writelen, chip->page_size - page_offset);
            azx_spi_flash_memcpy(chip->datbuf + page_offset,
                ops->datbuf + ops->retlen, size);
            if (page_offset)
            {
            	azx_spi_flash_memset(chip->datbuf, 0xff, page_offset);
            }
            if (size < chip->page_size - page_offset)
            {
                azx_spi_flash_memset(chip->datbuf + page_offset + size, 0xFF,
                chip->page_size - page_offset - size);
            }
        }

        if (oobwritelen)
        {
            oobsize = min(oobwritelen, ooblen - oob_offset);
            if (ops->mode == MTD_OPS_AUTO_OOB)
                spiFillAutoOob(chip, ops->oobbuf + ops->oobretlen,
                oobsize, oob_offset);
            else {
                azx_spi_flash_memset(chip->oobbuf, 0xff, chip->oob_size);
                azx_spi_flash_memcpy(chip->oobbuf + oob_offset,
                    ops->oobbuf + ops->oobretlen, oobsize);
            }
        }

		JSC_uint32 column;  // = size ? 0 : chip->page_size;
		JSC_uint8 *buf; 	// = size ? chip->datbuf : chip->oobbuf;
		JSC_size_t len;		// = (size ? chip->page_size : 0) + (oobsize ? chip->oob_size : 0);

		if (size)
		{
			column = 0;
			buf = chip->datbuf;
			len = chip->page_size;
		}
		else
		{
			column = chip->page_size;
			buf = chip->oobbuf;
			len = 0;
		}

		if (oobsize)
		{
			len += chip->oob_size;
		}

        ret = spiDoWritePage(chip, page_addr, column, buf, len, true);

        if (ret != AZX_SPI_FLASH_CODE_PASS)
        {
            AZX_SPI_FLASH_LOG_ERROR("error %d writing page 0x%x\n", ret, page_addr);
            break;
        }

        ops->oobretlen += oobsize;
		oobwritelen -= oobsize;

		ops->retlen += size;
		writelen -= size;

		page_offset = 0;
		oob_offset = 0;
		page_addr++;
    }

    if (ecc_off)
    {
        spiEnableEcc(chip);
    }

    return ret;

}
/***************** spiDoWriteOps ***********************************************/


/*******************************************************************************
                  spiDoWritePage

Function: spiDoWritePage
Arguments:
		* @chip: spi nand chip structure
		* @page_addr: page address/raw address
		* @column: column address
		* @buf: data buffer
		* @len: data length to write
		* @clr_cache: clear cache register with 0xFF or not
Return Value:
Description: write data from buffer to flash
 *   Page program sequence is as follows:
 *       06h (WRITE ENABLE)
 *       02h/32h/84h/34h (PROGRAM LOAD (RAMDOM_DATA) Xn)
 *       10h (PROGRAM EXECUTE)
 *       0Fh (GET FEATURE command to read the status)
 *   PROGRAM LOAD Xn instruction will reset the cache resigter with 0xFF,
 *   while PROGRAM LOAD RANDOM DATA Xn instruction will only update the
 *   data bytes that are specified by the command input sequence and the rest
 *   of data in the cache buffer will remain unchanged.
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiDoWritePage(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 page_addr, JSC_uint32 column, JSC_uint8 *buf, JSC_size_t len, JSC_bool clr_cache)
{
	JSC_uint8 status;
//	JSC_bool p_fail = false;
	AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_FAIL;

    ret = spiWriteEnable(chip);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	AZX_SPI_FLASH_LOG_ERROR("spiWriteEnable fails!\r\n");
    	return ret;
    }

    ret = spiProgramDataToCache(chip, column, len, buf, clr_cache);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	AZX_SPI_FLASH_LOG_ERROR("spiProgramDataToCache fails!\r\n");
    	return ret;
    }

    spiProgramExecute(chip, page_addr);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	AZX_SPI_FLASH_LOG_ERROR("spiProgramExecute fails!\r\n");
    	return ret;
    }

    ret = waitForReady(chip, &status);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
    	AZX_SPI_FLASH_LOG_ERROR("error %d writing page 0x%x from cache\n", ret, page_addr);
        return ret;
    }

    if ((status & STATUS_P_FAIL_MASK) == STATUS_P_FAIL)
    {
    	AZX_SPI_FLASH_LOG_ERROR("program page 0x%x failed\n", page_addr);
    	return AZX_SPI_FLASH_CODE_EIO;
    }

    return AZX_SPI_FLASH_CODE_PASS;

}
/***************** spiDoWritePage ***********************************************/


/*******************************************************************************
                  spiProgramDataToCache

Function: spiDoWritePage
Arguments:
		 * @chip: SPI-NAND device structure
		 * @column: the location to write to the cache
		 * @len: number of bytes to write
		 * @wrbuf: buffer held @len bytes
		 * @clr_cache: clear cache register or not
Return Value:
Description:	write data to cache register
 *   Command can be 02h, 32h, 84h, 34h
 *   02h and 32h will clear the cache with 0xff value first
 *   Since it is writing the data to cache, there is no tPROG time.
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiProgramDataToCache(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 column, JSC_size_t len, JSC_uint8 *wbuf, JSC_bool clr_cache)
{
	SPI_FLASH_CMD cmd;

	azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
	if (clr_cache)
	{
		cmd.cmd = chip->write_cache_op;
	}
	else
	{
		cmd.cmd = chip->write_cache_rdm_op;
	}
	cmd.n_addr = 2;
	cmd.addr[0] = (JSC_uint8)(column >> 8);
	cmd.addr[1] = (JSC_uint8)(column);
	cmd.n_tx = len;
	cmd.tx_buf = wbuf;

	return spiIssueCmd(chip, &cmd);
}
/***************** spiProgramDataToCache ***********************************************/


/*******************************************************************************
                  spiProgramExecute
Function:
Arguments:
		* cache to the Nand array
		* @chip: SPI-NAND device structure
		* @page_addr: the physical page location to write the page.
Return Value:
Description: send command 10h to write a page from
Need to wait for tPROG time to finish the transaction.

*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiProgramExecute(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 page_addr)
{
	SPI_FLASH_CMD cmd;

	azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
	cmd.cmd = OPCODE_CMD_PROGRAM_EXECUTE;
	cmd.n_addr = 3;
	cmd.addr[0] = (JSC_uint8)(page_addr >> 16);
	cmd.addr[1] = (JSC_uint8)(page_addr >> 8);
	cmd.addr[2] = (JSC_uint8)(page_addr);

	return spiIssueCmd(chip, &cmd);
}
/***************** spiProgramExecute ***********************************************/


/*******************************************************************************
                  spiReadOob
Function:
Arguments:
		* @chip: spi nand device structure
		* @from: offset to read from
		* @ops: oob operation description structure
Return Value:
Description: [Interface] SPI-NAND read data and/or out-of-band

*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReadOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t from, AZX_SPI_FLASH_MTD_OOB_OPS *ops)
{
	AZX_SPI_FLASH_CODE_RESULT_E ret;

    ret = spiCheckOps(chip, from, ops);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
        return ret;
    }

    return spiDoReadOps(chip, from, ops);
}
/***************** spiReadOob ***********************************************/


/*******************************************************************************
                  spiReadUIDpage
Function:
Arguments:
			chip: 	spi nand chip structure
 	 	 	column: column address
			buf:	data buffer
			len: 	data length to read
Return Value:
Description:	[Interface] read UID page from flash to buffer
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E spiReadUIDpage(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 column, JSC_uint8 *buf, JSC_size_t len)
{
	AZX_SPI_FLASH_CODE_RESULT_E ret;
	JSC_uint8 status;

	ret = spiReadUIDPageToCache(chip);
  	if (ret != AZX_SPI_FLASH_CODE_PASS)
  	{
  		/* log error message */
  		JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_BAD_UID);
  		return AZX_SPI_FLASH_CODE_BAD_UID;
  	}

	ret = waitForReady(chip, &status);
  	if (ret != AZX_SPI_FLASH_CODE_PASS)
  	{
  		JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_BAD_UID);
  		return AZX_SPI_FLASH_CODE_BAD_UID;
  	}

	ret = spiReadFromCache(chip, column, len, buf);
  	if (ret != AZX_SPI_FLASH_CODE_PASS)
  	{
  		JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_BAD_UID);
  		return AZX_SPI_FLASH_CODE_BAD_UID;
  	}

    return ret;
}
/**************  spiReadUIDpage **********************************************/


/*******************************************************************************
                  spiReadUIDPageToCache
Function:
Arguments:
			chip: 	spi nand chip structure
Return Value:
Description:	send command EDh to read UID data from Nand to cache
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReadUIDPageToCache(AZX_SPI_FLASH_SPI_CHIP *chip)
{
    SPI_FLASH_CMD cmd;

    azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
    cmd.cmd = OPCODE_CMD_READ_UID;
    cmd.n_addr = 1;
    cmd.addr[0] = 0;

    return spiIssueCmd(chip, &cmd);
}
/**************  spiReadUIDPageToCache **********************************************/


/*******************************************************************************
                  spiReadParameterPageToCache
Function:
Arguments:
			chip: 	spi nand chip structure
Return Value:
Description:	send command ECh to read parameter data from Nand to cache
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReadParameterPageToCache(AZX_SPI_FLASH_SPI_CHIP *chip)
{
    SPI_FLASH_CMD cmd;

    azx_spi_flash_memset(&cmd, 0, sizeof(SPI_FLASH_CMD));
    cmd.cmd = OPCODE_CMD_READ_PARAMETER_PAGE;
    cmd.n_addr = 1;
    cmd.addr[0] = 0;

    return spiIssueCmd(chip, &cmd);
}
/**************  spiReadUIDPageToCache **********************************************/


/*******************************************************************************
                  onfiCrc16
Function:
Arguments:
			crc:
			p:
			len:
Return Value:
Description:
*******************************************************************************/
static JSC_uint16 onfiCrc16(JSC_uint16 crc, JSC_uint8 *p, JSC_size_t len)
{
    int i;

    while (len--)
    {
        crc ^= *p++ << 8;
        for (i = 0; i < 8; i++)
        {
            crc = (crc << 1) ^ ((crc & 0x8000) ? 0x8005 : 0);
        }
    }

    return crc;
}
/**************  onfiCrc16 **********************************************/


/*******************************************************************************
                  spiCheckParameter
Function:
Arguments:
			buffer: data
Return Value:
Description:	This function is called when we can not get info from id table.
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiCheckParameter(AZX_SPI_FLASH_ONFI_PARAMS *p)
{
    int i;
    for (i = 0; i < 3; i++, p++)
    {
    	if (p->sig[0] != 'O' ||
    		p->sig[1] != 'N' ||
			p->sig[2] != 'F' ||
			p->sig[3] != 'I')
    	{
    		continue;
    	}
        if (onfiCrc16(ONFI_CRC_BASE, (JSC_uint8 *)p, 254) == le16_to_cpu(p->crc))
        {
        	break;
        }
    }

    if (i == 3)
    {
		JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_ONFI);
    	return AZX_SPI_FLASH_CODE_ONFI;
    }

    return AZX_SPI_FLASH_CODE_PASS;
}
/**************  spiCheckParameter **********************************************/


/*******************************************************************************
                  spiReadParameterPage
Function:
Arguments:
			chip: spi nand chip structure
			column: column address
			buf: data buffer
			len: data length to read
Return Value:
Description:	[Interface] read parameter page from flash to buffer
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiReadParameterPage(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 column, JSC_uint8 *buf, JSC_size_t len)
{
	AZX_SPI_FLASH_CODE_RESULT_E ret;
	JSC_uint8 status;


	ret = spiReset(chip);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
  		JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_PARAMETER);
        return ret;
    }

	ret = spiReadParameterPageToCache(chip);
	if (ret != AZX_SPI_FLASH_CODE_PASS)
	{
  		JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_PARAMETER);
		return AZX_SPI_FLASH_CODE_PARAMETER;
	}

	ret = waitForReady(chip, &status);
	if (ret != AZX_SPI_FLASH_CODE_PASS)
	{
		JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_PARAMETER);
		return AZX_SPI_FLASH_CODE_PARAMETER;
	}

	ret = spiReadFromCache(chip, column, len, buf);
	if (ret != AZX_SPI_FLASH_CODE_PASS)
	{
		return AZX_SPI_FLASH_CODE_PARAMETER;
	}

	return ret;
}
/**************  spiReadParameterPage **********************************************/


/*******************************************************************************
                  spiFillAutoOob
Function:
Arguments:
 	 * @chip: SPI-NAND device structure
 	 * @oob: oob data buffer
 	 * @len: oob data write length
 	 * @offs:  offset of oob data in the oob area (only used for MTD_OPS_AUTO_OOB)
Return Value:
Description: transfer client buffer to oob
*******************************************************************************/
static AZX_SPI_FLASH_CODE_RESULT_E spiFillAutoOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *oob, JSC_size_t len, JSC_uint32 oob_offset)
{
    AZX_SPI_FLASH_OOBFREE *free = chip->ecclayout->oobfree;
    JSC_uint32 boffs = 0, woffs = oob_offset;
    JSC_size_t bytes = 0;

    azx_spi_flash_memset(chip->oobbuf, 0xFF, chip->oob_size);

    for (; free->length && len; free++, len -= bytes)
    {
        /* Write request not from offset 0? */
        if (woffs)
        {
            if (woffs >= free->length)
            {
                woffs -= free->length;
                continue;
            }
            boffs = free->offset + woffs;
            bytes = min(len, (free->length - woffs));
            woffs = 0;
        }
        else
        {
            bytes = min(len, free->length);
            boffs = free->offset;
        }
        azx_spi_flash_memcpy(chip->oobbuf + boffs, oob, bytes);
        oob += bytes;
    }

    return AZX_SPI_FLASH_CODE_PASS;
}
/***************** spiFillAutoOob ***********************************************/


/*******************************************************************************
                  spiWriteOob
Function:
Arguments:
 	 * @chip: spi nand device structure
 	 * @to: offset to write to
 	 * @ops: oob operation description structure
Return Value:
Description: [Interface] SPI-NAND write data and/or out-of-band

*******************************************************************************/

static AZX_SPI_FLASH_CODE_RESULT_E spiWriteOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t to, AZX_SPI_FLASH_MTD_OOB_OPS *ops)
{
	AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_FAIL;

    ret = spiCheckOps(chip, to, ops);
    if (ret != AZX_SPI_FLASH_CODE_PASS)
    {
        return ret;
    }

    return spiDoWriteOps(chip, to, ops);
}
/***************** spiWriteOob ***********************************************/


/*******************************************************************************
                  errorHandlerAndFreeMemory
Function:		void errorHandlerAndFreeMemory(const NAND_CODE_RESULT_E ret, int num, const char *ptr, ...)
Arguments:
				ret: error code value
				num: number of ptr pointers
				ptr: pointers to memory free
Return Value:
Description:	Free the memory of the pointers passed as variable parameters
				and log the message associated at the error code
*******************************************************************************/
void errorHandlerAndFreeMemory(const AZX_SPI_FLASH_CODE_RESULT_E ret, int num, const void *ptr, ...)
{
	void *pointer;
	va_list valist;

	/* initialize valist for ptr pointers of arguments to free */
	va_start(valist, ptr);

	/* access all the arguments assigned to valist */
	for (int i = 0; i < num; i++)
	{
		pointer = va_arg(valist, void*);
    	azx_spi_flash_free(pointer);
	}

	/* clean memory reserved for valist */
	va_end(valist);


	/* log error message */
	JSC_LOG_ERROR_MSG(ret);
}
/**************  errorHandlerAndFreeMemory **********************************************/


/* Global functions definition ===================================================================*/

/*******************************************************************************
                  NAND_lockBlock
Function:
Arguments:
Return Value:
Description:
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_lockBlock(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 lock)
{
	if (isInitialize(chip) == true)
	{
		return spiLockBlock(chip, lock);
	}
	return AZX_SPI_FLASH_CODE_FAIL;
}
/***************** NAND_lockBlock ***********************************************/


/*******************************************************************************
                  NAND_readID
Function:
Arguments:
Return Value:
Description:
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_readID(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *buf)
{
	if (isInitialize(chip) == true)
	{
	    if (spiReset(chip) != AZX_SPI_FLASH_CODE_PASS)
	    {
			AZX_LOG_ERROR("Nand reset failed!!\r\n");
			return AZX_SPI_FLASH_CODE_FAIL;
	    }
		return spiReadID (chip, buf);
	}
	return AZX_SPI_FLASH_CODE_FAIL;
}
/**************  NAND_readID **********************************************/


/*******************************************************************************
                  NAND_initialize
Function:
Arguments:	@spi: spi device structure
       	   	   	  will be referenced by spi_xfer(struct spi_slave *slave..)
       	   	   	  CS_GPIO_pin:		the GPIO pin number used in the board
       	   	   	  opts:				information about the interfaced used
       	   	  	  spi->hspi : spi and gpio device handle
       	   	  	  spi->op_mode_rx : spi operation mode for rx
 	 	 	 	 	 	 	 	 	SPI_OPM_RX_SING - Support 1 pin RX
 	 	 	 	 	 	 	 	 	SPI_OPM_RX_DUAL - Support 2 pin RX
      	  	  	  	  	  	  	  	SPI_OPM_RX_QUAD - Support 4 pin RX
    			  spi->op_mode_tx : spi operation mode for tx
      	  	  	  	  	  	  	  	SPI_OPM_TX_SING - Support 1 pin TX
      	  	  	  	  	  	  	  	SPI_OPM_TX_QUAD - Support 4 pin TX
      	  	  	  chip_ptr: 		pointer point to spi nand device structure pointer
Return Value:
Description: [Interface] Init SPI-NAND device driver
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_initialize( const JSC_uint8 CS_GPIO_pin,
									AZX_SPI_FLASH_MICRO_SPI_PARAMS *opts,
									AZX_SPI_FLASH_SPI_CHIP **chip_ptr)
{
	AZX_SPI_FLASH_CODE_RESULT_E	ret;
    AZX_SPI_FLASH_SPI_CHIP *chip = NULL;
	AZX_SPI_FLASH_SPI_SLAVE *spi = NULL;
	AZX_SPI_FLASH_MICRO_SPI_PARAMS *micro_params = NULL;
//	JSC_int32 *fd_gpio = NULL;
//	JSC_int32 *fd_spi = NULL;

	//TODO: free all allocated resources in case of error
	//TODO: sets default values

	AZX_SPI_FLASH_LOG_DEBUG("Start NAND_initialize\r\n");

	chip = (AZX_SPI_FLASH_SPI_CHIP*) azx_spi_flash_malloc(sizeof(AZX_SPI_FLASH_SPI_CHIP));
	if (!chip)
	{
    	JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_MALLOC_FAIL);
		return AZX_SPI_FLASH_CODE_MALLOC_FAIL;
	}
	azx_spi_flash_memset(chip, 0, sizeof(AZX_SPI_FLASH_SPI_CHIP));

	micro_params = (AZX_SPI_FLASH_MICRO_SPI_PARAMS*) azx_spi_flash_malloc(sizeof(AZX_SPI_FLASH_MICRO_SPI_PARAMS));
	if (!micro_params)
	{
    	JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_MALLOC_FAIL);
		return AZX_SPI_FLASH_CODE_MALLOC_FAIL;
	}
	azx_spi_flash_memset(micro_params, 0, sizeof(AZX_SPI_FLASH_MICRO_SPI_PARAMS));
	chip->opts = micro_params;
	chip->opts->init = false;

	chip->opts->clbk.clbkFunc = opts->clbk.clbkFunc;
    chip->opts->clbk.clbkHndl = opts->clbk.clbkHndl;

	spi = (AZX_SPI_FLASH_SPI_SLAVE*) azx_spi_flash_malloc(sizeof(AZX_SPI_FLASH_SPI_SLAVE));
	if (!spi)
	{
    	JSC_LOG_ERROR_MSG(AZX_SPI_FLASH_CODE_MALLOC_FAIL);
		return AZX_SPI_FLASH_CODE_MALLOC_FAIL;
	}
	azx_spi_flash_memset(spi, 0, sizeof(AZX_SPI_FLASH_SPI_SLAVE));
	chip->spi = spi;
	chip->spi->op_mode_rx = opts->spi.op_mode_rx;
	chip->spi->op_mode_tx = opts->spi.op_mode_tx;
	chip->spi->hspi = &chip->opts->fd;

	setRdWrMode(chip);

	ret = azx_spi_flash_spi_initialization(&chip->opts->fd.spi);
    if (ret < 0)
    {
    	errorHandlerAndFreeMemory(ret, 2, spi, chip);
        return ret;
    }

	ret = azx_spi_flash_gpio_initialization(CS_GPIO_pin, &chip->opts->fd.gpio);
    if (ret < 0)
    {
    	errorHandlerAndFreeMemory(ret, 2, spi, chip);
        return ret;
    }

	ret = spiReset(chip);
    if (ret < 0)
    {
    	errorHandlerAndFreeMemory(ret, 2, spi, chip);
        return ret;
    }

    //delay error
    //	azx_spi_flash_delay(FLASH_DELAY_FOR_READY); //300


	JSC_uint8 id[3] = {0};
	ret = spiReadID(chip, id);
    if (ret < 0)
    {
    	errorHandlerAndFreeMemory(ret, 2, spi, chip);
        return ret;
    }
//    AZX_LOG_INFO("\r\n");
//	AZX_LOG_INFO("id[0] = %d\r\n", id[0]);
//	AZX_LOG_INFO("id[1] = %d\r\n", id[1]);
//	AZX_LOG_INFO("id[2] = %d\r\n", id[2]);
//	AZX_LOG_INFO("\r\n");
//
//	NAND_memset(id, 0x00, sizeof(id));
//	ret = spiReadID(chip, id);
//    if (ret < 0)
//    {
//    	errorHandlerAndFreeMemory(ret, 2, spi, chip);
//        return ret;
//    }
//    AZX_LOG_INFO("\r\n");
//	AZX_LOG_INFO("id[0] = %d\r\n", id[0]);
//	AZX_LOG_INFO("id[1] = %d\r\n", id[1]);
//	AZX_LOG_INFO("id[2] = %d\r\n", id[2]);
//	AZX_LOG_INFO("\r\n");


	ret = spiIdTable(chip, id);
	if (ret < 0)
	{
		errorHandlerAndFreeMemory(ret, 2, spi, chip);
		return ret;
	}

	ret = setEccProtection(chip);
	if (ret < 0)
	{
		errorHandlerAndFreeMemory(ret, 2, spi, chip);
		return ret;
	}

	/*unlock all block */
	ret = spiLockBlock(chip, BL_ALL_UNLOCKED);
	if (ret < 0)
	{
		errorHandlerAndFreeMemory(ret, 2, spi, chip);
		return ret;
	}

	ret = spiEnableEcc(chip);
	if (ret < 0)
	{
		errorHandlerAndFreeMemory(AZX_SPI_FLASH_CODE_ECC_FAIL, 2, spi, chip);
		return ret;
	}

	//TODO check chip mode
	ret = spiQuadEnable(chip);
	if (ret < 0)
	{
		errorHandlerAndFreeMemory(AZX_SPI_FLASH_CODE_QUAD_FAIL, 2, spi, chip);
		return ret;
	}

	chip->opts->init = true;
	*chip_ptr = chip;


	EMIT_EVENT(chip, AZX_SPI_FLASH_INITIALIZED);
	return AZX_SPI_FLASH_CODE_PASS;
}
/**************  NAND_initialize **********************************************/


/*******************************************************************************
                  NAND_configure
Function:
Arguments:
Return Value:
Description:
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_configure(AZX_SPI_FLASH_MICRO_SPI_PARAMS *opts, AZX_SPI_FLASH_SPI_CHIP **chip_ptr)
{
	(void) opts, (void) chip_ptr;
	return AZX_SPI_FLASH_CODE_PASS;
}
/**************  NAND_configure **********************************************/


/*******************************************************************************
                  NAND_callbak
Function:
Arguments:
Return Value:
Description:
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_callbak(AZX_SPI_FLASH_SPI_CHIP *chip, AZX_SPI_FLASH_JSC_EVENT_E event)
{
	if (isInitialize(chip) == true)
	{
		chip->opts->clbk.clbkFunc(NULL,	(AZX_SPI_FLASH_JSC_EVENT_E) event, sizeof(AZX_SPI_FLASH_JSC_EVENT_RES_T), NULL, NULL);
		return AZX_SPI_FLASH_CODE_PASS;
	}
	else
	{
		return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
	}
}
/**************  NAND_callbak **********************************************/


/*******************************************************************************
                  NAND_enableEcc
Function:
Arguments:
Return Value:
Description:
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_enableEcc(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	if (isInitialize(chip) == true)
	{
		return spiEnableEcc (chip);
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_enableEcc **********************************************/


/*******************************************************************************
                  NAND_disableEcc
Function:
Arguments:
Return Value:
Description:
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_disableEcc(AZX_SPI_FLASH_SPI_CHIP *chip)
{
	if (isInitialize(chip) == true)
	{
		return spiDisableEcc(chip);
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_disableEcc **********************************************/


/*******************************************************************************
                  NAND_releaseResources
Function:
Arguments:
Return Value:
Description:
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_releaseResources (AZX_SPI_FLASH_SPI_CHIP *chip)
{
	if (isInitialize(chip) == true)
	{
		//release SPI handle
		//release GPIO handle
		//release memory

		if (spiReleaseSPI(chip)	!= AZX_SPI_FLASH_CODE_PASS 	||
			spiReleaseGPIO(chip) 	!= AZX_SPI_FLASH_CODE_PASS 	||
			spiReleaseMemory(chip) != AZX_SPI_FLASH_CODE_PASS)
		{
			return AZX_SPI_FLASH_CODE_FAIL;
		}

		return AZX_SPI_FLASH_CODE_PASS;
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_releaseResources **********************************************/


/*******************************************************************************
                  NAND_blockIsBad
Function:
Arguments:
Return Value:
Description:
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_blockIsBad(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t block_addr, JSC_bool *result)
{
	if (isInitialize(chip) == true)
	{
		return spiBlockIsBad(chip, block_addr, result);
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_blockIsBad **********************************************/


/*******************************************************************************
                  NAND_erase
Function:	NAND_erase(SPI_NAND_CHIP *chip, JSC_uint64 addr, JSC_uint64 len)
Arguments:
			@chip: spi nand device structure
 	 	 	@addr: address that erase start with, should be blocksize aligned
 	 	 	@len: length that want to be erased, should be blocksize aligned
Return Value:
Description:	[Interface] erase block(s)
 *   Erase one ore more blocks
 *   The command sequence for the BLOCK ERASE operation is as follows:
 *       06h (WRITE ENBALE command)
 *       D8h (BLOCK ERASE command)
 *       0Fh (GET FEATURES command to read the status register)
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_erase(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint64 addr, JSC_uint64 len)
{
	if (isInitialize(chip) == true)
	{
		return spiErase(chip, addr, len);
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_erase **********************************************/


/*******************************************************************************
                  NAND_writeOob
Function:	NAND_writeOob(SPI_NAND_CHIP *chip, JSC_loff_t to, MTD_OOB_OPS *ops)
Arguments:
			chip: 	spi nand device structure
 	 	 	to: 	offset to write to
 	 	 	ops: 	oob operation description structure
Return Value:
Description:	[Interface] SPI-NAND write data and/or out-of-band
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_writeOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t to, AZX_SPI_FLASH_MTD_OOB_OPS *ops)
{
	if (isInitialize(chip) == true)
	{
		return spiWriteOob(chip, to, ops);
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_writeOob **********************************************/


/*******************************************************************************
                  Nand_readOob
Function:
Arguments:
			chip: 	spi nand device structure
 	 	 	from: 	offset to read from
			ops: 	oob operation description structure
Return Value:
Description:	[Interface] SPI-NAND read data and/or out-of-band
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_readOob(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_loff_t from, AZX_SPI_FLASH_MTD_OOB_OPS *ops)
{
	if (isInitialize(chip) == true)
	{
		return spiReadOob(chip, from, ops);
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_readOob **********************************************/


/*******************************************************************************
                  NAND_readUIDpage
Function:
Arguments:
			chip: 	spi nand chip structure
 	 	 	column: column address
			buf:	data buffer
			len: 	data length to read
Return Value:
Description:	[Interface] read UID page from flash to buffer
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_readUIDpage(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 column, JSC_uint8 *buf, JSC_size_t len)
{
	if (isInitialize(chip) == true)
	{
		return spiReadUIDpage(chip, column, buf, len);
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_readUIDpage **********************************************/


/*******************************************************************************
                  NAND_readParameterPage
Function:
Arguments:
			chip: spi nand chip structure
			column: column address
			buf: data buffer
			len: data length to read
Return Value:
Description:	[Interface] read parameter page from flash to buffer
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_readParameterPage(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint32 column, JSC_uint8 *buf, JSC_size_t len)
{
	if (isInitialize(chip) == true)
	{
		return spiReadParameterPage(chip, column, buf, len);
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_readParameterPage **********************************************/


/*******************************************************************************
                  NAND_waitForReady
Function:
Arguments:
		chip: spi nand chip structure
		s: buffer to store status register(can be NULL)
Return Value:
Description:	[Interface] wait until the command is done
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_waitForReady(AZX_SPI_FLASH_SPI_CHIP *chip, JSC_uint8 *s)
{
	if (isInitialize(chip) == true)
	{
		AZX_SPI_FLASH_CODE_RESULT_E ret = AZX_SPI_FLASH_CODE_FAIL;
		ret =  waitForReady(chip, s);
		if (ret == AZX_SPI_FLASH_CODE_PASS)
		{
			JSC_uint8 status = *s;
	        if ((status & STATUS_E_FAIL_MASK) == STATUS_E_FAIL)
	        {
	            ret = AZX_SPI_FLASH_CODE_EIO;
	        }
		}
		return ret;
	}
	return AZX_SPI_FLASH_CODE_NOT_INITIALIZED;
}
/**************  NAND_waitForReady **********************************************/


/*******************************************************************************
                  NAND_readParameterPage
Function:
Arguments:
			buffer: data
Return Value:
Description:	[Interface] This function is called when we can not get info from id table.
*******************************************************************************/
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_checkParameter(AZX_SPI_FLASH_ONFI_PARAMS *p)
{
	return spiCheckParameter(p);
}
/**************  NAND_readParameterPage **********************************************/


