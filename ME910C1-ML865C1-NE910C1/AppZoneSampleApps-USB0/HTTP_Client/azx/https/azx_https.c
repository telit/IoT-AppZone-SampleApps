/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/* Include files =============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "m2mb_types.h"

#ifndef _ARMABI
#include <unistd.h>
#endif
#include "m2mb_os_types.h"
#include "m2mb_os.h"
#include "m2mb_socket.h"
#include "m2mb_ssl.h"
#include "m2mb_fs_posix.h"
#include "m2mb_fs_errno.h"


#include "azx_log.h"
#include "azx_gnu_sys_time.h"
#include "azx_gnu_sys_types.h"
#include "azx_gnu_sys_socket.h"
#include "m2mb_fs_posix.h"

#include "azx_string_utils.h"

#include "azx_https.h"


/* Local defines =============================================================*/

#define SSL_CERT_CA_NAME "ca-cert-pool"
#define SSL_CLIENT_NAME "SSL-Client"

#define REQUEST_SIZE 4096


/* Local typedefs ============================================================*/
typedef struct
{
  AZX_HTTP_LOG_HOOK_LEVELS_E loglevel;    /* Global debug Level to set */
  azx_httpDebugHook logFunc;              /* function to call for prints */
  void *logArg;
  int padding[2];                         /*< Padding to be bigger than AZX_HTTP_OPTIONS*/
  int init;
} HTTPS_PARAMS_T;

/* Local statics =============================================================*/
static M2MB_SSL_CIPHER_SUITE_E CipherSuites[M2MB_SSL_MAX_CIPHERSUITES];
static HTTPS_PARAMS_T https_params; /* internal */



AZX_HTTP_OPTIONS *hi_struct = ( AZX_HTTP_OPTIONS * ) &https_params;


/* Local function prototypes =================================================*/
static char *methodTo_string( AZX_HTTP_METHOD method );
static char *auth_schemaTo_string( AZX_HTTP_AUTH_SCHEMA auth_schema );
static int strtoken( char *src, char *field_title, char *field_value );

static int parse_url( char *src_url, int *https, char *host, int *port, char *url,
                      char *auth_credentials );

static int https_init( AZX_HTTP_INFO *hi, char *url );
static int http_isInit( void );
static int https_header( AZX_HTTP_INFO *hi, char *param );
static int https_parse( AZX_HTTP_INFO *hi );

static int https_open( AZX_HTTP_INFO *hi, char *url );
static int https_close( AZX_HTTP_INFO *hi );
static int http_connect( AZX_HTTP_INFO *hi, int proto );
static int https_secure_connect( AZX_HTTP_INFO *hi );
static int https_write( AZX_HTTP_INFO *hi, char *buffer, int len );
static int https_write_header( AZX_HTTP_INFO *hi );
static int https_read( AZX_HTTP_INFO *hi, char *buffer, int len );
static int https_read_chunked( AZX_HTTP_INFO *hi, BOOLEAN only_header );

static int https_SSLDeinit( AZX_HTTP_SSL *sslInfo );

static int readCertFile( char *certFilePath, UINT8 **certBuf, SIZE_T *st_size );


/* Static functions ==========================================================*/
static char *methodTo_string( AZX_HTTP_METHOD method )
{
  switch( method )
  {
    case AZX_HTTP_GET :
      return ( char * )"GET";

    case AZX_HTTP_POST :
      return ( char * )"POST";

    case AZX_HTTP_HEAD :
      return ( char * )"HEAD";

    default:
      return ( char * )"NOT VALID";
  }
}

static char *auth_schemaTo_string( AZX_HTTP_AUTH_SCHEMA auth_schema )
{
  switch( auth_schema )
  {
    case azx_AuthSchemaNone :
      return ( char * )"NONE";

    case azx_AuthSchemaBasic :
      return ( char * )"Basic";

    case azx_AuthNotSupported :
      return ( char * )"NOT SUPPORTED";

    default:
      return ( char * )"NOT VALID";
  }
}
#define MIN(i,j) (((i) < (j)) ? (i) : (j))

static int strtoken( char *src, char *field_title, char *field_value )
{
  char *p_end_line, *p_split;
  char current_line[256] = {0};
  int len = 0;
  if( ( p_end_line = strstr( src, "\r\n" ) ) == NULL )
  {
    return 1;
  }
  len = MIN( (UINT32)(p_end_line - src), (UINT32)(sizeof(current_line) - 2 ) );
  strncpy( current_line, src, len );
  current_line[len] = '\0';

  if( ( p_split = strstr( current_line, ":" ) ) == NULL )
  {
    strcpy( field_title, current_line );
    return 0;
  }

  strncpy( field_title, current_line, p_split - current_line );
  strcpy( field_value, p_split + 1 );
  azx_str_l_trim( field_title );
  azx_str_r_trim( field_title );
  azx_str_l_trim( field_value );
  azx_str_r_trim( field_value );
  return 0;
}


