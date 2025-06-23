/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/***************************************************************************/
/*									   */
/* ftplib.c - callable ftp access routines				   */
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
/* 									   */
/***************************************************************************/

#if !defined(AZX)
#define AZX
#endif

#if defined(__GNUC__) && __GNUC__ >= 7 || defined(__clang__) && __clang_major__ >= 12
 #define FALL_THROUGH __attribute__ ((fallthrough))
#else
 #define FALL_THROUGH ((void)0) /* fall through */
#endif /* __GNUC__ >= 7 */

/* Include files =============================================================*/

#if defined(AZX)
#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "m2mb_net.h"
#include "m2mb_pdp.h"

#include "m2mb_socket.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_ssl.h"

#ifndef _STDIO_H_
#include <stdio.h>
#endif

#include <stddef.h>
#include "azx_gnu_stdio.h"
#include <stdlib.h>
#include "azx_gnu_stdlib.h"

#include "azx_gnu_sys_time.h"
#include "azx_gnu_sys_types.h"
#include "azx_gnu_sys_socket.h"

#include <limits.h>
#include <inttypes.h>

#include <string.h>
#include "azx_gnu_string.h"

#else //defined(AZX)
#if defined(__unix__) || defined(__VMS)
#include <unistd.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif //defined(AZX)

#include <errno.h>
#include <ctype.h>

#if !defined(AZX)
  #if defined(__unix__)
  #include <sys/time.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #elif defined(VMS)
  #include <types.h>
  #include <socket.h>
  #include <in.h>
  #include <netdb.h>
  #include <inet.h>
  #elif defined(_WIN32)
  #include <winsock.h>
  #endif
  #if defined(__APPLE__)
  #undef _REENTRANT
  #endif
#endif //!defined(AZX)



#if defined(AZX)
#include "azx_ftp.h"
#else
#include "ftplib.h"
#endif

/* Local defines =============================================================*/

#if defined(__UINT64_MAX) && !defined(PRIu64)
#if ULONG_MAX == __UINT32_MAX
#define PRIu64 "llu"
#else
#define PRIu64 "lu"
#endif
#endif


#if defined(_WIN32)
#define SETSOCKOPT_OPTVAL_TYPE (const CHAR *)
#else
#define SETSOCKOPT_OPTVAL_TYPE (void *)
#endif

#define AZX_FTP_BUFSIZE 8192
#define AZX_FTP_RESPONSE_BUFSIZE 1024

#define ACCEPT_TIMEOUT 30

#define AZX_FTP_CONTROL 0
#define AZX_FTP_READ 1
#define AZX_FTP_WRITE 2

#if !defined FTPLIB_DEFMODE
#define AZX_FTP_DEFMODE AZX_FTP_PASSIVE
#endif


#define version "ftplib Release 4.0 07-Jun-2013, copyright 1996-2003, 2013 Thomas Pfau"

/* Local typedefs ============================================================*/
struct AZX_FTP_NET_BUF_TAG {
  INT32 handle;
  INT32 cavail,cleft;
  INT32 dir;
  INT32 cmode;
  struct timeval idletime;
  azx_ftp_callback idlecb;
  void *idlearg;
  unsigned long int xfered;
  unsigned long int cbbytes;
  unsigned long int xfered1;
  INT32 abort;
  CHAR *cput,*cget;
  AZX_FTP_NET_BUF_T *ctrl;
  AZX_FTP_NET_BUF_T *data;
  CHAR *response;
  CHAR *buf;
#ifdef M2M_M2MB_SSL_H
  BOOLEAN AUTHdone;
  M2MB_SSL_CONNECTION_HANDLE sslHandle;
#endif
};

AZX_FTP_OPTIONS_T ftp_opts;


/* Local statics =============================================================*/
const char AZX_FTP_DEFAULT_PNUM[] = "ftp";

/* Local function prototypes =================================================*/
#if defined(AZX)
static CHAR *azx_ftp_strdup( const CHAR *s );
#endif

static INT32 net_read(INT32 fd, CHAR *buf, size_t len);
static INT32 net_write(INT32 fd, const CHAR *buf, size_t len);


static INT32 socket_wait(AZX_FTP_NET_BUF_T *ctl);
static INT32 readline(CHAR *buf, INT32 max, AZX_FTP_NET_BUF_T *ctl);
static INT32 writeline(const CHAR *buf, INT32 len, AZX_FTP_NET_BUF_T *nData);
static INT32 readresp(CHAR c, AZX_FTP_NET_BUF_T *nControl);
static M2MB_RESULT_E FtpAssociateCid( const CHAR *host, INT8 cid,
    struct M2MB_SOCKET_BSD_SOCKADDR_IN *sin, INT32 *sControl );
static INT32 FtpSendCmd(const CHAR *cmd, CHAR expresp, AZX_FTP_NET_BUF_T *nControl);
#ifdef M2M_M2MB_SSL_H
static INT32 FtpStartAuth(CHAR expresp, AZX_FTP_NET_BUF_T *nControl);
#endif
static INT32 FtpOpenPort(AZX_FTP_NET_BUF_T *nControl, AZX_FTP_NET_BUF_T **nData, INT32 mode, INT32 dir);
static INT32 FtpAcceptConnection(AZX_FTP_NET_BUF_T *nData, AZX_FTP_NET_BUF_T *nControl);
static INT32 FtpXferFile(AZX_FTP_FILE_INFO_T *localfile, AZX_FTP_FILE_INFO_T *remfileinfo,
    AZX_FTP_NET_BUF_T *nControl, INT32 typ, INT32 mode);
static INT32 FtpXferBuffer( AZX_FTP_BUFFER_T *localinfo, AZX_FTP_FILE_INFO_T *remfileinfo,
    AZX_FTP_NET_BUF_T *nControl, INT32 typ, INT32 mode);


/* Static functions ==========================================================*/

#if !defined(AZX) &&( defined(__unix__) || defined(VMS))
int net_read(int fd, CHAR *buf, size_t len)
{
  while ( 1 )
  {
    int c = read(fd, buf, len);
    if ( c == -1 )
    {
      if ( errno != EINTR && errno != EAGAIN )
        return -1;
    }
    else
    {
      return c;
    }
  }
}

int net_write(int fd, const CHAR *buf, size_t len)
{
  int done = 0;
  while ( len > 0 )
  {
    int c = write( fd, buf, len );
    if ( c == -1 )
    {
      if ( errno != EINTR && errno != EAGAIN )
        return -1;
    }
    else if ( c == 0 )
    {
      return done;
    }
    else
    {
      buf += c;
      done += c;
      len -= c;
    }
  }
  return done;
}
#define net_close close
#elif defined(_WIN32)
#define net_read(x,y,z) recv(x,y,z,0)
#define net_write(x,y,z) send(x,y,z,0)
#define net_close closesocket
#endif

#if defined(AZX)
static CHAR *azx_ftp_strdup( const CHAR *s )
{
  CHAR *newp = ( CHAR * ) m2mb_os_malloc( strlen( s ) + 1 );
  if( !newp )
  {
    return NULL;
  }
  else
  {
    strcpy( newp,s );
    return newp;
  }
}

static INT32 net_read(INT32 fd, CHAR *buf, size_t len)
{
  INT32 err;
  while ( 1 )
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "trying to receive %d bytes on socket %p.\r\n", len, fd);
    INT32 c = m2mb_socket_bsd_recv(fd, buf, len, 0);
    if ( c == -1 )
    {

      err = m2mb_socket_errno();
      if ( err != M2MB_SOCKET_BSD_EINTR  && err != M2MB_SOCKET_TRY_AGAIN )
      {
        return -1;
      }
    }
    else
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "Received (%d): <%.*s>\r\n", c, c, buf);
      return c;
    }
  }
}
#ifdef M2M_M2MB_SSL_H
static INT32 net_ssl_read(M2MB_SSL_CONNECTION_HANDLE sslConnHndl, CHAR *buf, size_t len)
{
  INT32 err;
  while ( 1 )
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "trying to receive %d bytes on socket %p.\r\n", len, sslConnHndl);
    INT32 c = m2mb_ssl_read(sslConnHndl, buf, len);
    if ( c == -1 )
    {

      err = m2mb_socket_errno();
      if ( err != M2MB_SOCKET_BSD_EINTR  && err != M2MB_SOCKET_TRY_AGAIN )
      {
        return -1;
      }
    }
    else
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "Received (%d): <%.*s>\r\n", c, c, buf);
      return c;
    }
  }
}
#endif

static INT32 net_write(INT32 fd, const CHAR *buf, size_t len)
{
  INT32 done = 0;
  INT32 err;
  CHAR *b = (CHAR*)buf;
  while ( len > 0 )
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "sending %d bytes <%s> on socket %p\r\n", len, b, fd);
    INT32 c = m2mb_socket_bsd_send( fd, b, len, 0);
    if ( c == -1 )
    {
      err = m2mb_socket_errno();
      if ( err != M2MB_SOCKET_BSD_EINTR  && err != M2MB_SOCKET_TRY_AGAIN )
      {
        return -1;
      }
    }
    else if ( c == 0 )
    {
      return done;
    }
    else
    {
      b += c;
      done += c;
      len -= c;
    }
  }
  return done;
}

