/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */


#ifndef AZX_HTTPS_CLIENT_H
#define AZX_HTTPS_CLIENT_H

/**
  @file azx_https.h
  @version 1.1.1
  @dependencies azx_string_utils gnu azx_base64

  @brief HTTPs client

  Porting of HTTPs client from https://github.com/HISONA/https_client

  @note
    Dependencies:
       m2mb_types.h
       m2mb_socket.h
       m2mb_ssl.h
       
       

  @author
    Luca Boi
    Fabio Pintus (AZX conversion)

  @date
     11/09/2020
*/

/** \defgroup httpConf HTTP client configuration
    Functions that can be used to configure HTTP client parameters
*/

/** \defgroup httpUsage HTTP client usage
    Functions that can be used to perform HTTP operations
*/


/* Global declarations =======================================================*/

/** \addtogroup  httpUsage
  @{ */

/** \name Header defines
   \brief These defines are related to HTTP header info
    @{ */
#define AZX_HTTP_H_FIELD_SIZE     512       /**< Max size in bytes for each header field*/
#define AZX_HTTP_H_READ_SIZE      2048        /**< Max size in bytes for a single read operation from the remote server*/
#define AZX_HTTP_H_CHUNK_SIZE     50          /**< Max chunk header size in bytes*/
/** @} */
/** @} */  //close addtogroup



/* Global typedefs ===========================================================*/
//typedef unsigned char BOOL;   /**< Boolean value*/



/*!
   \brief HTTP Supported methods
   \ingroup httpConf
*/
typedef enum _AZX_HTTP_METHOD
{
  AZX_HTTP_GET      = 0,              /**<Perform a GET */
  AZX_HTTP_POST,                      /**<Perform a POST */
  AZX_HTTP_HEAD                       /**<Perform a HEAD */
} AZX_HTTP_METHOD;


/*!
   \brief HTTP Supported authentication methods
   \ingroup httpConf
*/
typedef enum _HTTP_AUTH_SCHEMA
{
  azx_AuthSchemaNone      = 0,        /**<No authentication*/
  azx_AuthSchemaBasic,                   /**<Basic authentication*/
  azx_AuthNotSupported                   /**<Placeholder for unsupported authentication*/
} AZX_HTTP_AUTH_SCHEMA;

/*!
  \struct AZX_HTTP_HEADER

  \brief HTTP header structure

  \details This structure holds the parameters required to manage HTTP header data

  \ingroup httpUsage

  <b>Refer to</b>
      AZX_HTTP_INFO
*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct
{
  AZX_HTTP_METHOD method;                      /**<Type of operation*/
  AZX_HTTP_AUTH_SCHEMA auth_type;              /**<Type of authentication*/
  int  status;                                 /**<Operation result status*/
  char content_type[AZX_HTTP_H_FIELD_SIZE];    /**<Field to hold HTTP content type*/
  long content_length;                         /**<Field to hold HTTP content length*/
  BOOLEAN chunked;                             /**<Is transmission chunked?*/
  BOOLEAN close;                               /**<Close connection at the end?*/
  char user_agent[AZX_HTTP_H_FIELD_SIZE];      /**<Field to hold HTTP user agent*/
  char location[AZX_HTTP_H_FIELD_SIZE];        /**<Field to hold HTTP location*/
  char referrer[AZX_HTTP_H_FIELD_SIZE];        /**<Field to hold HTTP referrer*/
  char boundary[AZX_HTTP_H_FIELD_SIZE];        /**<Field to hold HTTP boundary*/
  char accept_type[AZX_HTTP_H_FIELD_SIZE];     /**<Field to hold HTTP accept type*/
  char *post_data;                             /**<Field that will hold POST data. Must be allocated in user application */
  char *cookie;                                /**<Field that will hold cookie data. Must be allocated in user application */
  char *custom_fields;                         /**<Field that will hold additional custom fields. Must be allocated in user application */
} AZX_HTTP_HEADER;


/*!
  \struct AZX_HTTP_SSL

  \brief HTTP SSL structure

  \details This structure holds the parameters required to manage HTTP SSL connection (optional)

  \ingroup httpUsage

  <b>Refer to</b>
      AZX_HTTP_INFO
*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct
{
  M2MB_SSL_CTXT_HANDLE         sslH;                   /**<SSL context handle*/
  M2MB_SSL_CONFIG_HANDLE       sslConf;                /**<SSL configuration handle*/
  M2MB_SSL_AUTH_TYPE_E         sslAuthType;             /**<SSK Authentication type*/
  CHAR
  *CA_CERT_FILEPATH;           /**<SSL Root CA certificate file path (NULL if not needed)*/
  CHAR
  *CLIENT_CERT_FILEPATH;       /**<SSL client certificate file path (NULL if not needed)*/
  CHAR
  *CLIENT_KEY_FILEPATH;        /**<SSL client private key file path (NULL if not needed)*/
} AZX_HTTP_SSL;