static int parse_url( char *src_url, int *https, char *host, int *port, char *url,
                      char *auth_credentials )
{
  char *p1, *p2;
  char str[300];
  char port_buffer[6];
  memset( str, 0, sizeof( str ) );

  if( strncmp( src_url, "http://", 7 ) == 0 )
  {
    p1 = &src_url[7];
    *https = 0;
  }
  else
    if( strncmp( src_url, "https://", 8 ) == 0 )
    {
      p1 = &src_url[8];
      *https = 1;
    }
    else
    {
      p1 = &src_url[0];
      *https = 0;
    }

  //auth
  if( ( p2 = strstr( p1, "@" ) ) != NULL )
  {
    strncpy( auth_credentials, p1, p2 - p1 );
    p1 = p2 + 1;
  }

  if( ( p2 = strstr( p1, "/" ) ) == NULL )
  {
    sprintf( str, "%s", p1 );
    sprintf( url, "/" );
  }
  else
  {
    strncpy( str, p1, p2 - p1 );
    snprintf( url, 256, "%s", p2 );
  }

  if( ( p1 = strstr( str, ":" ) ) != NULL )
  {
    *p1 = 0;
    snprintf( host, 256, "%s", str );
    snprintf( port_buffer, 6, "%s", p1 + 1 );
  }
  else
  {
    snprintf( host, 256, "%s", str );

    if( *https == 0 )
    {
      snprintf( port_buffer, 5, "80" );
    }
    else
    {
      snprintf( port_buffer, 5, "443" );
    }
  }
  *port = atoi(port_buffer);

  return 0;

}

static int https_init( AZX_HTTP_INFO *hi, char *url )
{
  char auth_credentials[256] = {0};

  if( !http_isInit() )
  {
    return -1;
  }

  parse_url( url, &hi->url.https, hi->url.host, &hi->url.port, hi->url.path, auth_credentials );

  if( strlen( auth_credentials ) )
  {
    if (hi->user_b64encode != NULL)
    {
      hi->user_b64encode( ( unsigned char * )hi->url.auth_credentials,
                       ( const unsigned char * )auth_credentials, strlen( auth_credentials ) );
      AZX_HTTP_LOG(AZX_HTTP_LOG_DEBUG, "\nCredentials (B64): %s\n\r",hi->url.auth_credentials);
    }
    else
    {
      https_close( hi );
      return -1;
    }

  }

  AZX_HTTP_LOG(AZX_HTTP_LOG_INFO, "Connecting to %s:%d/%s\n\r", hi->url.host, hi->url.port, hi->url.path );

  if( ( https_secure_connect( hi ) ) < 0 )
  {
    https_close( hi );
    return -1;
  }

  return 0;
}

static int http_isInit( void )
{
  return ( https_params.init == 1 );
}



