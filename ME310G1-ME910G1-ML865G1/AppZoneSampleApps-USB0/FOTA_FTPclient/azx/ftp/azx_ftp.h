/***************************************************************************/
/*									   */
/* ftplib.h - header file for callable ftp access routines                 */
/* Copyright (C) 1996-2001, 2013, 2016 Thomas Pfau, tfpfau@gmail.com	   */
/*	1407 Thomas Ave, North Brunswick, NJ, 08902			   */
/*									   */
/* This library is free software.  You can redistribute it and/or	   */
/* modify it under the terms of the Artistic License 2.0.		   */
/* 									   */
/* This library is distributed in the hope that it will be useful,	   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	   */
/* Artistic License 2.0 for more details.				   */
/* 									   */
/* See the file LICENSE or 						   */
/* http://www.perlfoundation.org/artistic_license_2_0			   */
/*									   */
/***************************************************************************/

/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*===========================================================================*/
/**
 * @file azx_ftp.h
 * @version 1.3.0
 * @dependencies
 *
 * @note
 *   Dependencies:
 *      m2mb_types.h
 *
 * @author Fabio Pintus, Roberta Galeazzo
 *
 * @date 21/02/2020
 * @brief FTP Library routines
 *
 * This file contains all the functions for FTP operations. It is a porting of Thomas Pfau
 * work at https://nbpfaus.net/~pfau/ftplib/ (see disclaimer above)
 * renaming original functions and defines/ typedefs to use azx suffix
 */

/** \defgroup AZX_FTP_CONF FTP Client configuration
 *  Functions that can be used to configure the FTP client
 */

/** \defgroup AZX_FTP_USAGE FTP Client usage
 *  Functions that can be used to perform FTP operations
 */

/* Global declarations =======================================================*/

#ifndef HDR_AZX_FTP_H
#define HDR_AZX_FTP_H

/** \cond PRIVATE */
#if !defined(AZX)
#define AZX
#endif

#if defined(__unix__) || defined(VMS) || defined(AZX)
#define AZX_FTP_GLOBALDEF
#define AZX_FTP_GLOBALREF extern
#elif defined(_WIN32) && ! defined(__MINGW32__)
#if defined BUILDING_LIBRARY
#define AZX_FTP_GLOBALDEF __declspec(dllexport)
#define AZX_FTP_GLOBALREF __declspec(dllexport)
#else
#define AZX_FTP_GLOBALREF __declspec(dllimport)
#endif
#endif


#if defined(AZX)
#define __UINT64_MAX ULLONG_MAX
#define _REENTRANT 0
#if defined(__ARMCLIB_VERSION)
#undef memccpy
#define NEED_MEMCCPY
#endif
#endif
/** \endcond */


/* azx_ftp_access() type codes */
/** \addtogroup  AZX_FTP_USAGE
@{ */

/**
 * \brief Type codes used in azx_ftp_access()
 *  @{ */
#define AZX_FTP_DIR 1                       /**< List directory (NLST)*/
#define AZX_FTP_DIR_VERBOSE 2               /**< List directory verbose (LIST)*/
#define AZX_FTP_FILE_READ 3                 /**< Read file (RETR)*/
#define AZX_FTP_FILE_WRITE 4                /**< Write file (STOR)*/
/** @} */



/* azx_ftp_access() mode codes */
/** \name azx_ftp_access mode codes
 * \brief Mode codes used in azx_ftp_access()
 *  @{ */
#define AZX_FTP_ASCII 'A'                   /**< Use ASCII mode*/
#define AZX_FTP_IMAGE 'I'                   /**< Use binary (image) mode*/
#define AZX_FTP_TEXT AZX_FTP_ASCII          /**< Use ASCII mode*/
#define AZX_FTP_BINARY AZX_FTP_IMAGE        /**< Use binary (image) mode*/
/** @} */



/* ftp connection modes */
/** \name azx_ftp_access connection modes
 * \brief Connection modes used in azx_ftp_access()
 *  @{ */
#define AZX_FTP_PASSIVE 1                   /**< Use passive mode*/
#define AZX_FTP_PORT 2                      /**< Use active mode*/

/** @} */
/** @} */  //close addtogroup