#ifdef M2M_M2MB_SSL_H
static INT32 net_ssl_write(M2MB_SSL_CONNECTION_HANDLE sslConnHndl, const CHAR *buf, size_t len)
{
  INT32 done = 0;
  INT32 err;
  CHAR *b = (CHAR*)buf;
  while ( len > 0 )
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "sending %d bytes <%s> on socket %p\r\n", len, b, sslConnHndl);
    INT32 c = m2mb_ssl_write( sslConnHndl, b, len);
    if ( c == -1 )
    {
      err = m2mb_socket_errno();
      if ( err != M2MB_SOCKET_BSD_EINTR  && err != M2MB_SOCKET_TRY_AGAIN )
      {
        return -1;
      }
    }
    else if ( c == 0 )
    {
      return done;
    }
    else
    {
      b += c;
      done += c;
      len -= c;
    }
  }
  return done;
}
#endif


#define net_close m2mb_socket_bsd_close

#ifdef M2M_M2MB_SSL_H

static INT32 net_ssl_close(M2MB_SOCKET_BSD_SOCKET s, M2MB_SSL_CONNECTION_HANDLE sslConnHndl)
{
  INT32 ret;

  ret = m2mb_ssl_shutdown(sslConnHndl);
  if (ret < 0)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "fail to close ssl\r\n");
  }
  else
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "SSL socket shutdown\r\n");
  }

  ret = m2mb_socket_bsd_close(s);
  if(ret < 0 )
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "failed to close socket\r\n");
  }
  else
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "Socket closed\r\n");
  }
  return ret;
}
#endif

#endif


#if defined(NEED_MEMCCPY)
/*
 * VAX C does not supply a memccpy routine so I provide my own
 */
void *memccpy(void *dest, const void *src, int c, size_t n)
{
  int i=0;
  const unsigned char *ip = (const unsigned char *)src;
  unsigned char *op = (unsigned char *)dest;

  while (i < n)
  {
    if ((*op++ = *ip++) == c)
    {
      break;
    }

    i++;
  }
  if (i == n)
  {
    return NULL;
  }
  return op;
}
#endif
#if defined(NEED_STRDUP)
/*
 * strdup - return a malloc'ed copy of a string
 */
char *strdup(const char *src)
{
  int l = strlen(src) + 1;
  char *dst = malloc(l);
  if (dst)
    strcpy(dst,src);
  return dst;
}
#endif

/*
 * socket_wait - wait for socket to receive or flush data
 *
 * return 1 if no user callback, otherwise, return value returned by
 * user callback
 */
static INT32 socket_wait(AZX_FTP_NET_BUF_T *ctl)
{
  fd_set fd, *rfd = NULL, *wfd = NULL;
  struct timeval tv;
  INT32 rv = 0;
  if ((ctl->dir == AZX_FTP_CONTROL) || (ctl->idlecb == NULL))
  {
    return 1;
  }

  if (ctl->dir == AZX_FTP_WRITE)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"Setting wfd to fd %p\r\n", &fd);
    wfd = &fd;
  }
  else
  {
    rfd = &fd;
  }

  FD_ZERO(&fd);
  do
  {
    FD_SET(ctl->handle, &fd);
    tv = ctl->idletime;

    rv = select(ctl->handle+1, rfd, wfd, NULL, &tv);
    if (rv == -1)
    {
      rv = 0;
      strncpy(ctl->ctrl->response, strerror(errno),
          AZX_FTP_RESPONSE_BUFSIZE);
      //sizeof(ctl->ctrl->response));

      break;
    }
    else if (rv > 0)
    {
      rv = 1;
      break;
    }
  }
  while ((rv = ctl->idlecb(ctl, ctl->xfered, ctl->idlearg) > 0 ));
  if(rv == -1)
  {
    ctl->ctrl->abort = 1;
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"CB request to close...\r\n");
    rv =0;
  }
  return rv;
}

/*
 * read a line of text
 *
 * return -1 on error or bytecount
 */
static INT32 readline(CHAR *buf, INT32 max, AZX_FTP_NET_BUF_T *ctl)
{
  INT32 x,retval = 0;
  CHAR *end,*bp=buf;
  INT32 eof = 0;

  if ((ctl->dir != AZX_FTP_CONTROL) && (ctl->dir != AZX_FTP_READ))
  {
    return -1;
  }
  if (max == 0)
  {
    return 0;
  }

  do
  {
    if (ctl->cavail > 0)
    {
      x = (max >= ctl->cavail) ? ctl->cavail : max-1;
      end = (CHAR*) memccpy((void*)bp, (const void*)ctl->cget,'\n',x);
      if (end != NULL)
      {
        x = end - bp;
      }

      retval += x;
      bp += x;
      *bp = '\0';
      max -= x;
      ctl->cget += x;
      ctl->cavail -= x;
      if (end != NULL)
      {
        bp -= 2;
        if (strcmp(bp,"\r\n") == 0)
        {
          *bp++ = '\n';
          *bp++ = '\0';
          --retval;
        }
        break;
      }
    }
    if (max == 1)
    {
      *buf = '\0';
      break;
    }
    if (ctl->cput == ctl->cget)
    {
      ctl->cput = ctl->cget = ctl->buf;
      ctl->cavail = 0;
      ctl->cleft = AZX_FTP_BUFSIZE;
    }
    if (eof)
    {
      if (retval == 0)
      {
        retval = -1;
      }
      break;
    }
    if (!socket_wait(ctl))
    {
      return retval;
    }
#ifdef M2M_M2MB_SSL_H
    if((ftp_opts.ssl == 1) && (ctl->AUTHdone == TRUE))
    {
      if ((x = net_ssl_read(ctl->sslHandle,ctl->cput,ctl->cleft)) == -1)
      {

        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "read\r\n");
        retval = -1;
        break;
      }
    }
    else
    {
      if ((x = net_read(ctl->handle,ctl->cput,ctl->cleft)) == -1)
      {

        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "read\r\n");
        retval = -1;
        break;
      }
    }
#else
    if ((x = net_read(ctl->handle,ctl->cput,ctl->cleft)) == -1)
     {
       AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "read\r\n");
       retval = -1;
       break;
     }
#endif
    if (x == 0)
    {
      eof = 1;
    }
    ctl->cleft -= x;
    ctl->cavail += x;
    ctl->cput += x;

  }
  while (1);

  return retval;
}

/*
 * write lines of text
 *
 * return -1 on error or bytecount
 */
static INT32 writeline(const CHAR *buf, INT32 len, AZX_FTP_NET_BUF_T *nData)
{
  INT32 x, nb=0, w;
  const CHAR *ubp = buf;
  CHAR *nbp;
  CHAR lc=0;

  if (nData->dir != AZX_FTP_WRITE)
    return -1;
  nbp = nData->buf;
  for (x=0; x < len; x++)
  {
    if ((*ubp == '\n') && (lc != '\r'))
    {
      if (nb == AZX_FTP_BUFSIZE)
      {
        if (!socket_wait(nData))
          return x;
#ifdef M2M_M2MB_SSL_H
        if((ftp_opts.ssl == 1) && (nData->AUTHdone == TRUE))
        {
          w = net_ssl_write(nData->sslHandle, nbp, AZX_FTP_BUFSIZE);
        }
        else
        {
        w = net_write(nData->handle, nbp, AZX_FTP_BUFSIZE);
        }
#else
        w = net_write(nData->handle, nbp, AZX_FTP_BUFSIZE);
#endif
        if (w != AZX_FTP_BUFSIZE)
        {
          AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_INFO, "net_write(1) returned %d, errno = %d\r\n", w, errno);
          return(-1);
        }
        nb = 0;
      }
      nbp[nb++] = '\r';
    }
    if (nb == AZX_FTP_BUFSIZE)
    {
      if (!socket_wait(nData))
      {
        return x;
      }
#ifdef M2M_M2MB_SSL_H
      if((ftp_opts.ssl == 1) && (nData->AUTHdone == TRUE))
      {
         w = net_ssl_write(nData->sslHandle, nbp, AZX_FTP_BUFSIZE);
      }
      else
      {
      w = net_write(nData->handle, nbp, AZX_FTP_BUFSIZE);
      }
#else
      w = net_write(nData->handle, nbp, AZX_FTP_BUFSIZE);
#endif
      if (w != AZX_FTP_BUFSIZE)
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_INFO,"net_write(2) returned %d, errno = %d\r\n", w, errno);
        return(-1);
      }
      nb = 0;
    }
    nbp[nb++] = lc = *ubp++;
  }
  if (nb)
  {
    if (!socket_wait(nData))
    {
      return x;
    }
#ifdef M2M_M2MB_SSL_H
    if((ftp_opts.ssl == 1) && (nData->AUTHdone == TRUE))
    {
      w = net_ssl_write(nData->sslHandle, nbp, AZX_FTP_BUFSIZE);
    }
    else
    {
    w = net_write(nData->handle, nbp, nb);
    }
#else
    w = net_write(nData->handle, nbp, nb);
#endif
    if (w != nb)
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_INFO,"net_write(3) returned %d, errno = %d\r\n", w, errno);
      return(-1);
    }
  }
  return len;
}