/*!
  \struct AZX_HTTP_URL

  \brief HTTP URL structure

  \details This structure holds the parameters required to manage HTTP URL information

  \ingroup httpUsage

  <b>Refer to</b>
      HTTP_INFO
*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct
{

  int    https;                              /**<Is connection secure?*/
  char   host[256];                       /**<Destination host name*/
  int    port;                               /**<Destination host port*/
  char   path[AZX_HTTP_H_FIELD_SIZE];         /**<Destination URI (e.g. "/index.html" */
  char   auth_credentials[256];       /**< Authentication credentials. Will contain "username:password" base64 encoded */
} AZX_HTTP_URL;

/* The required option to correctly use the callback function */

/*!
  @brief
    User callback prototype

  @details
    This is the function signature (user must define one and pass it to will be called by the client to allow user receive and manage incoming data.

  @param[in]
       Incoming data buffer
  @param[in]
       Incoming data length
  @param[inout]
       the Event Flag pointer. It can be used to force the HTTP client to stop current operation.


  @return
       Integer value, not used

  <b>Refer to</b>
  azx_https_set_CB() azx_httpCallbackOptions

  @ingroup httpConf
*/
/*-----------------------------------------------------------------------------------------------*/
typedef INT32( *AZX_HTTPS_USER_CB )( void *, UINT32, INT8 * );


/*!
  @brief
    User base 64 encoder function prototype

  @details
    This is the function signature (user must define one and pass it to the https_init if basic 
    authentication is required

  @param[out] output
       allocated buffer that will contain the encoded credentials string
  @param[in] input
       Input credentials
  @param[in] inputLen
       the length in bytes of the input buffer
    
  <b>Refer to</b>
  azx_https_get() 

  @ingroup httpConf
*/
/*-----------------------------------------------------------------------------------------------*/
typedef void (*user_base64_encode)( UINT8 *out, const UINT8 *in, int inlen );

/*!
  @struct azx_httpCallbackOptions

  @brief The HTTP callback options structure

  @details This structure holds the parameters necessary to set the user callback that will be executed every time data is received from remote

  <b>Refer to</b>
      azx_http_setCB()

  @ingroup httpConf
*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct
{
  UINT32
  user_cb_bytes_size;    /**< The buffer size. Every time the buffer content reaches this size, the callback function will executed. */
  void
  *cbData;               /**< The buffer pointer (already allocated) that will contain the data to output with the callback */
  INT8
  *cbEvtFlag;            /**< Callback event pointer. This is a flag. When activated (set to a value different from 0) from user application side this leads the HTTP read process to be forcefully stopped */
  AZX_HTTPS_USER_CB cbFunc;                /**< Function pointer to the callback function */
} azx_httpCallbackOptions;

/*!
  @struct AZX_HTTP_INFO

  @brief The HTTP operation structure

  @details This structure holds all the parameters required to perform the HTTP operations

  <b>Refer to</b>
      azx_http_get()

  @ingroup httpUsage
*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct
{
  int ssl_fd;                         /**< \internal */
  int sck_fd;                         /**< \internal */

  AZX_HTTP_URL    url;                /**< \internal */

  AZX_HTTP_HEADER request;            /**< \internal */
  AZX_HTTP_HEADER response;           /**< \internal */
  AZX_HTTP_SSL    tls;                /**< \internal */

  UINT32         length;                 /**< \internal  body length or chunk length remaining to read */
  BOOLEAN        header_end;             /**< \internal */
  BOOLEAN        body_start;             /**< \internal */
  azx_httpCallbackOptions http_cb;    /**< \internal */
  user_base64_encode user_b64encode;  /**< \internal */
  long       r_len;                   /**< \internal */
  long    r_size;                     /**< \internal */
  char      *w_buf;                   /**< \internal */
  long    w_len;                      /**< \internal */

  long    chunk_buf_len;           /**< \internal */
  char      r_buf[AZX_HTTP_H_READ_SIZE];   /**< \internal */
  char      chunk_size_buf[AZX_HTTP_H_CHUNK_SIZE]; /**< \internal */

} AZX_HTTP_INFO;



/*!
   \brief HTTP Log global level
   \ingroup httpConf
*/
typedef enum
{
  AZX_HTTP_LOG_NONE = 0,             /**<Do not print any message*/
  AZX_HTTP_LOG_ERROR = 1,            /**<Print an error message*/
  AZX_HTTP_LOG_INFO = 2,              /**<Print an information message*/
  AZX_HTTP_LOG_DEBUG = 3             /**<Print a debug message*/
} AZX_HTTP_LOG_HOOK_LEVELS_E;



/* Global functions ==========================================================*/