//##################################################################################################################################
/**
    \brief Decode the metadata contained in the HTTP header response

    \param [in] param:    complete http header string


*/
//##################################################################################################################################
static int https_header( AZX_HTTP_INFO *hi, char *header )
{
  int  len, ret_tok;
  char t1[AZX_HTTP_H_FIELD_SIZE];
  char t2[AZX_HTTP_H_FIELD_SIZE];

  while( header != NULL )
  {
    memset( t1, 0, AZX_HTTP_H_FIELD_SIZE );
    memset( t2, 0, AZX_HTTP_H_FIELD_SIZE );
    ret_tok = strtoken( header, t1, t2 );

    if( ret_tok > 0 ) // finish
    {
      break;
    }

    if( strncasecmp( t1, "HTTP", 4 ) ==
        0 ) // HTTP doesn't split by : so extract status code looking for the spaces
    {
      char *pt1, *pt2;

      if( ( pt1 = strstr( t1, " " ) ) != NULL )
      {
        azx_str_l_trim( pt1 );
        azx_str_r_trim( pt1 );

        if( ( pt2 = strstr( pt1, " " ) ) != NULL )
        {
          int i;
          UINT32 code;
          int res;
          for (i = 0; i < (pt2 - pt1); i++)
          {
            t1[i] = pt1[i];
          }
          t1[pt2 - pt1] = 0;
          azx_str_l_trim( pt1 );
          azx_str_r_trim( pt1 );
          res = azx_str_to_ul(t1, &code);
          if(res != 0)
          {
            hi->response.status = 0;
            AZX_HTTP_LOG(AZX_HTTP_LOG_ERROR, "Cannot parse %d\r\n",res);
          }
          else
          {
            hi->response.status = code;
            AZX_HTTP_LOG(AZX_HTTP_LOG_DEBUG, "<%d>\r\n", hi->response.status);
          }

        }
      }
    }
    else
      if( strncasecmp( t1, "Set-cookie", 10 ) == 0 )
      {
        /* hi->response.cookie = ( char * )m2mb_os_malloc( 512 ); */

        if( hi->response.cookie != NULL )
        {
          snprintf( hi->response.cookie, AZX_HTTP_H_FIELD_SIZE, "%s", t2 );
        }
      }
      else
        if( strncasecmp( t1, "Location", 8 ) == 0 )
        {
          len = ( int )strlen( t2 );
          strncpy( hi->response.location, t2, len );
          hi->response.location[len] = 0;
        }
        else
          if( strncasecmp( t1, "Content-length", 14 ) == 0 )
          {
            hi->response.content_length = atoi( t2 );
          }
          else
            if( strncasecmp( t1, "Transfer-encoding", 17 ) == 0 )
            {
              if( strncasecmp( t2, "Chunked", 7 ) == 0 )
              {
                hi->response.chunked = TRUE;
              }
            }
            else
              if( strncasecmp( t1, "Connection", 10 ) == 0 )
              {
                if( strncasecmp( t2, "close", 5 ) == 0 )
                {
                  hi->response.close = TRUE;
                }
              }
              else if(strncasecmp( t1, "Content-Range", 13 ) == 0)
              {
                if( hi->response.content_range != NULL )
                {
                  snprintf(  hi->response.content_range, AZX_HTTP_H_FIELD_SIZE, "%s", t2 );
                }
              }

    header = strstr( header, "\r\n" );

    if( header != NULL )
    {
      header = header + 2; // skip CR+LF
    }
  }

  return 1;
}

static int https_parse( AZX_HTTP_INFO *hi )
{
  char    *p1;

  if( hi->r_len <= 0 )
  {
    return -1;
  }

  p1 = hi->r_buf;

  if( hi->header_end == FALSE )   // header parser
  {
    if( (UINT32) hi->w_len >=  hi->http_cb.user_cb_bytes_size )
    {
      return -1;
    }

    strncpy( &( hi->w_buf[hi->w_len] ), p1, 1 );
    hi->w_len += 1;
    hi->r_len = 0;

    if( hi->w_len > 4 && !strcmp( "\r\n\r\n", &hi->w_buf[hi->w_len - 4] ) )
    {
      hi->header_end = TRUE;
      hi->w_buf[hi->w_len] = 0;
      https_header( hi, hi->w_buf );
      hi->length = hi->response.content_length;
    }
  }
  else
  {
    hi->body_start = TRUE;

    if( hi->response.chunked == TRUE && hi->length == 0 ) //begin of the chunk. Read the chunk-size
    {
      memcpy( &( hi->chunk_size_buf[hi->chunk_buf_len] ), p1, hi->r_size );
      hi->chunk_buf_len += 1;
      hi->r_len = 0;

      if( hi->chunk_buf_len > 2 &&
          !strcmp( "\r\n", &hi->chunk_size_buf[hi->chunk_buf_len - 2] ) ) // remove CR + LF from chunk size
      {
        azx_str_rem_ch( hi->chunk_size_buf, '\r' );
        azx_str_rem_ch( hi->chunk_size_buf, '\n' );

        if( azx_str_to_ul_hex( hi->chunk_size_buf, (UINT32 *) &hi->length ) != 0 )
        {
          return -1;
        }

        if( hi->length == 0 ) //meaning you reached the and
        {
          return 1;
        }

        hi->r_size = (hi->length > AZX_HTTP_H_READ_SIZE )? AZX_HTTP_H_READ_SIZE : hi->length ;
        //clean writing buffer
        memset( hi->chunk_size_buf, 0, AZX_HTTP_H_CHUNK_SIZE );
        hi->chunk_buf_len = 0;
      }
    }
    else   //if length > 0 the client started to read the chunk
    {
      if((UINT32) hi->r_len < ( hi->http_cb.user_cb_bytes_size -
                        hi->w_len ) ) //if w_buf can contain all the read data. copy all
      {
        memcpy( &( hi->w_buf[hi->w_len] ), p1, hi->r_len );
        hi->w_len += hi->r_len;
        hi->length -= hi->r_len;
        hi->r_len = 0;
      }
      else                                                          //else copy only the partial data to fill w_buf
      {
        int part_len = hi->http_cb.user_cb_bytes_size - hi->w_len;
        memcpy( &( hi->w_buf[hi->w_len] ), p1, ( part_len ) );
        //trim the read buffer in order to let only the data not copied in w_buff for the next run
        hi->r_len -=  part_len;
        memcpy( hi->r_buf, p1 + ( part_len ), hi->r_len );
        hi->w_len = hi->http_cb.user_cb_bytes_size;
        hi->r_buf[hi->r_len] = 0;
        hi->length -= part_len;
      }

      if( hi->length < (UINT32) hi->r_size )
      {
        hi->r_size = hi->length;
      }

      if( hi->length <= 0 )
      {
        if( hi->response.chunked == FALSE ) //finish
        {
          return 1;
        }

        hi->r_size = 1; //set and continue to read header byte-per-byte
      }
    }
  }

  return 0;
}