/*
 * read a response from the server
 *
 * return 0 if first char doesn't match
 * return 1 if first char matches
 */
static INT32 readresp(CHAR c, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR match[5];
  if (readline(nControl->response,AZX_FTP_RESPONSE_BUFSIZE,nControl) == -1)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"Control socket read failed\r\n");
    return 0;
  }

  AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_INFO, "%s\r\n", nControl->response);


  if (nControl->response[3] == '-')
  {
    strncpy(match,nControl->response,3);
    match[3] = ' ';
    match[4] = '\0';
    do
    {
      if (readline(nControl->response,AZX_FTP_RESPONSE_BUFSIZE,nControl) == -1)
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"Control socket read failed\r\n");

        return 0;
      }

      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_INFO,"%s\r\n",nControl->response);

    }
    while (strncmp(nControl->response,match,4));
  }
  if (nControl->response[0] == c)
  {
    return 1;
  }
  return 0;
}


//FabioPi
static M2MB_RESULT_E FtpAssociateCid( const CHAR *host, INT8 cid, struct M2MB_SOCKET_BSD_SOCKADDR_IN *sin, INT32 *sControl )
{
  CHAR *lhost;
  CHAR *pnum;
  CHAR tmpbuf[AZX_FTP_TMP_BUFSIZ];
  struct M2MB_SOCKET_BSD_HOSTENT *phe;
  struct M2MB_SOCKET_BSD_HOSTENT he;
  INT32 herr;
  CHAR addr[24];
  M2MB_RESULT_E ret = M2MB_RESULT_SUCCESS;

  memset( sin,0,sizeof( struct M2MB_SOCKET_BSD_SOCKADDR_IN ) );

  sin->sin_family = M2MB_SOCKET_BSD_AF_INET;

  lhost = azx_ftp_strdup( host );

  if(!lhost)
  {
    ret = M2MB_RESULT_FAIL;
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR, "azx_ftp_strdup returned null!\r\n" );
  }
  else
  {
    pnum = (CHAR *) strchr( ( const char * )lhost,':' );
    if( pnum == NULL )
    {
      pnum = ( CHAR * )AZX_FTP_DEFAULT_PNUM;
    }
    else
    {
      /*Advance pnum pointer by one and put a EOS where : was*/
      *pnum++ = '\0';
    }

    if( pnum[strspn( pnum, "0123456789" )] == 0 ) //check if string is composed of base10 digits only
    {
      sin->sin_port = m2mb_socket_bsd_htons( (UINT16) (atoi( pnum ) & 0xFFFF ));
    }
    else
    {
      sin->sin_port = m2mb_socket_bsd_htons( AZX_FTP_DEFAULT_PORTNUM );
    }

    if( ( m2mb_socket_bsd_get_host_by_name_2_r_cid( ( const CHAR * )lhost, M2MB_SOCKET_BSD_AF_INET,
        &he, tmpbuf, AZX_FTP_TMP_BUFSIZ, &phe, &herr, cid )  != 0 ) || ( phe == NULL ) )
    {
      m2mb_os_free( lhost );
      ret = M2MB_RESULT_FAIL;
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR, "Get Host By Name %s\r\n", hstrerror( herr ) );
    }

    if( ret == M2MB_RESULT_SUCCESS )
    {
      memcpy( ( CHAR * )( &( sin->sin_addr ) ), phe->h_addr_list[0], phe->h_length );

      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_INFO, "Retrieved address: %s\r\n", ( ( INT8 * )m2mb_socket_bsd_inet_ntop( M2MB_SOCKET_BSD_AF_INET,
          ( void * ) &( sin->sin_addr.s_addr ),
          addr, sizeof( addr ) ) ) );
      m2mb_os_free( lhost );

      *sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (*sControl == -1)
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"socket\r\n");
        return M2MB_RESULT_FAIL;
      }

      if ( m2mb_socket_set_cid( *sControl, ftp_opts.cid ) != 0 )
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"socket cid\r\n");
        net_close(*sControl);
        return M2MB_RESULT_FAIL;
      }
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_INFO, "socket %p ok\r\n", *sControl);
    }
  }
  return ret;
}


/*
 * FtpSendCmd - send a command and wait for expected response
 *
 * return 1 if proper response received, 0 otherwise
 */
static INT32 FtpSendCmd(const CHAR *cmd, CHAR expresp, AZX_FTP_NET_BUF_T *nControl)
{
  if (nControl->dir != AZX_FTP_CONTROL)
  {
    return 0;
  }
  if(nControl->abort)
  {
    return 0;
  }
  AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_INFO,"%s\r\n",cmd);

  memset(nControl->buf,0,AZX_FTP_BUFSIZE);
  AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "FtpSendCmd: %s. expresp: %c\r\n", cmd, expresp);

  if ((strlen(cmd) + 3) > AZX_FTP_BUFSIZE)
  {
    return 0;
  }

  sprintf(nControl->buf,"%s\r\n",cmd);
#ifdef M2M_M2MB_SSL_H
  if((ftp_opts.ssl == 1) && (nControl->AUTHdone == TRUE))
  {
    if (net_ssl_write(nControl->sslHandle, nControl->buf, strlen(nControl->buf)) <= 0)
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"write\r\n");
      return 0;
    }
    return readresp(expresp, nControl);
  }
  else
  {
  if (net_write(nControl->handle, nControl->buf, strlen(nControl->buf)) <= 0)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"write\r\n");
    return 0;
  }
  return readresp(expresp, nControl);
  }
#else
  if (net_write(nControl->handle, nControl->buf, strlen(nControl->buf)) <= 0)
   {
     AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"write\r\n");
     return 0;
   }
   return readresp(expresp, nControl);
#endif
}

#ifdef M2M_M2MB_SSL_H
static INT32 FtpStartAuth(CHAR expresp, AZX_FTP_NET_BUF_T *nControl)
{
  int ret;
  ret = FtpSendCmd("AUTH TLS", expresp, nControl);
  if(ret == 1)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_INFO,"Start with SSL handshake\r\n");
    //nControl->AUTHdone = TRUE;
  }
  else
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"FtpSendCmd\r\n");
    return 0;
  }

  return 1;
}
#endif
/*
 * FtpOpenPort - set up data connection
 *
 * return 1 if successful, 0 otherwise
 */
static INT32 FtpOpenPort(AZX_FTP_NET_BUF_T *nControl, AZX_FTP_NET_BUF_T **nData, INT32 mode, INT32 dir)
{
  INT32 sData;
  union {
    struct sockaddr sa;
    struct sockaddr_in in;
  } sin;
#if !defined(AZX)  //unsupported
  struct linger lng = { 0, 0 };
#endif
#if defined (AZX)
  INT32 l;
#else
  unsigned int l;
#endif
  INT32 on=1;
  AZX_FTP_NET_BUF_T *ctrl = NULL;
  CHAR *cp;

  CHAR buf[AZX_FTP_TMP_BUFSIZ];

  if (nControl->dir != AZX_FTP_CONTROL)
  {
    return -1;
  }

  if ((dir != AZX_FTP_READ) && (dir != AZX_FTP_WRITE))
  {
    sprintf(nControl->response, "Invalid direction %d\n", dir);
    return -1;
  }
  if ((mode != AZX_FTP_ASCII) && (mode != AZX_FTP_IMAGE))
  {
    sprintf(nControl->response, "Invalid mode %c\n", mode);
    return -1;
  }
  l = sizeof(sin);
  if (nControl->cmode == AZX_FTP_PASSIVE)
  {
    int port = 0;
    char sep;

    char search_template[20] = {0};
    memset(&sin, 0, l);
    sin.in.sin_family = AF_INET;

    if (m2mb_socket_bsd_get_peer_name(nControl->handle, &sin.sa, &l) < 0)
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"getpeername\r\n");
      return -1;
    }
    /* rfc2428 */
    if (!FtpSendCmd("EPSV",'2',nControl))
    {
      return -1;
    }
    cp = strchr(nControl->response,'(');
    if (cp == NULL)
    {
      return -1;
    }
    cp++;
    sep = *cp;
    sprintf(search_template, "%c%c%c%%d%c", sep,sep,sep,sep);

    if(1 != sscanf(cp, search_template, &port))
    {
      return -1;
    }
    else
    {
      sin.in.sin_port = m2mb_socket_bsd_htons((UINT16)(port & 0xffff));
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"port (net order): %x\r\n", sin.in.sin_port);
    }
  }
  else
  {
    if (getsockname(nControl->handle, &sin.sa, &l) < 0)
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"getsockname\r\n");
      return -1;
    }
  }
  sData = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sData == -1)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"socket\r\n");
    return -1;
  }