/** \addtogroup  AZX_FTP_CONF
@{ */

/* connection option names */
/** \name azx_ftp_access connection options
 * \brief Connection options used with azx_ftp_options()
 *  @{ */
#define AZX_FTP_CONNMODE 1                  /**< set connection mode*/
#define AZX_FTP_CALLBACK 2                  /**< set idle callback function*/
#define AZX_FTP_IDLETIME 3                  /**< set idle callback timeout value (it will be called every time the timeout expires)*/
#define AZX_FTP_CALLBACKARG 4               /**< set an user argument for the callback function*/
#define AZX_FTP_CALLBACKBYTES 5             /**< set idle callback bytes amount (it will be called every time the amount is exchanged with the server)*/
/** @} */
/** @} */  //close addtogroup

/** \cond PRIVATE */
/*FabioPi*/
#define AZX_FTP_TMP_BUFSIZ           1024
#define AZX_FTP_DEFAULT_PORTNUM 21
#define AZX_FTP_REST_CMD_SIZE        32
/****/

/** \endcond */


//----Fabiopi
/** @addtogroup  AZX_FTP_USAGE
@{ */

/** @name FTP max REST retries
 *  @brief
    Number of retries when using REST command during a RETR operation.

    @details
    When the RETR operation (file get) is performed, and an error occurs (e.g. server closes
    the connection due to slow transfer), the REST command is tried to restore the download from the point it reached.
    Up to 5 retries are attempted.
 *  @{ */
#define AZX_FTP_MAX_REST             5        /**< Maximum number of retries for REST during RETR operation*/
/** @} */
/** @} */  //close addtogroup


#ifdef __cplusplus
extern "C" {
#endif

/* Global typedefs ===========================================================*/

#if defined(__UINT64_MAX)
typedef uint64_t azx_ftp_fsz_t;               /**< unsigned long long definition*/
#else
typedef uint32_t azx_ftp_fsz_t;               /**< unsigned long definition*/
#endif



/**
  @struct AZX_FTP_NET_BUF_TAG

  @brief Ftp Client parameters structure

  @details This structure holds the parameters of the ftp client

  \ingroup AZX_FTP_USAGE

  <b>Refer to</b>
      azx_ftp_connect()
 */
/*-----------------------------------------------------------------------------------------------*/
typedef struct AZX_FTP_NET_BUF_TAG AZX_FTP_NET_BUF_T;  /**< Typedef of struct AZX_FTP_NET_BUF_TAG*/


/* Global functions ==========================================================*/

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Callback function signature

  @details
    This is the callback function signature (optional) that the client will call during data exchange with the remote server. 
    User callback must follow this format.

  @param[in] nControl
        Pointer to the FTP client structure pointer         
   @param[in] xfered
        Indicates how much data bytes have been transferred until now
   @param[in] arg
        User parameter, set in azx_ftp_setCallback()

  @return
		1 if the FTP client must keep running
  @return
		0 to force the FTP client to interrupt the current operation and close the data socket.

  <b>Refer to</b>
    azx_ftp_setCallback() azx_ftp_clearCallback()

  <b>Sample usage</b>
  @code
        AZX_FTP_NET_BUF_T *ftp_client;

		    FtpCallbackOptions opt;
        opt.cbFunc = log_progress;
        opt.cbArg = &f;
        opt.idleTime = 1000;  //call each second
        opt.bytesXferred = 1024;  //call each 1024 bytes

        azx_ftp_setCallback(&opt,ftp_client);
  @endcode
  @ingroup AZX_FTP_CONF
 */
/*-----------------------------------------------------------------------------------------------*/
typedef int (*azx_ftp_callback)(AZX_FTP_NET_BUF_T *nControl, azx_ftp_fsz_t xfered, void *arg);



/**
 * \brief Logging levels for FTP logs
 * \ingroup AZX_FTP_CONF
 */
typedef enum
{
  AZX_FTP_DEBUG_HOOK_NONE = 0,    /**<Do not print any message*/
  AZX_FTP_DEBUG_HOOK_ERROR = 1,   /**<Print an error message*/
  AZX_FTP_DEBUG_HOOK_INFO = 2,    /**<Print an information message*/
  AZX_FTP_DEBUG_HOOK_DEBUG = 3    /**<Print a debug message*/
} AZX_FTP_DEBUG_HOOK_LEVELS_E;

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Debug callback that is called by the library to for debug purposes.

  @details
    This function prototype can be registered by the user with the azx_ftp_init call.
		It is user's responsibility to define the logic of all debug levels (see
		FTP_DEBUG_HOOK_LEVELS_E enum above ) and how they are printed.

	@param[in] level
    Debug level, see FTP_DEBUG_HOOK_LEVELS
	@param[in] function
    This indicates the function name where the debug print is occurring
	@param[in] file
    This indicates the .c file name where the debug print is occurring
	@param[in] line
    This indicates the .c file line number where the debug print is occurring
	@param[in] fmt
    format string (as in printf). It contains the log message to be printed,
    plus possible additional parameters, to be used with va_list.

  @return
    An integer value, not used at the moment

  <b>Refer to</b>
    azx_ftp_setCallback() azx_ftp_clearCallback()

  <b>Sample usage</b>
  @code
  	#include <stdarg.h>

  	int (FTP_DEBUG_HOOK_LEVELS_E level, const char *function, const char *file, int line, const char *fmt, ...)
  	{
  		char buf[512];
			int bufSize = sizeof(buf);
			va_list arg;
			int offset = 0;
			memset(buf,0,bufSize);
			switch(level)
			{
						case FTP_DEBUG_HOOK_ERROR:
							offset = sprintf(buf, "ERR %32s:%-4d - ",
												function,
												line
												);
						break;

						case FTP_DEBUG_HOOK_INFO:
							offset = 0;
						break;

						case FTP_DEBUG_HOOK_DEBUG:
							offset = sprintf(buf, "DBG %32s:%-4d - ",
											function,
											line
											);

						break;
			}
			va_start(arg, fmt);
			vsnprintf(buf + offset, bufSize-offset, fmt, arg);
			va_end(arg);
			return PRINT(buf); //NOTE: PRINT is a generic print function like printf
  	}
  @endcode

  @ingroup AZX_FTP_CONF
 */
