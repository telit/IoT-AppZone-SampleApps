/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showing how to compress/uncompress with ZLIB. Debug prints on MAIN UART
  @version
    1.0.2
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
  	  Fabio Pintus
  	  Norman Argiolas


  @date
    21/14/2020
*/

/* Include files ================================================================================*/
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"


#include "azx_log.h"
#include "azx_utils.h"


#include "azx_zlib.h"

#include "azx_gnu_stdio.h"
#include "azx_gnu_fcntl.h"
#include "azx_gnu_unistd.h"


#include "app_cfg.h"


/* Local defines ================================================================================*/
#define CHUNK 128

/* Static defines =============================================================================*/
//message to compress and uncompress
static z_const char data[] =
		"the quick brown fox jumped over the lazy dog. the quick brown fox jumped over the lazy dog. the quick brown fox jumped over the lazy dog.";


/* Local typedefs ===============================================================================*/
typedef enum
{
	TEST_ERROR 	 = -1,
	TEST_SUCCESS =  1
} TEST_STATUS_E;

/* Local statics ================================================================================*/

/* Local function prototypes ====================================================================*/
int test_compress_uncompress(Byte *compr, uLong comprLen, Byte *uncompr, uLong uncomprLen);
int test_uncompress(char* source_S, char* dest_S);


/* Local macros =============================================================================*/
#define CHECK_ERR(err, msg) { 						\
    if (err != Z_OK) { 								\
        AZX_LOG_ERROR("%s error: %d\n", msg, err);  \
        return TEST_ERROR; 	   						\
    } 												\
}


/* Local function definitions ====================================================================*/

/* Test compress() and uncompress()*/
int test_compress_uncompress(Byte *compr, uLong comprLen, Byte *uncompr, uLong uncomprLen)
{
	int err;
	uLong i;
	uLong len = (uLong) strlen(data) + 1;

	err = azx_zlib_compress(compr, &comprLen, (const Bytef*) data, len);
	CHECK_ERR(err, "compress");

	AZX_LOG_INFO("len: %u; comprLen: %u\r\n", len, comprLen);
	strcpy((char*) uncompr, "garbage");

	AZX_LOG_INFO("Compressed message:\r\n");
	for (i = 0; i < comprLen; i++)
	{
		AZX_LOG_INFO("%c", compr[i]);
	}
	AZX_LOG_INFO("\r\n");

	err = azx_zlib_uncompress(uncompr, &uncomprLen, compr, comprLen);
	CHECK_ERR(err, "uncompress");

	AZX_LOG_INFO("comprLen: %u; uncomprLen: %u\r\n", comprLen, uncomprLen);

	if (strcmp((char*) uncompr, data))
	{
		AZX_LOG_ERROR("bad uncompress\n");
		return TEST_ERROR;
	}
	else
	{
		AZX_LOG_INFO("uncompress():\n%s\n", (char * )uncompr);
	}
	return TEST_SUCCESS;
}