static int https_open( AZX_HTTP_INFO *hi, char *url )
{
  if( NULL == hi )
  {
    return -1;
  }

  if( https_init( hi, url ) == -1 )
  {
    return -1;
  }

  return 0;
}

static int https_close( AZX_HTTP_INFO *hi )
{
  AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "\r\nHTTP client closed.\r\n" );

  if( hi->url.https == 1 )
  {
    m2mb_ssl_shutdown( hi->ssl_fd );
    https_SSLDeinit( &hi->tls );
  }

  return m2mb_socket_bsd_close( hi->sck_fd );
}


static int http_connect( AZX_HTTP_INFO *hi, int proto )
{
  struct addrinfo hints;
  struct addrinfo *res0;
  struct addrinfo *res;
  INT32 rc;

  char port_buffer[6] = {0};

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = proto; /*sock_stream*/
  hints.ai_protocol = IPPROTO_TCP;

  /*getaddrinfo requires the port as a string, generate it from the number*/
  snprintf(port_buffer, sizeof(port_buffer) - 1, "%u", hi->url.port );

  if(getaddrinfo(hi->url.host, port_buffer, &hints, &res0) != 0)
  {

    AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "Cannot retrieve IP\r\n" );
    return -1;
  }

  for(res = res0; res; res = res->ai_next)
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "Retrieved Server IP Address on Address Family %d\r\n", res->ai_family );
    hi->sck_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(hi->sck_fd < 0)
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "Cannot open socket: %d\r\n", m2mb_socket_errno());
      continue;
    }

    if(hi_struct->loglevel >= AZX_HTTP_LOG_DEBUG)
    {
      char ipAddr[256];
      memset(ipAddr, 0, sizeof(ipAddr));
      switch(res->ai_family)
      {
        case AF_INET6:
          AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "Computing ipv6 address\r\n");
          m2mb_socket_bsd_inet_ntop( AF_INET6, res->ai_addr, ( CHAR * )&( ipAddr ), sizeof( ipAddr ) );
          break;
        case AF_INET:
          AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "Computing ipv4 address\r\n");
          m2mb_socket_bsd_inet_ntop( AF_INET, res->ai_addr, ( CHAR * )&( ipAddr ), sizeof( ipAddr ) );
          break;
        default:
          AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "Unexpected socket family type! %d\r\n", res->ai_family );
          break;
      }
      AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "Retrieved Server IP Address: %s\r\n", ipAddr );
    }


    rc = m2mb_socket_set_cid(hi->sck_fd, azx_gnu_getgCID());

    if(rc != M2MB_OS_SUCCESS  )
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "Cannot set cid\r\n");
      closesocket(hi->sck_fd);
      continue;
    }
    if(connect(hi->sck_fd, res->ai_addr, res->ai_addrlen) < 0)
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR,  "CONNECT failed!\r\n" );
      closesocket(hi->sck_fd);
      hi->sck_fd = -1;
    }
    else
    {
      break;
    }
  }

  freeaddrinfo(res0);

  if(hi->sck_fd < 0)
  {
    return -1;
  }

  AZX_HTTP_LOG( AZX_HTTP_LOG_INFO, "Socket connected!\r\n\r\n" );
  return 0;
}

static int https_secure_connect( AZX_HTTP_INFO *hi )
{
  int ret;
  ret = http_connect( hi, SOCK_STREAM );

  if( ret != 0 )
  {
    return ret;
  }

  if( hi->url.https == 1 )
  {
    INT32 sslRes;
    hi->ssl_fd = m2mb_ssl_secure_socket( hi->tls.sslConf, hi->tls.sslH, hi->sck_fd, &sslRes );

    if( hi->ssl_fd == 0 )
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR,  "m2mb_ssl_secure_socket FAILED error %d \r\n", sslRes );
      return -1;
    }
    else
    {
      sslRes = m2mb_ssl_connect( hi->ssl_fd );
    }

    if( sslRes != 0 )
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR,  "m2mb_ssl_connect FAILED error %d \r\n", sslRes );
      return -1;
    }
    else
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_INFO, "m2mb_ssl_connect SUCCESS \r\n" );
    }
  }

  return 0;
}