#if defined(AZX)
  if ( m2mb_socket_set_cid( sData, ftp_opts.cid ) < 0 )
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"socket %p on cid %d\r\n",  sData, ftp_opts.cid);
    net_close(sData);
    return -1;
  }
#endif

  if (setsockopt(sData,SOL_SOCKET,SO_REUSEADDR,
      SETSOCKOPT_OPTVAL_TYPE &on,sizeof(on)) == -1)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"setsockopt\r\n");
    net_close(sData);
    return -1;
  }

#if !defined(AZX)  //unsupported
  if (setsockopt(sData,SOL_SOCKET,SO_LINGER,
      SETSOCKOPT_OPTVAL_TYPE &lng,sizeof(lng)) == -1)
  {
    if (ftplib_debug)
      perror("setsockopt");
    net_close(sData);
    return -1;
  }
#endif

  if (nControl->cmode == AZX_FTP_PASSIVE)
  {
    if (connect(sData, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"connect\r\n");
      net_close(sData);
      return -1;
    }
  }
  else
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"Active mode not supported! \r\n");
    return 0;

    sin.in.sin_port = 0;
    if (bind(sData, &sin.sa, sizeof(sin)) == -1)
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"bind\r\n");
      net_close(sData);
      return -1;
    }
    if (listen(sData, 1) < 0)
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"listen\r\n");
      net_close(sData);
      return -1;
    }
    if (getsockname(sData, &sin.sa, &l) < 0)
    {
      return -1;
    }

    sprintf(buf, "PORT %d,%d,%d,%d,%d,%d",
        (unsigned char) sin.sa.sa_data[2],
        (unsigned char) sin.sa.sa_data[3],
        (unsigned char) sin.sa.sa_data[4],
        (unsigned char) sin.sa.sa_data[5],
        (unsigned char) sin.sa.sa_data[0],
        (unsigned char) sin.sa.sa_data[1]);
    if (!FtpSendCmd(buf,'2',nControl))
    {
      net_close(sData);
      return -1;
    }
  }
  ctrl = (AZX_FTP_NET_BUF_T *) calloc(1,sizeof(AZX_FTP_NET_BUF_T));
  if (ctrl == NULL)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"calloc\r\n");
    net_close(sData);
    return -1;
  }
  if ((mode == 'A') && ((ctrl->buf = (CHAR*)malloc(AZX_FTP_BUFSIZE)) == NULL))
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"calloc\r\n");
    net_close(sData);
    free(ctrl);
    return -1;
  }
  ctrl->handle = sData;
  ctrl->dir = dir;
  ctrl->idletime = nControl->idletime;
  ctrl->idlearg = nControl->idlearg;
  ctrl->xfered = 0;
  ctrl->xfered1 = 0;
  ctrl->cbbytes = nControl->cbbytes;
  ctrl->ctrl = nControl;
  if (ctrl->idletime.tv_sec || ctrl->idletime.tv_usec || ctrl->cbbytes)
  {
    ctrl->idlecb = nControl->idlecb;
  }
  else
  {
    ctrl->idlecb = NULL;
  }

  nControl->data = ctrl;
  *nData = ctrl;
  return 1;
}


/*
 * FtpAcceptConnection - accept connection from server
 *
 * return 1 if successful, 0 otherwise
 */
static INT32 FtpAcceptConnection(AZX_FTP_NET_BUF_T *nData, AZX_FTP_NET_BUF_T *nControl)
{
  INT32 sData;
  struct sockaddr addr;
#if defined (AZX)
  INT32 l;
#else
  unsigned int l;
#endif
  INT32 i;
  struct timeval tv;
  fd_set mask;
  INT32 rv = 0;

  FD_ZERO(&mask);
  FD_SET(nControl->handle, &mask);
  FD_SET(nData->handle, &mask);
  tv.tv_usec = 0;
  tv.tv_sec = ACCEPT_TIMEOUT;
  i = nControl->handle;
  if (i < nData->handle)
    i = nData->handle;
  i = select(i+1, &mask, NULL, NULL, &tv);
  if (i == -1)
  {
    strncpy(nControl->response, strerror(errno),
        //sizeof(nControl->response));
        AZX_FTP_RESPONSE_BUFSIZE);
    net_close(nData->handle);
    nData->handle = 0;
    rv = 0;
  }
  else if (i == 0)
  {
    strcpy(nControl->response, "timed out waiting for connection");
    net_close(nData->handle);
    nData->handle = 0;
    rv = 0;
  }
  else
  {
    if (FD_ISSET(nData->handle, &mask))
    {
      l = sizeof(addr);
      sData = accept(nData->handle, &addr, &l);
      i = errno;
      net_close(nData->handle);
      if (sData > 0)
      {
        rv = 1;
        nData->handle = sData;
      }
      else
      {
        strncpy(nControl->response, strerror(i),
            AZX_FTP_RESPONSE_BUFSIZE);
        //sizeof(nControl->response));
        nData->handle = 0;
        rv = 0;
      }
    }
    else if (FD_ISSET(nControl->handle, &mask))
    {
      net_close(nData->handle);
      nData->handle = 0;
      readresp('2', nControl);
      rv = 0;
    }
  }
  return rv;
}



/*
 * FtpXferFile - issue a command and transfer data to a local file
 *
 * return 1 if successful, 0 otherwise
 */
static INT32 FtpXferFile(AZX_FTP_FILE_INFO_T *localfile, AZX_FTP_FILE_INFO_T *remfileinfo,
    AZX_FTP_NET_BUF_T *nControl, INT32 typ, INT32 mode)
{
  INT32 l,c;
  CHAR *dbuf;
  FILE *local = NULL;
  AZX_FTP_NET_BUF_T *nData;
  INT32 rv=1;
  UINT32 byteTransf = 0;
  INT32 retry = 0; /*counter for REST cmd*/

  if (localfile != NULL && localfile->path != NULL)
  {
    CHAR ac[4];
    memset( ac, 0, sizeof(ac) );
    if (typ == AZX_FTP_FILE_WRITE)
    {
      ac[0] = 'r';
    }
    else
    {
      ac[0] = 'w';
    }
    if (mode == AZX_FTP_IMAGE)
    {
      ac[1] = 'b';
    }

    local = fopen(localfile->path, ac);

    if (local == NULL)
    {
      strncpy(nControl->response, strerror(errno),
          AZX_FTP_RESPONSE_BUFSIZE);
      //sizeof(nControl->response));
      return 0;
    }
   // AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"local file opened.\r\n");
  }

  if(localfile == NULL)
  {
    AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_DEBUG,"no localfile provided\r\n");
  }
#if defined(AZX) //STDIN unsupported on m2mb
  if (local == NULL)
  {
    if (typ == AZX_FTP_FILE_WRITE)
    {
      AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_ERROR,"stdin unsupported\r\n");
      return 0;
    }
    else
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"using stdout\r\n");
      local = (M2MB_FILE_T*) stdout;
    }
  }
#else
  if (local == NULL)
    local = (typ == AZX_FTP_FILE_WRITE) ? stdin : stdout;
#endif

  if (!azx_ftp_access(remfileinfo->path, typ, mode, nControl, &nData, 0))
  {
    if (localfile && localfile->path)
    {
      fclose(local);
      if ( typ == AZX_FTP_FILE_READ )
      {
        unlink(localfile->path);
      }
    }
    return 0;
  }
  dbuf = (CHAR *) malloc(AZX_FTP_BUFSIZE);
  if (typ == AZX_FTP_FILE_WRITE)
  {
    while ((l = fread(dbuf, 1, AZX_FTP_BUFSIZE, local)) > 0)
    {
      c = azx_ftp_write(dbuf, l, nData);
      if (c < l)
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "short write: passed %d, wrote %d\r\n", l, c);
        rv = 0;
        break;
      }
    }
  }
  else  //READ
  {
    do
    {
      while ((l = azx_ftp_read(dbuf, AZX_FTP_BUFSIZE, nData)) > 0)
      {
        byteTransf += l;
        if (fwrite(dbuf, 1, l, local) == 0)
        {
          AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"localfile write\r\n");
          rv = 0;
          break;
        }
      }
      if (nControl->abort == 1) //aborted by CB
      {
        rv = 0;
        break;
      }

      if( remfileinfo->fileSize )
      {
        retry++;
        if( remfileinfo->fileSize > byteTransf )
        {
          AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"Restart transfer from offset %d\r\n", byteTransf );

          //m2mb_fs_fflush( local );
          azx_ftp_close( nData );
          nData = NULL;
          nControl->data = NULL;

          if( !azx_ftp_access(remfileinfo->path, typ, mode, nControl, &nData, byteTransf ) )
          {
            AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR, "FtpAccess error\r\n" );
            rv = 0;
            break;
          }
        }
      }
    }
    while( ( remfileinfo->fileSize ) && ( remfileinfo->fileSize > byteTransf ) && ( retry < AZX_FTP_MAX_REST ) && nControl->abort != 1 );

  }

  free(dbuf);
  fflush(local);
  if (localfile && localfile->path != NULL)
  {
    fclose(local);
    if(nControl->abort == 1)
    {
      //DELETE local file
      unlink(localfile->path);
    }
  }
  azx_ftp_close(nData);
  if( retry >= AZX_FTP_MAX_REST )
  {
    return 0;
  }
  return rv;
}