/*!
  \brief
    Debug callback that is called by the library to for debug purposes.

  \details
    This function prototype can be registered by the user with the httpInit call.
    It is user's responsibility to define the logic of all debug levels (see
    HTTP_LOG_HOOK_LEVELS_E enum above ) and how they are printed.

  \param[in] level
    Debug level, see HTTP_DEBUG_HOOK_LEVELS

  \param[in] function
    This indicates the function name where the debug print is occurring

  \param[in] file
    This indicates the .c file name where the debug print is occurring

  \param[in] line
    This indicates the .c file line number where the debug print is occurring

  \param[in] fmt
    format string (as in printf). It contains the log message to be printed,
    plus possible additional parameters, to be used with va_list.

  <b>Refer to</b>
      azx_http_initialize()

   \return
        The printed bytes

   \ingroup httpConf

  <b>Sample usage</b>
  \code
    #include <stdarg.h>

    int (AZX_HTTP_LOG_HOOK_LEVELS_E level, const char *function, const char *file, int line, const char *fmt, ...)
    {
      char buf[512];
      int bufSize = sizeof(buf);
      va_list arg;
      int offset = 0;
      memset(buf,0,bufSize);
      switch(level)
      {
                case HTTP_LOG_DEBUG:
                    offset = sprintf(buf, "ERR %32s:%-4d - ",
                                        function,
                                        line
                                        );

                break;

                case HTTP_LOG_INFO:
                    offset = 0;
                break;

                case HTTP_LOG_ERROR:
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
  \endcode
*/
/*-----------------------------------------------------------------------------------------------*/
typedef int ( *azx_httpDebugHook )( AZX_HTTP_LOG_HOOK_LEVELS_E level, const char *function,
                                    const char *file, int line, const char *fmt, ... );

/*!
  @struct AZX_HTTPS_OPTIONS

  @brief The HTTP configuration options

  @details This structure holds is used to configure the HTTP client options

  <b>Refer to</b>
      azx_http_initialize()

  @ingroup httpConf
*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char cid;                               /**< Context id to be used for connection */
  AZX_HTTP_LOG_HOOK_LEVELS_E loglevel;    /**< Global debug Level to set */
  azx_httpDebugHook logFunc;                    /**< Function to call for prints */
  void *logArg;                                      /**< Parameter for logging function. Unused*/
  int padding;                                        /**< Padding*/
} AZX_HTTP_OPTIONS;

/*! \cond PRIVATE */
extern AZX_HTTP_OPTIONS *hi_struct;

#define AZX_HTTP_LOG(l,a...)   ({ { if(hi_struct->loglevel >= l && hi_struct->logFunc) {hi_struct->logFunc(l, __FUNCTION__, __FILE__, __LINE__, a);}}})
/*! \endcond */

/*-----------------------------------------------------------------------------------------------*/
/*!
  \brief
    Initialize SSL data

  \details
    Initialize SSL context and load the client/CA certificates according to the authentication method

  \param[in] sslInfo
    HTTP_SSL struct will contain certificate and context

  \param[in] authType
    ssl authentication type (None, Client, Client and Server)

  \return
    0 - Ok
   \return
     -1 - Failure

  <b>Refer to</b>
   AZX_HTTP_SSL

  \ingroup httpConf

  <b>Sample usage</b>
  \code
    AZX_HTTP_SSL  tls = {0};
    if(-1 == azx_http_SSLInit(&tls))
    {
      AZX_LOG_INFO("SSL init error \r\n");
    }
  \endcode
*/
/*-----------------------------------------------------------------------------------------------*/
int  azx_http_SSLInit( AZX_HTTP_SSL *sslInfo );

/*-----------------------------------------------------------------------------------------------*/
/*!
  \brief
    Initialize HTTPs client parameters

  \details
    Initialize HTTPs parameters such CID, log function, log level

  \param[in] opts
     HTTPS_OPTIONS struct that will contain Context ID to be used, log function hook, log level

   \return
        None

  \ingroup httpConf

  <b>Sample usage</b>
  \code
    int MYLOG(HTTP_LOG_HOOK_LEVELS_E level, const char *function, const char *file, int line, const char *fmt, ...)
    {
      [...]
    }
    AZX_HTTP_OPTIONS opt;

    opt.cbFunc = MYPRINT;
    opt.cid = 1;
    opt.level = HTTP_LOG_INFO;
    azx_http_initialize(&opt);
  \endcode
*/
/*-----------------------------------------------------------------------------------------------*/
void azx_http_initialize( AZX_HTTP_OPTIONS *opts );