static int https_read( AZX_HTTP_INFO *hi, char *buffer, int len )
{
  if( hi->url.https == 1 )
  {
    return m2mb_ssl_read( hi->ssl_fd, ( u_char * )buffer, ( size_t )len );
  }
  else
  {
    return recv( hi->sck_fd, ( u_char * )buffer, ( size_t )len, 0 );
  }
}



static int https_write( AZX_HTTP_INFO *hi, char *buffer, int len )
{
  int ret, slen = 0;

  AZX_HTTP_LOG(AZX_HTTP_LOG_DEBUG, "<%.*s>", len, buffer);
  while( 1 )
  {
    if( hi->url.https == 1 )
    {
      ret = m2mb_ssl_write( hi->ssl_fd, ( u_char * )&buffer[slen], ( size_t )( len - slen ) );
    }
    else
    {
      ret = send( hi->sck_fd, ( u_char * )&buffer[slen], ( size_t )( len - slen ), 0 );
    }

    if( ret == -1 )
    {
      continue;
    }
    else
      if( ret <= 0 )
      {
        return ret;
      }

    slen += ret;

    if( slen >= len )
    {
      break;
    }
  }

  return slen;
}

static int https_write_header( AZX_HTTP_INFO *hi )
{
  char        request[REQUEST_SIZE] = {0};
  int         len = 0;

  if( NULL == hi )
  {
    return -1;
  }

  /*Mandatory fields*/
  len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "%s %s HTTP/1.1\r\n",
                   methodTo_string( hi->request.method ), hi->url.path );

  if( hi->request.user_agent[0] != 0 )
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "User-Agent: %s\r\n",
                     hi->request.user_agent );
  }
  else
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "User-Agent: Mozilla/4.0\r\n" );
  }

  len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Host: %s:%d\r\n", hi->url.host,
                   hi->url.port );

  if( hi->request.content_type[0] != 0 )
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Content-Type: %s\r\n",
                     hi->request.content_type );
  }
  else
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE,
                     "Content-Type: text/html; charset=UTF-8\r\n" );
  }

  if( hi->request.close == TRUE )
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Connection: close\r\n" );
  }
  else
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Connection: Keep-Alive\r\n" );
  }

  /*Optional fields*/
  if( hi->request.accept_type[0] != 0 )
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Accept: %s\r\n", hi->request.accept_type );
  }

  if( hi->request.referrer[0] != 0 )
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Referer: %s\r\n", hi->request.referrer );
  }

  if( hi->request.chunked == TRUE )
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Transfer-Encoding: chunked\r\n" );
  }

  if( hi->request.content_length != 0 )
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Content-Length: %ld\r\n",
                     hi->request.content_length );
  }

  if( hi->request.auth_type != azx_AuthSchemaNone && hi->request.auth_type != azx_AuthNotSupported )
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Authorization: %s %s\r\n",
                     auth_schemaTo_string( hi->request.auth_type ), hi->url.auth_credentials );
  }

  if( hi->request.cookie != NULL )
  {
    len += snprintf( &request[len], strlen( hi->request.cookie ) + 3, "Cookie: %s\r\n",
                     hi->request.cookie );  //+3 -> strlen + \0 + \r\n
  }

  if( hi->request.custom_fields != NULL )
  {
    len += snprintf( &request[len], strlen( hi->request.custom_fields ) + 3, "%s\r\n",
                     hi->request.custom_fields );
  }

  if( hi->request.post_data != NULL )
  {
    len += snprintf( &request[len], strlen( hi->request.post_data ) + 3, "\r\n%s",
                     hi->request.post_data );
  }

  if( hi->request.content_range != NULL )
  {
    len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "Range: bytes=%s\r\n", hi->request.content_range );
  }

  len += snprintf( &request[len], AZX_HTTP_H_FIELD_SIZE, "\r\n" );

  request[len] = 0;

  AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "Header: %s\r\n", request);
  if( ( https_write( hi, request, len ) ) != len )
  {
    https_close( hi );
    return -1;
  }

  return 0;
}