/*-----------------------------------------------------------------------------------------------*/

typedef INT32 (*azx_ftp_debugHook)(AZX_FTP_DEBUG_HOOK_LEVELS_E level, const CHAR *function, const CHAR *file, INT32 line, const CHAR *fmt, ...);



/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP initialization options structure

  @details
    This structure contains the options for the FTP client, as the debug callback function hook FtpDebugHook()
    The CID to be used for the network operations (a PDP context must be already active to use the client)
    the debug level to be used.
    a callback argument (unused at the moment)

  @return
        None

  <b>Refer to</b>
    azx_ftp_init()
 */
/*-----------------------------------------------------------------------------------------------*/

typedef struct AZX_FTP_OPTIONS_TAG{
  azx_ftp_debugHook cbFunc;		             /**< Debug callback function to call for client logging*/
  AZX_FTP_DEBUG_HOOK_LEVELS_E level;		   /**< Global debug Level to set */
  unsigned char cid;                       /**< Context id used by active connection*/
  void *cbArg;                             /**< User defined parameter, not used*/
  #ifdef M2M_M2MB_SSL_H
  INT8 ssl;                                /*RoGa: new SSL parameters*/
  M2MB_SSL_CONFIG_HANDLE sslConfigH;
  M2MB_SSL_CTXT_HANDLE sslCtxtH;
#endif
} AZX_FTP_OPTIONS_T;


/** \cond PRIVATE */
AZX_FTP_GLOBALREF AZX_FTP_OPTIONS_T ftp_opts;


#define AZX_FTP_DEBUG(l,a...) 		if(ftp_opts.level >= l && ftp_opts.cbFunc) { ftp_opts.cbFunc(l,	__FUNCTION__, __FILE__, __LINE__, a);}
/** \endcond */



/**
 * \brief FTP user callback events. When data is received, one of the events is reported
 * \ingroup AZX_FTP_USAGE
 */