/*-----------------------------------------------------------------------------------------------*/
/*!
  \brief
    Run a HTTP GET request

  \details
    Run a HTTP GET request using the URL info and the additional info stored in HTTP_INFO info

  \param[inout] hi
    AZX_HTTP_INFO struct used to provide additional data for the request (i.e the HTTP authentication type) and store the retrieved metadata from the header

  \param[in] url
    the URL to parse for the HTTP request

  \return
    HTTP response code - Ok
  \return
     -1   - Failure

  <b>Refer to</b>
    azx_http_initialize() azx_http_SSLInit() azx_http_set_CB()

  @ingroup httpUsage

  <b>Sample usage</b>
  \code
      AZX_HTTP_INFO hi;
      memset(&hi, 0, sizeof(HTTP_INFO));
      hi.request.auth_type = AuthSchemaBasic;
      azx_http_get(&hi,"http://guest:guest@jigsaw.w3.org/HTTP/Basic/");
  \endcode
*/
/*-----------------------------------------------------------------------------------------------*/
int  azx_http_get( AZX_HTTP_INFO *hi, char *url );
/** \example{lineno} azx_https_example.c
   This is a detailed example of cJSON functions usage.
*/


/*-----------------------------------------------------------------------------------------------*/
/*!
  \brief
    Run a HTTP HEAD request

  \details
    Run a HTTP HEAD request using the URL info

  \param[inout] hi
    AZX_HTTP_INFO struct used to store the retrieved metadata from the header

  \param[in] url
    the URL to parse for the HTTP request

  \return
    HTTP response code - Ok
  \return
     -1   - Failure

  <b>Refer to</b>
    azx_http_initialize() azx_http_SSLInit() azx_http_set_CB()

  @ingroup httpUsage

  <b>Sample usage</b>
  \code
      AZX_HTTP_INFO hi;
      memset(&hi, 0, sizeof(HTTP_INFO));
      azx_http_head(&hi,(char*) "https://www.google.com");
  \endcode
*/
/*-----------------------------------------------------------------------------------------------*/
int  azx_http_head( AZX_HTTP_INFO *hi, char *url );
/*-----------------------------------------------------------------------------------------------*/

/*!
  \brief
    Run a HTTP POST request

  \details
    Run a HTTP POST request using the URL info and the additional info stored in HTTP_INFO info

  \param[inout] hi
    AZX_HTTP_INFO struct used to provide additional data for the request (i.e the POST data) and store the retrieved metadata from the header

  \param[in] url
    the URL to parse for the HTTP request

   \return
    HTTP response code - Ok
  \return
     -1   - Failure

  <b>Refer to</b>
    azx_http_initialize() azx_http_SSLInit() azx_http_set_CB()

  @ingroup httpUsage

  <b>Sample usage</b>
  \code
      AZX_HTTP_INFO hi;
      memset(&hi, 0, sizeof(AZX_HTTP_INFO));
      char *data = "This is the POST data";
      hi.request.post_data = (char *)m2mb_os_malloc(sizeof(char)* (strlen(data) + 1));
      memset(hi.request.post_data, 0, sizeof(sizeof(char)* (strlen(data) + 1)));
      strcpy(hi.request.post_data,data);
      ret = azx_http_post(&hi,(char *)"http://postman-echo.com:80/post");
  \endcode
*/
/*-----------------------------------------------------------------------------------------------*/
int  azx_http_post( AZX_HTTP_INFO *hi, char *url );
/*-----------------------------------------------------------------------------------------------*/
/*!
  \brief
    Sets user callback

  \details
    This function will set the user callback options

  \param[out] hi
    AZX_HTTP_INFO struct used to store the retrieved metadata from the header

  \param[in] http_cb
    the callback options

  \return
     None

  <b>Refer to</b>
    azx_httpCallbackOptions

  @ingroup httpConf

  <b>Sample usage</b>
  \code
      INT8 cbEvt = 0;

      int DATA_CB(char* buffer, UINT32 size, INT8 *flag)
      {
        //set stop flag if needed
        // *flag = 1;

        return 0;
      }

      azx_httpCallbackOptions cbOpt;

      cbOpt.user_cb_bytes_size = 1500;
      cbOpt.cbFunc = DATA_CB;
      cbOpt.cbData = m2mb_os_malloc(cbOpt.user_cb_bytes_size + 1); //one more element for \0
      cbOpt.cbEvtFlag = &cbEvt;

      AZX_HTTP_INFO hi;
      memset(&hi, 0, sizeof(HTTP_INFO));
      azx_http_setCB(&hi,cbOpt);

  \endcode
*/
/*-----------------------------------------------------------------------------------------------*/
void azx_http_setCB( AZX_HTTP_INFO *hi, azx_httpCallbackOptions http_cb );


/*!
  \brief
    Return configured CID

  \details
    This function will return the configured CID


  \return
     CID value

  <b>Refer to</b>
    azx_http_initialize

  @ingroup httpUsage

*/
/*-----------------------------------------------------------------------------------------------*/
UINT8 azx_http_getCID( void );


#endif //AZX_HTTPS_CLIENT_H