static int https_read_chunked( AZX_HTTP_INFO *hi, BOOLEAN only_header )
{
  int ret;
  int pars_ret = 0;


  if( NULL == hi )
  {
    return -1;
  }

  hi->response.status = 0;
  hi->response.content_length = 0;
  hi->response.close = 0;
  hi->header_end = 0;
  hi->body_start = 0;
  hi->w_buf = ( char * ) hi->http_cb.cbData;
  hi->r_len = 0;
  hi->w_len = 0;
  hi->r_size = 1; //start reading header byte-per-byte
  memset( hi->r_buf, 0, AZX_HTTP_H_READ_SIZE );
  memset( hi->w_buf, 0, hi->http_cb.user_cb_bytes_size );
  hi->chunk_buf_len = 0;
  memset( hi->chunk_size_buf, 0, AZX_HTTP_H_CHUNK_SIZE );

  while( 1 )
  {
    memset( hi->r_buf, 0, AZX_HTTP_H_READ_SIZE );
    ret = https_read( hi, &hi->r_buf[0], ( int ) hi->r_size );


    if( ret == -1 )
    {
      continue;
    }
    else
      if( ret < 0 )
      {
        return -1;
      }
      else
        if( ret == 0 )
        {
          break;
        }

    hi->r_len = ret;
    hi->r_buf[hi->r_len] = 0;

    do
    {
      if( ( pars_ret = https_parse( hi ) ) < 0 )
      {
        return pars_ret;
      }

      if( hi->header_end && !hi->body_start )
      {
        if( only_header ) // HTTP HEAD
        {
          if( hi->http_cb.cbFunc != NULL )
          {
            hi->http_cb.cbFunc( hi->w_buf, hi->w_len, hi->http_cb.cbArg );
          }

          return hi->response.status;
        }

        //clean writing buffer
        memset( hi->w_buf, 0, hi->http_cb.user_cb_bytes_size );
        hi->w_len = 0;
      }

      if( (UINT32) hi->w_len >= hi->http_cb.user_cb_bytes_size )
      {
        if( hi->http_cb.cbFunc != NULL )
        {

          if(0 != hi->http_cb.cbFunc( hi->w_buf, hi->w_len, hi->http_cb.cbArg ))
          {
            return hi->response.status;
          }
        }

        hi->w_len = 0;
        memset( hi->w_buf, 0, hi->http_cb.user_cb_bytes_size );
      }

    }
    while( hi->r_len > 0 );

    if( pars_ret > 0 )
    {
      break;
    }
  }

  if( hi->http_cb.cbFunc != NULL )
  {
    hi->http_cb.cbFunc( hi->w_buf, hi->w_len, hi->http_cb.cbArg );
  }

  return hi->response.status;
}


static int https_SSLDeinit( AZX_HTTP_SSL *sslInfo )
{
  INT32 res;

  if( ( sslInfo->sslAuthType == M2MB_SSL_SERVER_AUTH ) ||
      ( sslInfo->sslAuthType == M2MB_SSL_SERVER_CLIENT_AUTH ) )
  {
    res = m2mb_ssl_cert_delete( M2MB_SSL_CACERT, ( CHAR * )SSL_CERT_CA_NAME );

    if( res != 0 )
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "m2mb_ssl_cert_delete failed with code %d\r\n", res );
    }
  }

  if( sslInfo->sslAuthType == M2MB_SSL_SERVER_CLIENT_AUTH )
  {
    res = m2mb_ssl_cert_delete( M2MB_SSL_CERT, ( CHAR * )SSL_CLIENT_NAME );

    if( res != 0 )
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "m2mb_ssl_cert_delete failed with code %d\r\n", res );
    }
  }

  res = m2mb_ssl_delete_config( sslInfo->sslConf );

  if( res != 0 )
  {
    AZX_LOG_ERROR( "m2mb_ssl_delete_config failed with code %d\r\n", res );
  }

  m2mb_ssl_delete_ctxt( sslInfo->sslH );
  return 0;
}


static int readCertFile( char *certFilePath, UINT8 **certBuf, SIZE_T *st_size )
{
  AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "loading CERT from file %s\r\n", certFilePath );
  struct M2MB_STAT st;
  INT32 fd = -1;
  SIZE_T fs_res;
  if(-1 == m2mb_fs_stat( certFilePath, &st ))
  {
    return  -1;
  }
  *certBuf = ( UINT8 * ) m2mb_os_malloc( ( sizeof( UINT8 ) * ( st.st_size ) ) );

  if( !*certBuf )
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "Cannot allocate buffer for file content\r\n" );
    return  -1;
  }

  memset( *certBuf, 0, ( st.st_size ) );
  fd = m2mb_fs_open( certFilePath, M2MB_O_RDONLY );

  if( fd == -1 )
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "Cannot open file\r\n" );
    return  -1;
  }

  fs_res = m2mb_fs_read( fd, *certBuf, st.st_size );

  if( fs_res != st.st_size )
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "Failed reading buffer into file.\r\n" );
    return  -1;
  }

  AZX_HTTP_LOG( AZX_HTTP_LOG_INFO,
          "Buffer successfully received from file. %d bytes were loaded.\r\n", fs_res );
  *st_size = st.st_size;
  m2mb_fs_close( fd );
  return 0;
}