typedef enum
{
  DATA_CB_START,              /**< The FTP client is notifying that the connection with the server is ready and the data download will start shortly. In this case, data and datalen are meaningless.) */
  DATA_CB_DATA,               /**< This event is called for each block of data received by the server.
                                                data variable will contain the block of data. 
                                                datalen variable will be the length of the received block of data */
  DATA_CB_END                 /**<  This event is called when the transfer is complete.
                                                data variable is meaningless
                                                datalen indicates the total received data (it can be compared with the file size on the server)*/
} AZX_FTP_USER_CB_EV_E;

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP data callback.

  @details
    This callback can be registered while receiving data in buffer mode (*NOT in file mode*)
    If a callback is passed to the configuration structure, the provided buffer will be used to retrieve
    data from the FTP connection, and for each block of data the callback will be called to allow the user
    to manage the data.

  @param[in] data
    The buffer with the received data
  @param[in] datalen
    Indicates the amount of receive data (see ev parameter description for more details)
 	@param[in] ev
    This is an event provided by the client. it can be
        DATA_CB_START
        DATA_CB_DATA    
        DATA_CB_END

  @return
    an Integer number. unused for now


  <b>Refer to</b>
    AZX_FTP_BUFFER

  <b>Sample usage</b>
  @code
		int my_data_cb(char *data, uint32_t datalen, int ev)
		{
			switch(ev)
			{
				case DATA_CB_START:
					//do some initialization before receiving data (e.g. open a file descriptor)
				break;
				case DATA_CB_DATA:
				  //manage the data
				break;
				case DATA_CB_END:
					//finalize
				break;
			}
		  return 1;
		}

        main()
        {

            //...//
            AZX_FTP_XFER local;
            AZX_FTP_NET_BUF_T *ftp_client;
            char remote_file_path[] = "myfile";
            char buffer[512];

            // Init ftp client

            local.type = AZX_FTP_XFER_BUFF;
            local.payload.buffInfo.buffer = (char*) buffer;
            local.payload.buffInfo.bufferSize = sizeof(buffer);
            local.payload.buffInfo.buf_cb = my_data_cb; //the data callback needed to manage the data.

            azx_ftp_get(&(local), remote_file_path, AZX_FTP_BINARY, ftp_client);

        }
  @endcode

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
typedef INT32 (*azx_ftp_dataCallback)(CHAR *data, UINT32 datalen, INT32 ev);


/*-----------------------------------------------------------------------------------------------*/
/**
  @struct AZX_FTP_FILE_INFO

  @brief
    Generic file information structure

  @details
    This structure holds the information of a file to be downloaded or uploaded.
    It has the path of the file and its size in bytes

  <b>Refer to</b>
    AZX_FTP_XFER

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
typedef struct AZX_FTP_FILE_INFO_TAG
{
  CHAR *path;                      /**<File path (local or remote)*/
  UINT32 fileSize;                 /**<File size in bytes*/
} AZX_FTP_FILE_INFO_T;


/*-----------------------------------------------------------------------------------------------*/
/**
  @struct AZX_FTP_BUFFER

  @brief
    Generic buffer information structure

  @details
    This structure holds the information of a buffer, where data can be stored during a file get or
    from where data can be retrieved during a file put.

    It contains

    the buffer size


    <b>Refer to</b> 
    AZX_FTP_XFER azx_ftp_get() azx_ftp_put()

    <b>Sample usage</b>
    @code
  	  char buffer[1500];

      AZX_FTP_BUFFER b = { buffer, 1500, my_data_cb};
    @endcode
 */
/*-----------------------------------------------------------------------------------------------*/
typedef struct AZX_FTP_BUFFER_TAG
{
  CHAR *buffer;                    /**< The pointer to the buffer to be used when downloading*/
  UINT32 bufferSize;               /**< The buffer  size*/
  azx_ftp_dataCallback buf_cb;     /**< The data callback hook*/
} AZX_FTP_BUFFER_T;


/**
 * @brief FTP transfer type.

   <b>Refer to</b>
   AZX_FTP_XFER_T

 * @ingroup AZX_FTP_USAGE
 */
