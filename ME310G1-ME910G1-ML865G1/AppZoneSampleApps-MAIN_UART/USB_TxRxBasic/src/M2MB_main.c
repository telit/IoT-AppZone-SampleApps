/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application that shows how to use the basic read/write USB apis. Synchronous or asynchronous mode is available setting SYNC to 1 or 0. Debug prints on MAIN UART
  @version 
    1.0.0 
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
    Roberta galeazzo

  @date
    04/11/2021
*/

/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_usb.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
#define BUF_LEN 32
#define WR_BYTE_VAL 0x41
#define CYCLE 1  //set to one to read all bytes with a cycle, to read byte using callback length (resp_struct) set 0
#define SYNC 1   //for synchronous read/write, to have asynchronous read/write set to 0
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
BOOLEAN is_USB_connected;
INT32 g_USBfd = -1;
BOOLEAN state = FALSE;
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/
#if !SYNC
static void USB_Cb( INT32 fd, M2MB_USB_IND_E usb_event, UINT16 resp_size, void *resp_struct, void *userdata )
{
	(void)fd;
	(void)userdata;
	(void)resp_size;
	INT32 dataToRead, writtenData;
	INT32 dataRead;
	CHAR recBuf[BUF_LEN], buff[1024 + 1];

	switch(usb_event)
	{
		case M2MB_USB_CABLE_CONNECTED_EVENT:
		{
			is_USB_connected = *((BOOLEAN*)(resp_struct));
			AZX_LOG_INFO("Usb cable check event, USB status: %d\r\n", is_USB_connected);
		}
		break;

		case M2MB_USB_RX_EVENT:
		{

#if CYCLE
			memset(buff, 0, 1024);
			do
			{
				dataToRead = BUF_LEN;
				dataRead = m2mb_usb_read(fd, recBuf, dataToRead);
				if(dataRead > 0)//read fixed number of bytes bytes until buffer is empty
					strncat(buff, recBuf, dataRead);
				AZX_LOG_INFO("USB read bytes %d\r\n", dataRead);
			}while (dataRead > 0);


			AZX_LOG_INFO("Rec from USB: %s\r\n", buff);
#else
			dataToRead = *((INT32*)resp_struct); //use resp_struct information to read data
			dataRead = m2mb_usb_read(fd, buff, dataToRead);
			AZX_LOG_INFO("USB read bytes %d\r\n", dataRead);
			buff[dataRead] = 0;
			AZX_LOG_INFO("Rec from USB: %s\r\n", buff);
#endif
		}
		break;

		case M2MB_USB_TX_EVENT:
		{
			writtenData = *((INT32*)resp_struct);
			AZX_LOG_INFO("Written bytes: %d\r\n", writtenData);

		}
		break;

		default:
			break;

	}

}
#endif
/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
**************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;

	INT32 retVal;
	INT32 wrBytes;
#if SYNC
	INT32 rdBytes;
	UINT8 dstBuf[BUF_LEN];
#else
	UINT8 srcBuf[BUF_LEN];
	M2MB_USB_CFG_T cfg;