/*
 * FtpXferBuffer - issue a command and transfer data to a buffer
 *
 * return transferred bytes if successful, 0 otherwise
 */
static INT32 FtpXferBuffer( AZX_FTP_BUFFER_T *localinfo, AZX_FTP_FILE_INFO_T *remfileinfo,
    AZX_FTP_NET_BUF_T *nControl, INT32 typ, INT32 mode)
{
  AZX_FTP_NET_BUF_T *nData;
  INT32 l;
  UINT32 byteTransf = 0;
  INT32 retry = 0; /*counter for REST cmd*/


  if( ( typ != AZX_FTP_FILE_READ ) && ( typ != AZX_FTP_FILE_WRITE ) && ( typ != AZX_FTP_DIR ) && ( typ != AZX_FTP_DIR_VERBOSE ))
  {
    AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_ERROR, "type error\r\n" );
    return 0;
  }

  if( ( mode != AZX_FTP_BINARY ) && ( mode != AZX_FTP_TEXT ) )
  {
    AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_ERROR, "mode error\r\n" );
    return 0;
  }

  if( !azx_ftp_access(remfileinfo->path, typ, mode, nControl, &nData, 0))
  {
    AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_ERROR,  "FtpAccess error: %s\r\n", azx_ftp_lastResponse(nControl) );
    return 0;
  }


  if( typ == AZX_FTP_FILE_READ || typ == AZX_FTP_DIR_VERBOSE || typ == AZX_FTP_DIR)
  {
    do
    {
      if (localinfo->buf_cb )
      {
        localinfo->buf_cb(NULL, 0, DATA_CB_START); //notify the user of the data start
        while( ( l = azx_ftp_read( localinfo->buffer, localinfo->bufferSize, nData ) ) > 0 )
        {
          AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_DEBUG, "Received: %d\r\n",l );
          byteTransf += l;
          localinfo->buf_cb(localinfo->buffer, l, DATA_CB_DATA);
        }
      }
      else
      {
        while( ( l = azx_ftp_read( localinfo->buffer + byteTransf, localinfo->bufferSize - byteTransf, nData ) ) > 0 )
        {
          AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_DEBUG, "Received: %d\r\n",l );
          byteTransf += l;
        }
      }


      if (l < 0) //aborted by CB
      {
        break;
      }


      if( remfileinfo->fileSize )
      {
        retry++;

        if( remfileinfo->fileSize > byteTransf )
        {
          AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_DEBUG, "Restart transfer from offset %d\r\n", byteTransf );

          azx_ftp_close( nData );
          nData = NULL;
          nControl->data = NULL;

          if( !azx_ftp_access(remfileinfo->path, typ, mode, nControl, &nData, byteTransf ) )
          {
            AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_ERROR, "azx_ftp_access error\r\n" );
            return 0;
          }
        }
      }
    }
    while( ( remfileinfo->fileSize ) && ( remfileinfo->fileSize > byteTransf ) && ( retry < AZX_FTP_MAX_REST ) );
  }
  else /*To Be developed*/
  {
    do
    {
      l = azx_ftp_write( localinfo->buffer, localinfo->bufferSize, nData );
      if( (UINT32) l < localinfo->bufferSize )
      {
        AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_ERROR,  "azx_ftp_write error\r\n" );
        azx_ftp_close( nData );
        return 0;
      }

    }
    while( localinfo->bufferSize );
  }

  azx_ftp_close( nData );

  if( retry >= AZX_FTP_MAX_REST )
  {
    return 0;
  }

  if (localinfo->buf_cb )
  {
    localinfo->buf_cb(NULL, byteTransf, DATA_CB_END); //notify the user of the data completion, passing the number of bytes received
  }
  //AZX_FTP_DEBUG( AZX_FTP_DEBUG_HOOK_INFO,  "byteTransf: %d\r\n", byteTransf );
  return byteTransf; //returns transferred bytes
}


/*
 * azx_ftp_init
 */
AZX_FTP_GLOBALDEF  INT32 azx_ftp_init(const AZX_FTP_OPTIONS_T *opt)
{
#if defined(_WIN32)
  WORD wVersionRequested;
  WSADATA wsadata;
  int err;
  wVersionRequested = MAKEWORD(1,1);
  if ((err = WSAStartup(wVersionRequested,&wsadata)) != 0)
    m2m_fprintf(stderr,"Network failed to start: %d\n",err);
#endif

  memset(&ftp_opts,0,sizeof(AZX_FTP_OPTIONS_T));

  ftp_opts.cbFunc = opt->cbFunc;
  ftp_opts.level = opt->level;
  ftp_opts.cid = opt->cid;
  ftp_opts.ssl = opt->ssl;
#ifdef TLS_REUSE
  ftp_opts.sslReuse = opt -> sslReuse;
#endif
  ftp_opts.sslConfigH = opt->sslConfigH;
  ftp_opts.sslCtxtH = opt->sslCtxtH;
  return 1;
}

/*
 * FtpLogin - log in to remote server
 *
 * return 1 if logged in, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_login(const CHAR *user, const CHAR *pass, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR tempbuf[64];

  if (((strlen(user) + 7) > sizeof(tempbuf)) ||
      ((strlen(pass) + 7) > sizeof(tempbuf)))
    return 0;
  sprintf(tempbuf,"USER %s",user);
  if (!FtpSendCmd(tempbuf,'3',nControl))
  {
    if (nControl->response[0] == '2')
    {
      return 1;
    }
    return 0;
  }
  sprintf(tempbuf,"PASS %s",pass);
  return FtpSendCmd(tempbuf,'2',nControl);
}

/*
 * FtpSSLCfg - set SSL parameters for data channel
 *
 * return 1 if set is successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_sslCfg(CHAR prot, CHAR buffSise, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR tempbuf[64];

  sprintf(tempbuf,"PBSZ %c",buffSise);
  if (!FtpSendCmd(tempbuf,'2',nControl))
  {
    return 0;
  }
  sprintf(tempbuf,"PROT %c",prot);
  return FtpSendCmd(tempbuf,'2',nControl);
}

/*
 * FtpAccess - return a handle for a data stream
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_access(const CHAR *path, INT32 typ, INT32 mode, AZX_FTP_NET_BUF_T *nControl,
    AZX_FTP_NET_BUF_T **nData, INT32 offset)
{
  CHAR buf[AZX_FTP_TMP_BUFSIZ];
  INT32 dir;
  if ((path == NULL) &&
      ((typ == AZX_FTP_FILE_WRITE) || (typ == AZX_FTP_FILE_READ)))
  {
    sprintf(nControl->response,
        "Missing path argument for file transfer\n");
    return 0;
  }

  sprintf(buf, "TYPE %c", mode);
  AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"sending <%s>...\r\n", buf);

  if (!FtpSendCmd(buf, '2', nControl))
  {
    return 0;
  }

  switch (typ)
  {
  case AZX_FTP_DIR:
    strcpy(buf,"NLST");
    dir = AZX_FTP_READ;
    break;
  case AZX_FTP_DIR_VERBOSE:
    strcpy(buf,"LIST");
    dir = AZX_FTP_READ;
    break;
  case AZX_FTP_FILE_READ:
    strcpy(buf,"RETR");
    dir = AZX_FTP_READ;
    break;
  case AZX_FTP_FILE_WRITE:
    strcpy(buf,"STOR");
    dir = AZX_FTP_WRITE;
    break;
  default:
    sprintf(nControl->response, "Invalid open type %d\n", typ);
    return 0;
  }
  if (path != NULL)
  {
    INT32 i = strlen(buf);
    buf[i++] = ' ';
    if ((strlen(path) + i + 1) >= sizeof(buf))
      return 0;
    strcpy(&buf[i],path);
  }
  AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"Open data port...\r\n");
  if (FtpOpenPort(nControl, nData, mode, dir) == -1)
  {
    return 0;
  }

  if( ( typ == AZX_FTP_FILE_READ ) && ( offset != 0 ) )
  {
    CHAR bufRest[AZX_FTP_REST_CMD_SIZE];

    memset( bufRest, 0, AZX_FTP_REST_CMD_SIZE );
    sprintf( bufRest,"REST %d", offset );

    if( !FtpSendCmd( bufRest, '3', nControl ) )
    {
      azx_ftp_close( *nData );
      *nData = NULL;
      return 0;
    }

  }


  if (!FtpSendCmd(buf, '1', nControl))
  {
    azx_ftp_close(*nData);
    *nData = NULL;
    return 0;
  }
#ifdef M2M_M2MB_SSL_H
 M2MB_SSL_CONNECTION_HANDLE sslData = 0;
 INT32 sslRes;
 if(ftp_opts.ssl)
 {
#ifdef TLS_REUSE
   //AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"ftp_opts.sslReuse: %d \r\n",ftp_opts.sslReuse);
   if(ftp_opts.sslReuse)
   {
     sslData = m2mb_ssl_resume_secure_socket( ftp_opts.sslConfigH, ftp_opts.sslCtxtH, (*nData)->handle, &sslRes);
     if(sslData == 0)
     {
       AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"m2mb_ssl_resume_secure_socket FAILED error %d \r\n",sslRes);
       azx_ftp_close( *nData );
       *nData = NULL;
       return 0;
     }
     else
     {
       AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"In reuse - sslData: %d \r\n",sslData);
       if (ftp_opts.sslSessionH != NULL)
       {
         sslRes = m2mb_ssl_set_session(sslData, ftp_opts.sslSessionH);
         if(sslRes < 0)
         {
           AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"m2mb_ssl_set_session FAILED error %d \r\n",sslRes);
           azx_ftp_close( *nData );
           *nData = NULL;
           return 0;
         }
         else
         {
           sslRes = m2mb_ssl_connect( sslData );
           if( sslRes != 0 )
           {
             AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"m2mb_ssl_connect FAILED error %d. Please verify module clock with AT+CCLK? command \r\n.",sslRes );
             azx_ftp_close( *nData );
             *nData = NULL;
             return 0;
           }
         }
        }
     }
   }
   else
   {
#endif
   sslData = m2mb_ssl_secure_socket( ftp_opts.sslConfigH,ftp_opts.sslCtxtH, (*nData)->handle, &sslRes );
   if( sslData == 0 )
   {
     //AZX_LOG_ERROR("m2mb_ssl_secure_socket FAILED error %d \r\n",sslRes );
     AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"m2mb_ssl_secure_socket FAILED error %d \r\n",sslRes);
     azx_ftp_close( *nData );
     *nData = NULL;
     return 0;
   }
   else
   {
     sslRes = m2mb_ssl_connect( sslData );
     if( sslRes != 0 )
     {
       AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"m2mb_ssl_connect FAILED error %d. Please verify module clock with AT+CCLK? command \r\n.",sslRes );
       azx_ftp_close( *nData );
       *nData = NULL;
       return 0;
     }
   }
#ifdef TLS_REUSE
   }
#endif
 }
 (*nData)->sslHandle = sslData;
 (*nData)->AUTHdone = TRUE;
#endif

  if (nControl->cmode == AZX_FTP_PORT)
  {
    if (!FtpAcceptConnection(*nData,nControl))
    {
      azx_ftp_close(*nData);
      *nData = NULL;
      nControl->data = NULL;
      return 0;
    }
  }
  return 1;
}



/*
 * FtpLastResponse - return a pointer to the last response received
 */