typedef enum
{
  AZX_FTP_XFER_FILE,           /**< File will be transferred as a file stream*/
  AZX_FTP_XFER_BUFF,           /**< File transfer will be managed with a buffer*/
  AZX_FTP_XFER_MAX=0xFFFFFFFF  /**< max value, do not use*/
} AZX_FTP_XFER_E;


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Generic transfer information structure

  @details
    This structure holds the information what kind of transfer media to be used for the ftp get or put
    operations. It can be file or buffer then the payload can be either a AZX_FTP_FILE_INFO or AZX_FTP_BUFFER_INFO structure

  <b>Refer to</b> 
    azx_ftp_get() azx_ftp_put()

  <b>Sample usage</b>
 @code
  	char buffer[1500];

    AZX_FTP_BUFFER b = { buffer, 1500, my_data_cb};
    AZX_FTP_XFER x;
    x.type = AZX_FTP_XFER_BUFF;
    x.payload = b;
  @endcode

  @ingroup AZX_FTP_USAGE

 */
/*-----------------------------------------------------------------------------------------------*/
typedef struct  AZX_FTP_XFER_TAG
{
  AZX_FTP_XFER_E type;                     /**<The type of the transfer mode*/
  union
  {
    AZX_FTP_FILE_INFO_T fileInfo;          /**< If type is AZX_FTP_XFER_FILE, this will provide file information (path, size)*/
    AZX_FTP_BUFFER_T buffInfo;             /**< If type is AZX_FTP_XFER_BUFF, this will provide buffer information (pointer, size, data callback if needed)*/
  } payload;                                      /**< Payload of the transfer operation*/
} AZX_FTP_XFER_T;


/*-----------------------------------------------------------------------------------------------*/
/**
  @struct FtpCallbackOptions

  @brief
    Structure that holds status callback function parameters

  @details
    This structure holds all the parameters that can be used to set a transfer status callback. It will be called by the client
    each time one of the set event occurs (timeout or bytes transferred)

    <b>Refer to</b> 
    azx_ftp_setCallback()  azx_ftp_clearCallback()

  @ingroup AZX_FTP_CONF
 */
/*-----------------------------------------------------------------------------------------------*/
typedef struct AZX_FTP_CALLBACK_OPTIONS_TAG
{
  azx_ftp_callback cbFunc;		      /**< Callback Function to call */
  void *cbArg;		                  /**< User argument to pass to function */
  UINT32 bytesXferred;	            /**< Callback if this number of bytes transferred */
  UINT32 idleTime;	                /**< Callback if this many milliseconds have elapsed */
} AZX_FTP_CALLBACK_OPTIONS_T;         /**< Typedef of struct AZX_FTP_CALLBACK_OPTIONS_TAG*/



/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP initialize function.

  @details
    This function must be called at the beginning of the code, to set the parameters required by the client.

  @param[in] opt
    ftp_options struct holding the parameters

  @return
    1 

  @ingroup AZX_FTP_CONF
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_init(const AZX_FTP_OPTIONS_T *opt);
/** \example{lineno} azx_ftp_example.c
 * This is a detailed example of FTP client usage.
 */


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Return last FTP server response.

  @details
    This function will provide the last server response, helping the debugging in case of errors.

  @param[in] nControl
    Structure pointer of the FTP client

  @return
    The server response string


  <b>Refer to</b>
    azx_ftp_init() azx_ftp_login() azx_ftp_get() azx_ftp_put()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