#endif

	azx_sleep_ms(5000);

	/*Set log configuration */
	AZX_LOG_INIT();

	AZX_LOG_INFO("\r\n\r\nStarting USB read write demo app. This is v%s built on %s %s.\r\n\r\n",
        VERSION, __DATE__, __TIME__);

	//open usb port
	AZX_LOG_INFO("\r\nOpen USB port\r\n");
	g_USBfd = m2mb_usb_open("/dev/USB0", 0);
	if ( g_USBfd != -1 )
	{
		AZX_LOG_INFO( "m2mb_usb_open succeeded\r\n");
	}
	else
	{
		AZX_LOG_ERROR( "m2mb_usb_open failed\r\n");
		return;
	}


 	retVal = m2mb_usb_ioctl(g_USBfd, M2MB_USB_IOCTL_GET_CABLE_STATE, &state);
	if ( retVal != -1 )
	{
		AZX_LOG_INFO( "m2mb_usb_ioctl: got cable status\r\n");
		if (state == 0)
		{
			AZX_LOG_INFO( "USB cable DISCONNECTED, status: %d\r\n", state);
		}
		else
		{
			AZX_LOG_INFO( "USB cable CONNECTED, status: %d\r\n", state);
		}
	}
	else
	{
		AZX_LOG_ERROR( "Failure getting USB cable state event\r\n\r\n");
		return;
	}

	m2mb_usb_ioctl( g_USBfd, M2MB_USB_IOCTL_SET_CB, NULL ); // reset callback function
	m2mb_usb_ioctl( g_USBfd, M2MB_USB_IOCTL_SET_TX_TIMEOUT, 10); // set 5000 ms of timeout for writing (just to check if it's blocking)
	m2mb_usb_ioctl( g_USBfd, M2MB_USB_IOCTL_SET_RX_TIMEOUT, 1000 ); // set 5000 ms of timeout for reading (just to check if it's blocking)

	wrBytes = m2mb_usb_write(g_USBfd, "READY", 5);
	if (wrBytes <= 0)
	{
	  AZX_LOG_ERROR( "No bytes written\r\n");
	}

#if SYNC
	AZX_LOG_INFO( "Synchronous read and write\r\n" );
	azx_sleep_ms(3000);

	//synchronous reading
	AZX_LOG_INFO( "Read until some bytes are received...\r\n");

	do
	{
		rdBytes  = m2mb_usb_read(g_USBfd, dstBuf, BUF_LEN);
		if( rdBytes < 0 )
		{
			 AZX_LOG_ERROR( "m2mb_usb_read failure\r\n" );
		}
		else if( rdBytes == 0 )
		{
		  AZX_LOG_INFO( "rx timeout expired\r\n" );
		}
		else
		{
			AZX_LOG_INFO( "m2mb_usb_read read %d Bytes\r\n", rdBytes );
			azx_sleep_ms(1000);
			wrBytes = m2mb_usb_write(g_USBfd, "REC:", 4);
			wrBytes = m2mb_usb_write(g_USBfd, dstBuf, rdBytes);
			if( wrBytes < 0 )
			{
				AZX_LOG_ERROR( "m2mb_usb_write failure\r\n" );
			}
			else if( wrBytes == 0 )
			{
				AZX_LOG_INFO( "tx timeout expired\r\n" );
			}
			else
			{
				AZX_LOG_INFO( "m2mb_usb_write sent %d Bytes\r\n", wrBytes );
			}
		}
	}while (1);

	azx_sleep_ms(3000);

#else
	//ASYNCHRONOUS reading/writing with callback, and cable check
	memset(srcBuf, WR_BYTE_VAL, BUF_LEN);
	srcBuf[BUF_LEN]=0;
	azx_sleep_ms(2000);
	AZX_LOG_INFO( "Asynchronous read and write\r\n" );
	retVal = m2mb_usb_ioctl(g_USBfd, M2MB_USB_IOCTL_GET_CFG, &cfg);
	if ( retVal != -1 )
	{
		AZX_LOG_INFO("\r\nSet USB callback\r\n\r\n");
		retVal = m2mb_usb_ioctl(g_USBfd, M2MB_USB_IOCTL_SET_CB, &USB_Cb);
		if ( retVal != -1 ) 
		{
			AZX_LOG_INFO( "m2mb_usb_ioctl: set usb callback, OK\r\n");
		} 
		else 
		{
			AZX_LOG_ERROR( "Failure setting USB callback\r\n");
			return;
		}
	}

	azx_sleep_ms(3000);
	AZX_LOG_INFO( "Write 32 bytes...\r\n" );
	m2mb_usb_ioctl( g_USBfd, M2MB_USB_IOCTL_SET_TX_TIMEOUT, 5000 ); // set 5000 ms of timeout for writing (just to check if it's blocking)
	wrBytes = m2mb_usb_write(g_USBfd, srcBuf, BUF_LEN);
	if( wrBytes < 0 )
	{
		AZX_LOG_ERROR( "m2mb_usb_write failure" );
	}

	AZX_LOG_INFO( "waiting for characters to be read\r\n");
	
#endif
}