AZX_FTP_GLOBALDEF CHAR *azx_ftp_lastResponse(AZX_FTP_NET_BUF_T *nControl)
{
  if ((nControl) && (nControl->dir == AZX_FTP_CONTROL))
  {
    return nControl->response;
  }
  return NULL;
}

/*
 * FtpConnect - connect to remote server
 *
 * return 1 if connected, 0 if not
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_connect(const CHAR *host, AZX_FTP_NET_BUF_T **nControl)
{
  INT32 sControl;
  struct sockaddr_in sin;
  INT32 on=1;
  M2MB_SSL_CONNECTION_HANDLE sslControl = 0;
  AZX_FTP_NET_BUF_T *ctrl;

  memset(&sin,0,sizeof(sin));
  sin.sin_family = AF_INET;

  if( FtpAssociateCid( host, ftp_opts.cid, &sin, &sControl ) != M2MB_RESULT_SUCCESS )
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"Cid association fail\r\n" );
    return 0;
  }

  if (setsockopt(sControl,SOL_SOCKET, SO_REUSEADDR,
      SETSOCKOPT_OPTVAL_TYPE &on, sizeof(on)) == -1)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"setsockopt\r\n");
    net_close(sControl);
    return 0;
  }

  if (connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"connect\r\n");
    net_close(sControl);
    return 0;
  }

  ctrl = (AZX_FTP_NET_BUF_T *)calloc(1,sizeof(AZX_FTP_NET_BUF_T));
#ifdef M2M_M2MB_SSL_H
  ctrl->AUTHdone = FALSE; //SSL authentication to be done
#endif
  if (ctrl == NULL)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"calloc\r\n");
    net_close(sControl);
    return 0;
  }
  ctrl->buf = (CHAR*)malloc(AZX_FTP_BUFSIZE);
  if (ctrl->buf == NULL)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"calloc\r\n");
    net_close(sControl);
    free(ctrl);
    return 0;
  }
  ctrl->response = (CHAR*)malloc(AZX_FTP_RESPONSE_BUFSIZE + 1 /*for eos*/);
  if (ctrl->response == NULL)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"calloc\r\n");
    net_close(sControl);
    free(ctrl->buf);
    free(ctrl);
    return 0;
  }


  ctrl->handle = sControl;
  ctrl->dir = AZX_FTP_CONTROL;
  ctrl->ctrl = NULL;
  ctrl->data = NULL;
  ctrl->cmode = AZX_FTP_DEFMODE;
  ctrl->idlecb = NULL;
  ctrl->idletime.tv_sec = ctrl->idletime.tv_usec = 0;
  ctrl->idlearg = NULL;
  ctrl->xfered = 0;
  ctrl->xfered1 = 0;
  ctrl->cbbytes = 0;
  ctrl->abort = 0;

  if (readresp('2', ctrl) == 0)
  {
    net_close(sControl);
    free(ctrl->buf);
    free(ctrl->response);
    free(ctrl);
    return 0;
  }
#ifdef M2M_M2MB_SSL_H
  INT32 sslRes;
  if(ftp_opts.ssl)
  {

    if (ctrl->AUTHdone == FALSE)
    {
      sslRes = FtpStartAuth('2', ctrl);
      if(sslRes == 0)
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"FtpStartAuth\r\n");
        net_close(sControl);
        free(ctrl->buf);
        free(ctrl);
        return 0;
      }
    }

    sslControl = m2mb_ssl_secure_socket( ftp_opts.sslConfigH,ftp_opts.sslCtxtH,sControl,&sslRes );
    if( sslControl == 0 )
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"m2mb_ssl_secure_socket FAILED error %d \r\n",sslRes );
      net_close(sControl);
      free(ctrl->buf);
      free(ctrl);
      return 0;
    }
    else
    {
      sslRes = m2mb_ssl_connect( sslControl );
      if( sslRes != 0 )
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"m2mb_ssl_connect FAILED error %d. Please verify module clock with AT+CCLK? command \r\n.",sslRes );
        return 0;
      }
      else
      {
        ctrl->AUTHdone = TRUE;
#ifdef TLS_REUSE
        if(ftp_opts.sslReuse)
        {
          ftp_opts.sslSessionH = m2mb_ssl_get_session(sslControl);
        }
#endif
      }
    }
  }

  ctrl->sslHandle = sslControl;
#endif
/*
  if (readresp('2', ctrl) == 0)
  {
    net_close(sControl);
    free(ctrl->buf);
    free(ctrl->response);
    free(ctrl);
    return 0;
  }*/
  *nControl = ctrl;
  return 1;
}