/* Global functions ==========================================================*/

void azx_http_initialize( AZX_HTTP_OPTIONS *opts )
{
  https_params.logFunc = opts->logFunc;
  https_params.logArg = opts->logArg;
  https_params.loglevel = opts->loglevel;
  azx_gnu_setgCID(opts->cid);
  https_params.init = 1;
}

UINT8 azx_http_getCID( void )
{
  return azx_gnu_getgCID();
}

void azx_http_setCB( AZX_HTTP_INFO *hi, azx_httpCallbackOptions http_cb )
{
  hi->http_cb.cbFunc = http_cb.cbFunc;
  hi->http_cb.cbData = http_cb.cbData;
  hi->http_cb.cbArg = http_cb.cbArg;
  hi->http_cb.user_cb_bytes_size = http_cb.user_cb_bytes_size;
}


int azx_http_SSLInit( AZX_HTTP_SSL *sslInfo )
{
  INT32 sslRes;
  M2MB_SSL_CONFIG_T sslConfig;

  sslConfig.ProtVers = M2MB_SSL_PROTOCOL_TLS_1_2;
  sslConfig.CipherSuites = CipherSuites;
  sslConfig.CipherSuites[0] = M2MB_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256;
  sslConfig.CipherSuites[1] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA;
  sslConfig.CipherSuites[2] = M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA;
  sslConfig.CipherSuites[3] = M2MB_TLS_DHE_RSA_WITH_AES_256_CBC_SHA;
  sslConfig.CipherSuites[4] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA;
  sslConfig.CipherSuites[5] = M2MB_TLS_RSA_WITH_AES_128_CBC_SHA256;
  sslConfig.CipherSuites[6] = M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256;
  sslConfig.CipherSuites[7] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA256;
  sslConfig.CipherSuitesNum = 8;
  sslConfig.AuthType = sslInfo->sslAuthType;

  if( !http_isInit() )
  {
    return -1;
  }
  sslInfo->sslConf = m2mb_ssl_create_config( sslConfig, &sslRes );

  if( sslInfo->sslConf == NULL )
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "m2mb_ssl_create_config FAILED error %d \r\n", sslRes );
    return -1;
  }
  else
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "m2mb_ssl_create_config PASSED \r\n" );
  }

  sslInfo->sslH = m2mb_ssl_create_ctxt();

  if( sslInfo->sslH == NULL )
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "m2mb_ssl_create_ctxt FAILED error \r\n" );
    return -1 ;
  }
  else
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "m2mb_ssl_create_ctxt PASSED \r\n" );
  }

  if( sslConfig.AuthType == M2MB_SSL_SERVER_AUTH ||
          sslConfig.AuthType == M2MB_SSL_SERVER_CLIENT_AUTH )
  {
    M2MB_SSL_SEC_INFO_U sslCertInfo;
    M2MB_SSL_CA_INFO_T ca_Info[M2MB_SSL_MAX_CA_LIST];
    sslCertInfo.ca_List.ca_Cnt = 1;
    sslCertInfo.ca_List.ca_Info[0] = &ca_Info[0];

    if( -1 == readCertFile( ( char * )sslInfo->CA_CERT_FILEPATH,
            &sslCertInfo.ca_List.ca_Info[0]->ca_Buf, &sslCertInfo.ca_List.ca_Info[0]->ca_Size ) )
    {
      return -1;
    }

    if( 0 != m2mb_ssl_cert_store( M2MB_SSL_CACERT, sslCertInfo, ( CHAR * ) SSL_CERT_CA_NAME ) )
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "cannot store root CA certificate \r\n" );
      return -1;
    }

    if( 0 != m2mb_ssl_cert_load( sslInfo->sslH, M2MB_SSL_CACERT, ( CHAR * ) SSL_CERT_CA_NAME ) )
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "cannot load root CA certificate \r\n" );
      return -1;
    }

    if( sslConfig.AuthType == M2MB_SSL_SERVER_CLIENT_AUTH )
    {
      if( -1 == readCertFile( ( char * )sslInfo->CLIENT_CERT_FILEPATH, &sslCertInfo.cert.cert_Buf,
              &sslCertInfo.cert.cert_Size ) )
      {
        return -1;
      }

      if( -1 == readCertFile( ( char * )sslInfo->CLIENT_KEY_FILEPATH, &sslCertInfo.cert.key_Buf,
              &sslCertInfo.cert.key_Size ) )
      {
        return -1;
      }

      if( 0 != m2mb_ssl_cert_store( M2MB_SSL_CERT, sslCertInfo, ( CHAR * ) SSL_CLIENT_NAME ) )
      {
        AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "cannot store Client certificate \r\n" );
        return -1;
      }

      if( 0 != m2mb_ssl_cert_load( sslInfo->sslH, M2MB_SSL_CERT, ( CHAR * ) SSL_CLIENT_NAME ) )
      {
        AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "cannot load Client certificate \r\n" );
        return -1;
      }
    }
  }

  if( sslConfig.AuthType != M2MB_SSL_NO_AUTH )
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_INFO, "Certificates successfully loaded!\r\n" );
  }

  return 0;
}