/* Test compress() and uncompress()*/
int test_uncompress(char* source_S, char* dest_S)
{
    int  outfile;

    unsigned char in[CHUNK];

    int errnum = -1;
    unsigned int avail_in = 0;

	char mode_s = 'r';
    gzFile source;

    /* check valid name files */
	if (source_S == NULL || *source_S == 0 ||
		dest_S   == NULL || *dest_S   == 0	)
	{
    	AZX_LOG_ERROR("Invalid files name!\r\n");
		return AZX_GUNZIP_INVALID_NAME_FILE;
	}

    /* open source files */

    source = gzopen OF((source_S, &mode_s));
    if (source == NULL)
    {
    	AZX_LOG_ERROR("gun cannot open %s\n", source_S);
    	return AZX_GUNZIP_CANNOT_OPEN_FILE;
    }

    /* open or create destination files */
	outfile = m2mb_fs_open(dest_S, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (outfile == -1)
	{
		close(outfile);
		AZX_LOG_ERROR("cannot create %s\n", dest_S);
		return AZX_GUNZIP_CANNOT_CREATE_FILE;
	}

    do
    {
        memset(in, 0, CHUNK);
    	avail_in = gzread(source, in, CHUNK);
    	//AZX_LOG_INFO("%s", in); //prints uncompressed data read

    	gzerror(source, &errnum);
        if (errnum)
        {
        	gzclearerr(source);
			AZX_LOG_ERROR("Error: %d\n", errnum);
			break;
        }

        if ( (unsigned int) m2mb_fs_write(outfile, in, avail_in) != avail_in)
        {
        	AZX_LOG_ERROR("Error writing data!");
        	break;
        }

        if (avail_in == 0)
        {
        	break;
        }

    } while (!errnum);
	AZX_LOG_INFO("\n");

	azx_sleep_ms(2000); //waiting writing session

	if (gzclose(source) != Z_OK)
	{
    	AZX_LOG_ERROR("Error closing input file!");
    	return Z_ERRNO;
	}

    if (m2mb_fs_close(outfile) != 0)
    {
    	AZX_LOG_ERROR("Error closing output file!");
    	return Z_ERRNO;
    }

    if (errnum != 0)
    {
    	return Z_ERRNO;
    }
    else
    {
		AZX_LOG_INFO ("Data extracted correctly into the file %s\r\n", dest_S);
    	return AZX_GUNZIP_SUCCESS;
    }

}



/***************************************************************************************************
 \User Entry Point of Appzone

 \param [in] Module Id

 \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main(int argc, char **argv)
{
	//suppress Werror unused warning
	(void) argc;
	(void) argv;

	azx_sleep_ms(3000);

	//user code
	AZX_LOG_CFG_T log_cfg;

	/*Set log configuration */
	log_cfg.log_channel = LOG_CHANNEL; /* Defined in app_cfg.h */
	log_cfg.log_level = AZX_LOG_LEVEL_TRACE; /*Enable all logs*/
	log_cfg.log_colours = 0; /*Set to 1 to use coloured logs (not all terminals are compatible)*/

	azx_log_init(&log_cfg);

	AZX_LOG_INFO("Starting Logging demo app. This is v%s built on %s %s.\r\n",
	        VERSION, __DATE__, __TIME__);


	/*Starting zlib code */

	//compress/uncompress variables
	Byte *compr, *uncompr;
	uLong comprLen = 10000 * sizeof(int); /* don't overflow on MSDOS */
	uLong uncomprLen = comprLen;

	//gunzip variables
	int ret;
	char inname[] = LOCALPATH "/test.gz";
	char outname[] = LOCALPATH "/test.txt";

	//Starting TEST_COMPR_UNCOMPR
	AZX_LOG_INFO("\r\n");
	AZX_LOG_INFO("Starting TEST_COMPR_UNCOMPR.\r\n");

	compr = (Byte*) m2mb_os_calloc((uInt) comprLen);
	uncompr = (Byte*) m2mb_os_calloc((uInt) uncomprLen);
	/* compr and uncompr are cleared to avoid reading uninitialized
	 * data and to ensure that uncompr compresses well.
	 */
	if (compr == NULL || uncompr == NULL)
	{
		AZX_LOG_ERROR("out of memory\n");
		return;
	}


	if (test_compress_uncompress(compr, comprLen, uncompr, uncomprLen) == TEST_SUCCESS)
	{
		m2mb_os_free(compr);
		m2mb_os_free(uncompr);
		AZX_LOG_INFO("Ending TEST_COMPR_UNCOMPR with SUCCESS.\r\n\n");
	}


	//Starting test_uncompress
	AZX_LOG_INFO("Starting test_uncompress.\r\n");
	ret = test_uncompress(inname, outname);

	if (ret != AZX_GUNZIP_SUCCESS)
	{
		AZX_LOG_ERROR ("test_uncompress Error with: %d", ret);

	}
	else
	{
		AZX_LOG_INFO ("test_uncompress finished correctly!");

	}

	azx_log_deinit();

}