AZX_FTP_GLOBALDEF INT32 azx_ftp_setCallback(const AZX_FTP_CALLBACK_OPTIONS_T *opt, AZX_FTP_NET_BUF_T *nControl)
{
  if(NULL != nControl)
  {
    nControl->idlecb = opt->cbFunc;
    nControl->idlearg = opt->cbArg;
    nControl->idletime.tv_sec = opt->idleTime / 1000;
    nControl->idletime.tv_usec = (opt->idleTime % 1000) * 1000;
    nControl->cbbytes = opt->bytesXferred;
    return 1;
  }
  else
  {
    return 0;
  }
}
AZX_FTP_GLOBALDEF INT32 azx_ftp_clearCallback(AZX_FTP_NET_BUF_T *nControl)
{
  if(NULL != nControl)
  {
    nControl->idlecb = NULL;
    nControl->idlearg = NULL;
    nControl->idletime.tv_sec = 0;
    nControl->idletime.tv_usec = 0;
    nControl->cbbytes = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}
/*
 * FtpOptions - change connection options
 *
 * returns 1 if successful, 0 on error
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_options(INT32 opt, INT32 val, AZX_FTP_NET_BUF_T *nControl)
{
  INT32 v,rv=0;

  if(NULL != nControl)
  {

    switch (opt)
    {
    case AZX_FTP_CONNMODE:
      v = (int) val;
      if ((v == AZX_FTP_PASSIVE) || (v == AZX_FTP_PORT))
      {
        nControl->cmode = v;
        rv = 1;
      }
      break;
    case AZX_FTP_CALLBACK:
      nControl->idlecb = (azx_ftp_callback) val;
      rv = 1;
      break;
    case AZX_FTP_IDLETIME:
      v = (int) val;
      rv = 1;
      nControl->idletime.tv_sec = v / 1000;
      nControl->idletime.tv_usec = (v % 1000) * 1000;
      break;
    case AZX_FTP_CALLBACKARG:
      rv = 1;
      nControl->idlearg = (void *) val;
      break;
    case AZX_FTP_CALLBACKBYTES:
      rv = 1;
      nControl->cbbytes = (int) val;
      break;
    }
  }
  return rv;
}



/*
 * FtpRead - read from a data connection
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_read(void *buf, INT32 max, AZX_FTP_NET_BUF_T *nData)
{
  INT32 i;
  if(NULL == nData)
  {
    return 0;
  }
  
  if (nData->dir != AZX_FTP_READ)
  {
    return 0;
  }

  if (nData->buf)
  {
    i = readline((CHAR*)buf, max, nData);
  }
  else
  {
    i = socket_wait(nData);
    if (i != 1)
    {
      return 0;
    }
#ifdef M2M_M2MB_SSL_H
    if((ftp_opts.ssl == 1) && (nData->AUTHdone == TRUE))
    {
      i = net_ssl_read(nData->sslHandle,(CHAR*)buf,max);

    }
    else
    {
      i = net_read(nData->handle, (CHAR*)buf,max);
    }
#else
    i = net_read(nData->handle, (CHAR*)buf,max);
#endif
  }

  if (i == -1)
  {
    return 0;
  }
  nData->xfered += i;
  if (nData->idlecb && nData->cbbytes)
  {
    nData->xfered1 += i;
    if (nData->xfered1 > nData->cbbytes)
    {
      if (nData->idlecb(nData, nData->xfered, nData->idlearg) == 0)
      {
        nData->ctrl->abort = 1;
        return 0;
      }
      nData->xfered1 = 0;
    }
  }
  return i;
}

/*
 * FtpWrite - write to a data connection
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_write(const void *buf, INT32 len, AZX_FTP_NET_BUF_T *nData)
{
  INT32 i;
  if(NULL == nData)
  {
    return 0;
  }

  if (nData->dir != AZX_FTP_WRITE)
  {
    return 0;
  }
  if (nData->buf)
  {
    i = writeline((CHAR*)buf, len, nData);
  }
  else
  {
    socket_wait(nData);
#ifdef M2M_M2MB_SSL_H
    if((ftp_opts.ssl == 1) && (nData->AUTHdone == TRUE))
    {
      i =  net_ssl_write(nData->sslHandle, (CHAR*)buf, len);
    }
    else
    {
      i = net_write(nData->handle, (CHAR*)buf, len);
    }
#else
    i = net_write(nData->handle, (CHAR*)buf, len);
#endif
  }
  if (i == -1)
  {
    return 0;
  }
  nData->xfered += i;
  if (nData->idlecb && nData->cbbytes)
  {
    nData->xfered1 += i;
    if (nData->xfered1 > nData->cbbytes)
    {
      nData->idlecb(nData, nData->xfered, nData->idlearg);
      nData->xfered1 = 0;
    }
  }
  return i;
}

/*
 * FtpClose - close a data connection
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_close(AZX_FTP_NET_BUF_T *nData)
{
  AZX_FTP_NET_BUF_T *ctrl;
  if(NULL == nData)
  {
    return 0;
  }

  switch (nData->dir)
  {
  case AZX_FTP_WRITE:
    /* potential problem - if buffer flush fails, how to notify user? */
    if (nData->buf != NULL)
    {
      writeline(NULL, 0, nData);
    }
    /* fall through */
    FALL_THROUGH;
  case AZX_FTP_READ:
    if (nData->buf)
    {
      free(nData->buf);
    }
    /*shutdown is a dummy function*/
    shutdown(nData->handle,2);
    if(ftp_opts.ssl == 1 && (nData->AUTHdone == TRUE))
    {
      net_ssl_close(nData->handle, nData->sslHandle);
    }
    else
    {
    net_close(nData->handle);
    }
    ctrl = nData->ctrl;
    free(nData);

    if(ctrl)
    {
      ctrl->data = NULL;
    }
    if (ctrl && ctrl->response[0] != '4' && ctrl->response[0] != '5' && ctrl->abort != 1) //fabioPi: if the operation was aborted, do not attempt to read.
    {
      return(readresp('2', ctrl));
    }
    return 1;
  case AZX_FTP_CONTROL:
    if (nData->data)
    {
      nData->ctrl = NULL;
      azx_ftp_close(nData->data);
    }
    if(ftp_opts.ssl == 1 && (nData->AUTHdone == TRUE))
    {
      net_ssl_close(nData->handle, nData->sslHandle);
    }
    else
    {
    net_close(nData->handle);
    }
    free(nData);
    return 0;
  }
  return 1;
}