int azx_http_get( AZX_HTTP_INFO *hi, char *url )
{
  int         ret;

  if( https_open( hi, url ) != 0 )
  {
    return -1;
  }

  hi->request.method = AZX_HTTP_GET;

  if( https_write_header( hi ) == -1 )
  {
    https_close( hi );
    return -1;
  }

  ret = https_read_chunked( hi, FALSE );
  https_close( hi );
  return ret;
}

int azx_http_head( AZX_HTTP_INFO *hi, char *url )
{
  int         ret;

  if( https_open( hi, url ) != 0 )
  {
    return -1;
  }

  hi->request.method = AZX_HTTP_HEAD;

  if( https_write_header( hi ) == -1 )
  {
    https_close( hi );
    return -1;
  }

  ret = https_read_chunked( hi, TRUE );
  https_close( hi );
  return ret;
}


/*---------------------------------------------------------------------*/
int azx_http_post( AZX_HTTP_INFO *hi, char *url )
{
  int         ret;

  if( https_open( hi, url ) != 0 )
  {
    return -1;
  }

  hi->request.method = AZX_HTTP_POST;
  //hi->request.content_length = ( int ) strlen( hi->request.post_data );

  if( https_write_header( hi ) == -1 )
  {
    https_close( hi );
    return -1;
  }

  ret = https_read_chunked( hi, FALSE );
  https_close( hi );
  return ret;
}

int azx_http_post_file( AZX_HTTP_INFO *hi, char *url, char *filepath)
{
  int         ret;

  char chunk_buf[512];
  int chunk_size = sizeof(chunk_buf);
  SSIZE_T read;
  int data_size;
  INT32 fd;
  struct M2MB_STAT st;
  if( https_open( hi, url ) != 0 )
  {
    return -1;
  }

  hi->request.method = AZX_HTTP_POST;

  if (0 ==m2mb_fs_stat(filepath, &st))
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_DEBUG, "\r\nLocal file %s size: %u\r\n", filepath,  st.st_size);
    data_size = (int)st.st_size;
  }
  else
  {
    return -1;
  }

  hi->request.content_length = data_size;
  hi->request.post_data = NULL;

  if( https_write_header( hi ) == -1 )
  {
    https_close( hi );
    return -1;
  }

  fd = m2mb_fs_open(filepath, M2MB_O_RDONLY);
  if(fd == -1)
  {
    AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "Cannot open file %s for POST\r\n", filepath);
    return -1;
  }

  while(data_size > 0)
  {

    read = m2mb_fs_read(fd, chunk_buf, chunk_size);
    if (read < 0)
    {
      AZX_HTTP_LOG( AZX_HTTP_LOG_ERROR, "Cannot read file %s for POST. %d\r\n", filepath, m2mb_fs_get_errno_value());
      m2mb_fs_close(fd);
      return -1;
    }
    else
    {
      if( ( ret = https_write( hi, chunk_buf, read ) ) != read )
      {
        https_close( hi );
        m2mb_fs_close(fd);
        return -1;
      }
      else
      {
        if( hi->http_cb.cbFunc != NULL )
        {
          if(0 != hi->http_cb.cbFunc( NULL, ret, hi->http_cb.cbArg ))
          {
            https_close( hi );
            m2mb_fs_close(fd);
            return AZX_HTTP_CLIENT_CLOSED_REQUEST;
          }
        }
        data_size -= read;
      }
    }
  }

  m2mb_fs_close(fd);

  ret = https_read_chunked( hi, FALSE );
  https_close( hi );
  return ret;
}