CHAR *azx_ftp_lastResponse(AZX_FTP_NET_BUF_T *nControl);


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP connection function.

  @details
    This function will perform the network connection to the provided host.

  @param[in] host
    String with the host name or IP address
  @param[inout] nControl
    Address of the pointer to the FTP client structure that will be initialized 

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_connect(const CHAR *host, AZX_FTP_NET_BUF_T **nControl);


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP options function.

  @details
    This function allows to set client related options, see azx_ftp_access connection options

  @param[in] opt
       Option identifier
           AZX_FTP_CONNMODE
           AZX_FTP_CALLBACK
           AZX_FTP_IDLETIME
           AZX_FTP_CALLBACKARG
           AZX_FTP_CALLBACKBYTES

  @param[in] val
        Option value
  @param[in] nControl
    Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_options(INT32 opt, INT32 val, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP callback set function.

  @details
    This function allows to set the idle callback function, that will be called when a defined event occurs

  @param[in] opt
       pointer to the struct holding the parameters
  @param[in] nControl
    Pointer to the FTP client structure

  @return
    1

  <b>Refer to</b>
   azx_ftp_get() azx_ftp_put()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_setCallback(const AZX_FTP_CALLBACK_OPTIONS_T *opt, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP callback clear function.

  @details
    This function allows to unset a previously set idle callback function

  @param[in] nControl
    Pointer to the FTP client structure

  @return
    1

  <b>Refer to</b>
   azx_ftp_get() azx_ftp_put()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_clearCallback(AZX_FTP_NET_BUF_T *nControl);



/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP login function.

  @details
    This function performs the FTP login to the remote host

  @param[in] user
      FTP username 
  @param[in] pass
      FTP password
  @param[in] nControl
    Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() FtpQuit()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_login(const CHAR *user, const CHAR *pass, AZX_FTP_NET_BUF_T *nControl);


/** \cond PRIVATE */

#ifdef M2M_M2MB_SSL_H
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP set data channel security parameters.

  @details
    This function set PROT and PBSZ data channel security parameters

  @param[in] user
      FTP prot
  @param[in] pass
      FTP bufsize
  @param[in] nControl
    Pointer to the FTP client structure

  @return
    1 if Success
  @return
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() FtpQuit()

  @ingroup AZX_FTP_USAGE
 *
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_sslCfg(CHAR prot, CHAR buffSise, AZX_FTP_NET_BUF_T *nControl);
#endif

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP data access function.

  @details
    This function performs the operations required to setup the data exchange socket in passive mode
    Usually it is internally called, not by the user.
  @param[in] path 
      FTP file path 
  @param[in] typ
      Operation type:
            AZX_FTP_FILE_WRITE
            AZX_FTP_FILE_READ
  @param[in] mode
       Data transfer mode:
        AZX_FTP_ASCII
        AZX_FTP_IMAGE
  @param[in] nControl
      Pointer to the FTP client structure
  @param[out] nData
    Address of the pointer to the FTP structure holding the data socket info, that will be initialized.
  @param[in] offset 
       Integer providing the initial offset of the file to be downloaded. Used in case of REST

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_get() azx_ftp_put() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_access(const CHAR *path, INT32 typ, INT32 mode, AZX_FTP_NET_BUF_T *nControl,
    AZX_FTP_NET_BUF_T **nData, INT32 offset);


INT32 azx_ftp_read(void *buf, INT32 max, AZX_FTP_NET_BUF_T *nData);
INT32 azx_ftp_write(const void *buf, INT32 len, AZX_FTP_NET_BUF_T *nData);
INT32 azx_ftp_close(AZX_FTP_NET_BUF_T *nData);
/** \endcond */


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP SITE command

  @details
    This function sends site specific commands to remote server (like SITE IDLE 60 or SITE UMASK 002 ). Inspect SITE HELP output for complete list of supported commands.
    use FtpLastResponse() to read the server reply

  @param[in] cmd 
      String to be used as SITE parameter. 
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_site(const CHAR *cmd, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP SYST command

  @details
    This function sends a SYST command to the remote server. The response will strip the numeric return code.
    For example:
        >SYST
        <215 UNIX Type: L8
    The response will be "UNIX Type: L8"

  @param[out] buf 
        Allocated buffer that will be used to contain the response
  @param[in] max
        Max size of provider buffer
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_sysType(CHAR *buf, INT32 max, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP make dir at remote server

  @details
    This function asks remote server to create a directory.

  @param[in] path 
            Path of the new directory. Can be absolute or relative to current remote directory.
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_mkdir(const CHAR *path, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP change current path at remote server

  @details
    This function asks remote server to check provided path

  @param[in] path 
            Path of the requested work directory. Can be absolute or relative to current remote directory.
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_chdir(const CHAR *path, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP move to parent at remote server

  @details
    This function asks remote server to go up to parent folder 

  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_cdUp(AZX_FTP_NET_BUF_T *nControl);


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP remove dir to remote server

  @details
    This function asks remote server to delete a directory.

  @param[in] path 
            Path of the new directory. Can be absolute or relative to current remote directory. 
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_rmdir(const CHAR *path, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP get present working directory at remote server

  @details
    This function asks remote server the present working directory

  @param[out] path 
        Allocated buffer that will contain the CWD string
  @param[in] max
        Max size of the buffer
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_pwd(CHAR *path, INT32 max, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP ask NLST to remote server

  @details
    This function asks remote server an NLST command and returns the response

  @param[out] output
     AZX_FTP_XFER structure that will contain the NLST response
  @param[in] path
     Path to be passed to the NLST command   
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_nlst(AZX_FTP_XFER_T *output, const CHAR *path, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP ask LIST to remote server

  @details
    This function asks remote server an LIST command and returns the response

  @param[out] output
     AZX_FTP_XFER structure that will contain the LIST response
  @param[in] path
     Path to be passed to the LIST command 
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_dir(AZX_FTP_XFER_T *output, const CHAR *path, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP ask file size to remote server

  @details
    This function asks remote server the size in bytes of a remote file

  @param[in] path
     Path to be passed to the command. can be relative or absolute.
  @param[out] size
    Unsigned long variable that will contain the file size
  @param[in] mode
    The file mode, AZX_FTP_ASCII or AZX_FTP_IMAGE
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_size(const CHAR *path, UINT32 *size, CHAR mode, AZX_FTP_NET_BUF_T *nControl);


#if defined(__UINT64_MAX)
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP ask file size to remote server for big files

  @details
    This function asks remote server the size in bytes of a remote file. to be used for big files (UINT64 is used for file size)

  @param[in] path
     Path to be passed to the command. can be relative or absolute.
  @param[out] size
    Unsigned long long variable that will contain the file size
  @param[in] mode
    The communication mode, AZX_FTP_ASCII or AZX_FTP_IMAGE
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 axz_ftp_sizeLong(const CHAR *path, azx_ftp_fsz_t *size, CHAR mode, AZX_FTP_NET_BUF_T *nControl);
#endif

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP retrieve modification date of remote file

  @details
    This function asks remote server the modification date of a remote file.

  @param[in] path
     Path to be passed to the command. can be relative or absolute.
  @param[out] dt
    Allocated buffer that will contain the timestamp as a string
  @param[in] max
    Max size of the buffer
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_modDate(const CHAR *path, CHAR *dt, INT32 max, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP retrieve remote file

  @details
    This function asks remote server to send a remote file

  @param[out] output
     AZX_FTP_XFER structure that will hold the file content (can be a buffer or a file in the filesystem)
  @param[in] path
     Path to be passed to the command. can be relative or absolute.
  @param[in] mode
    The communication mode, AZX_FTP_ASCII or AZX_FTP_IMAGE
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_get(AZX_FTP_XFER_T *output, const CHAR *path, CHAR mode,
    AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP send local file

  @details
    This function sends a local file to the remote server

  @param[in] input
     AZX_FTP_XFER structure that will hold the file content (can be a buffer or a file in the filesystem)
  @param[in] path
     Path to be passed to the command. can be relative or absolute.
  @param[in] mode
    The communication mode, AZX_FTP_ASCII or AZX_FTP_IMAGE
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_put(AZX_FTP_XFER_T *input, const CHAR *path, CHAR mode,
    AZX_FTP_NET_BUF_T *nControl);


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP rename remote file

  @details
    This function asks remote server to rename a file

  @param[in] src
     Original file path
  @param[in] dst
     Destination file path to be applied
  @param[in] nControl
     Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_rename(const CHAR *src, const CHAR *dst, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP delete remote file

  @details
    This function asks remote server to delete a file

  @param[in] fnm
     Remote file path to be deleted (can be relative or absolute)
  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_ftp_delete(const CHAR *fnm, AZX_FTP_NET_BUF_T *nControl);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    FTP close session 

  @details
    This function asks remote server to close the session, and closes the communication socket

  @param[in] nControl
      Pointer to the FTP client structure

  @return
    1 if Success
  @return 
    0 otherwise

  <b>Refer to</b>
   azx_ftp_init() azx_ftp_connect() azx_ftp_login()

  @ingroup AZX_FTP_USAGE
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_ftp_quit(AZX_FTP_NET_BUF_T *nControl);

#ifdef __cplusplus
};
#endif

#endif /* HDR_AZX_FTP_H */