/*
 * FtpSite - send a SITE command
 *
 * return 1 if command successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_site(const CHAR *cmd, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR buf[AZX_FTP_TMP_BUFSIZ];

  if(NULL == nControl)
  {
    return 0;
  }

  if ((strlen(cmd) + 7) > sizeof(buf))
  {
    return 0;
  }
  sprintf(buf,"SITE %s",cmd);
  if (!FtpSendCmd(buf,'2',nControl))
    return 0;
  return 1;
}

/*
 * FtpSysType - send a SYST command
 *
 * Fills in the user buffer with the remote system type.  If more
 * information from the response is required, the user can parse
 * it out of the response buffer returned by FtpLastResponse().
 *
 * return 1 if command successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_sysType(CHAR *buf, INT32 max, AZX_FTP_NET_BUF_T *nControl)
{
  INT32 l = max;
  CHAR *b = buf;
  CHAR *s;

  if(NULL == nControl)
  {
    return 0;
  }

  if (!FtpSendCmd("SYST",'2',nControl))
  {
    return 0;
  }
  s = &nControl->response[4];
  while ((--l) && (*s != ' '))
    *b++ = *s++;
  *b++ = '\0';
  return 1;
}

/*
 * FtpMkdir - create a directory at server
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_mkdir(const CHAR *path, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR buf[AZX_FTP_TMP_BUFSIZ];

  if(NULL == nControl)
  {
    return 0;
  }

  if ((strlen(path) + 6) > sizeof(buf))
  {
    return 0;
  }
  sprintf(buf,"MKD %s",path);
  if (!FtpSendCmd(buf,'2', nControl))
    return 0;
  return 1;
}

/*
 * FtpChdir - change path at remote
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_chdir(const CHAR *path, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR buf[AZX_FTP_TMP_BUFSIZ];

  if(NULL == nControl)
  {
    return 0;
  }

  if ((strlen(path) + 6) > sizeof(buf))
  {
    return 0;
  }
  sprintf(buf,"CWD %s",path);
  if (!FtpSendCmd(buf,'2',nControl))
    return 0;
  return 1;
}

/*
 * FtpCDUp - move to parent directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_cdUp(AZX_FTP_NET_BUF_T *nControl)
{
  if(NULL == nControl)
  {
    return 0;
  }

  if (!FtpSendCmd("CDUP",'2',nControl))
    return 0;
  return 1;
}

/*
 * FtpRmdir - remove directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_rmdir(const CHAR *path, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR buf[AZX_FTP_TMP_BUFSIZ];

  if(NULL == nControl)
  {
    return 0;
  }

  if ((strlen(path) + 6) > sizeof(buf))
    return 0;
  sprintf(buf,"RMD %s",path);
  if (!FtpSendCmd(buf,'2',nControl))
    return 0;
  return 1;
}

/*
 * FtpPwd - get working directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_pwd(CHAR *path, INT32 max, AZX_FTP_NET_BUF_T *nControl)
{
  INT32 l = max;
  CHAR *b = path;
  CHAR *s;

  if(NULL == nControl)
  {
    return 0;
  }

  if (!FtpSendCmd("PWD",'2',nControl))
  {
    return 0;
  }
  s = strchr(nControl->response, '"');
  if (s == NULL)
  {
    return 0;
  }
  s++;
  while ((--l) && (*s) && (*s != '"'))
    *b++ = *s++;
  *b++ = '\0';
  return 1;
}


/*
 * FtpNlst - issue an NLST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_nlst(AZX_FTP_XFER_T *output, const CHAR *path,
    AZX_FTP_NET_BUF_T *nControl)
{

  AZX_FTP_FILE_INFO_T remote_info;
  remote_info.path = (CHAR*) path;
  remote_info.fileSize = 0;

  if(!path || ! output || !nControl)
  {
    return 0;
  }

  if( output->type == AZX_FTP_XFER_FILE )
  {
    return FtpXferFile( &( output->payload.fileInfo ), &remote_info, nControl, AZX_FTP_DIR, AZX_FTP_ASCII );

  }
  else if( output->type == AZX_FTP_XFER_BUFF )
  {
    return FtpXferBuffer( &( output->payload.buffInfo ), &remote_info, nControl, AZX_FTP_DIR, AZX_FTP_ASCII );
  }
  else
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"bad xfer type\r\n" );
    return 0;
  }
}

/*
 * FtpDir - issue a LIST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_dir(AZX_FTP_XFER_T *output, const CHAR *path, AZX_FTP_NET_BUF_T *nControl)
{
  AZX_FTP_FILE_INFO_T remote_info;
  remote_info.path = (CHAR*) path;
  remote_info.fileSize = 0;

  if(!path || ! output || !nControl)
  {
    return 0;
  }


  if( output->type == AZX_FTP_XFER_FILE )
  {
    return FtpXferFile( &( output->payload.fileInfo ), &remote_info, nControl, AZX_FTP_DIR_VERBOSE, AZX_FTP_ASCII );

  }
  else if( output->type == AZX_FTP_XFER_BUFF )
  {
    return FtpXferBuffer( &( output->payload.buffInfo ), &remote_info, nControl, AZX_FTP_DIR_VERBOSE, AZX_FTP_ASCII );
  }
  else
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"bad xfer type\r\n" );
    return 0;
  }
}

/*
 * FtpSize - determine the size of a remote file
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_size(const CHAR *path, UINT32 *size, CHAR mode, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR cmd[64];
  INT32 resp,rv=1;
  UINT32 sz;

  if(NULL == nControl)
  {
    return 0;
  }

  if ((strlen(path) + 7) > sizeof(cmd))
  {
    return 0;
  }
  sprintf(cmd, "TYPE %c", mode);
  AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "sending <%s>...\r\n", cmd);
  if (!FtpSendCmd(cmd, '2', nControl))
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"cannot change type\r\n");
    return 0;
  }
  sprintf(cmd,"SIZE %s",path);
  AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG,"sending <%s>...\r\n", cmd);
  if (!FtpSendCmd(cmd,'2',nControl))
  {
    rv = 0;
  }
  else
  {
    if (sscanf(nControl->response, "%d %u", &resp, &sz) == 2)
    {
      *size = sz;
    }
    else
    {
      rv = 0;
    }
  }
  return rv;
}

#if defined(__UINT64_MAX)
/*
 * FtpSizeLong - determine the size of a remote file
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 axz_ftp_sizeLong(const CHAR *path, azx_ftp_fsz_t *size, CHAR mode, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR cmd[AZX_FTP_TMP_BUFSIZ];
  INT32 resp,rv=1;
  azx_ftp_fsz_t sz;

  if(NULL == nControl)
  {
    return 0;
  }

  if ((strlen(path) + 7) > sizeof(cmd))
  {
    return 0;
  }
  sprintf(cmd, "TYPE %c", mode);
  if (!FtpSendCmd(cmd, '2', nControl))
  {
    return 0;
  }
  sprintf(cmd,"SIZE %s",path);
  if (!FtpSendCmd(cmd,'2',nControl))
  {
    rv = 0;
  }
  else
  {
    if (sscanf(nControl->response, "%d %llu", &resp, &sz) == 2)
    {
      *size = sz;
    }
    else
    {
      rv = 0;
    }
  }
  return rv;
}
#endif

/*
 * FtpModDate - determine the modification date of a remote file
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_modDate(const CHAR *path, CHAR *dt, INT32 max, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR buf[AZX_FTP_TMP_BUFSIZ];
  INT32 rv = 1;

  if(NULL == nControl)
  {
    return 0;
  }

  if ((strlen(path) + 7) > sizeof(buf))
  {
    return 0;
  }
  sprintf(buf,"MDTM %s",path);
  if (!FtpSendCmd(buf,'2',nControl))
  {
    rv = 0;
  }
  else
  {
    strncpy(dt, &nControl->response[4], max - 1);
    dt[max - 1] = '\0';
  }
  return rv;
}

/*
 * FtpGet - issue a GET command and write received data to output
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_get(AZX_FTP_XFER_T *output, const CHAR *path,
        CHAR mode, AZX_FTP_NET_BUF_T *nControl)
{
  AZX_FTP_FILE_INFO_T remote_info;
  INT32 ret;
  remote_info.path = (CHAR*) path;
  remote_info.fileSize = 0;

  if(!path || !output || !nControl)
  {
    return 0;
  }


  ret = azx_ftp_size(remote_info.path, &remote_info.fileSize, AZX_FTP_BINARY, nControl);
  if (ret == 1)
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_DEBUG, "Done. File size: %u.\r\n", remote_info.fileSize);
  }
  else
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"failed file size.. error: %s \r\n", azx_ftp_lastResponse(nControl));
    remote_info.fileSize = 0;
  }

  if( output->type == AZX_FTP_XFER_FILE )
  {
    ret = FtpXferFile( &( output->payload.fileInfo ), &remote_info, nControl, AZX_FTP_FILE_READ, mode );
    if( !ret )
    {
      if(nControl->abort)
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR, "Transmission aborted by user\r\n" );
      }
      else
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR, "Bad answer\r\n" );
      }
      return ret;
    }
  }
  else if( output->type == AZX_FTP_XFER_BUFF )
  {
    ret = FtpXferBuffer( &( output->payload.buffInfo ), &remote_info, nControl, AZX_FTP_FILE_READ, mode );
    if( !ret )
    {
      if(nControl->abort)
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR, "Transmission aborted by user\r\n" );
      }
      else
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR, "Bad answer. error: %s \r\n", azx_ftp_lastResponse(nControl) );
      }
      return ret;
    }
  }
  else
  {
    AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"bad xfer type\r\n" );
    return 0;
  }

  return ret;
}

/*
 * FtpPut - issue a PUT command and send data from input
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_put(AZX_FTP_XFER_T *input, const CHAR *path, CHAR mode,
        AZX_FTP_NET_BUF_T *nControl)
{
  AZX_FTP_FILE_INFO_T remote_info;
  remote_info.path = (CHAR*) path;
  remote_info.fileSize = 0;
  INT32 ret = 0;

  if(NULL == nControl)
  {
    return 0;
  }

  do
  {
    if( input->type == AZX_FTP_XFER_FILE )
    {
      ret = FtpXferFile( &( input->payload.fileInfo ), &remote_info, nControl, AZX_FTP_FILE_WRITE, mode );
      if( !ret )
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR, "bad answer\r\n" );
        break;
      }
    }
    else if( input->type == AZX_FTP_XFER_BUFF )
    {
      ret = FtpXferBuffer( &( input->payload.buffInfo ), &remote_info, nControl, AZX_FTP_FILE_WRITE, mode );
      if( !ret )
      {
        AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR, "bad answer\r\n" );
        break;
      }
    }
    else
    {
      AZX_FTP_DEBUG(AZX_FTP_DEBUG_HOOK_ERROR,"bad xfer type\r\n" );
      break;
    }
  }while(0);

  return ret;
}

/*
 * FtpRename - rename a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_rename(const CHAR *src, const CHAR *dst, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR cmd[AZX_FTP_TMP_BUFSIZ];


  if(NULL == nControl)
  {
    return 0;
  }

  if (((strlen(src) + 7) > sizeof(cmd)) ||
      ((strlen(dst) + 7) > sizeof(cmd)))
  {
    return 0;
  }
  sprintf(cmd,"RNFR %s",src);
  if (!FtpSendCmd(cmd,'3',nControl))
  {
    return 0;
  }
  sprintf(cmd,"RNTO %s",dst);
  if (!FtpSendCmd(cmd,'2',nControl))
  {
    return 0;
  }
  return 1;
}

/*
 * FtpDelete - delete a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF INT32 azx_ftp_delete(const CHAR *fnm, AZX_FTP_NET_BUF_T *nControl)
{
  CHAR cmd[AZX_FTP_TMP_BUFSIZ];

  if(NULL == nControl)
  {
    return 0;
  }

  if ((strlen(fnm) + 7) > sizeof(cmd))
  {
    return 0;
  }
  sprintf(cmd,"DELE %s",fnm);
  if (!FtpSendCmd(cmd,'2', nControl))
  {
    return 0;
  }
  return 1;
}

/*
 * FtpQuit - disconnect from remote
 *
 * return 1 if successful, 0 otherwise
 */
AZX_FTP_GLOBALDEF void azx_ftp_quit(AZX_FTP_NET_BUF_T *nControl)
{
  if(NULL == nControl)
  {
    return;
  }
  if (nControl->dir != AZX_FTP_CONTROL)
  {
    return;
  }

  FtpSendCmd("QUIT",'2',nControl);
  if(ftp_opts.ssl == 1 && (nControl->AUTHdone == TRUE))
  {
    net_ssl_close(nControl->handle, nControl->sslHandle);
  }
  else
  {
  net_close(nControl->handle);
  }
  free(nControl->buf);
  free(nControl->response);
  free(nControl);
}
