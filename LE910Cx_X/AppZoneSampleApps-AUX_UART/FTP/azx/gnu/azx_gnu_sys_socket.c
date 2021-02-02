/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 azx_gnu_sys_socket.c

 @brief
 gnu definition

 @details
 Porting from gnu to azx

 @note
 <note>

 @author Fabio Pintus
 @author Norman Argiolas

 @date
 11/02/2020
 */

/* Include files ================================================================================*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#include "m2mb_types.h"
#include "m2mb_socket.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"

#include "azx_gnu_sys_types.h"

#include "azx_log.h"
#include "azx_gnu_sys_time.h"
#include "azx_gnu_stdio.h"
#include "azx_gnu_stdlib.h"

/* Function prototypes and Local defines ========================================================*/
#include "azx_gnu_sys_socket.h"

/* Local defines ================================================================================*/
#define	HOST_NOT_FOUND	1 /* Authoritive Answer Host not found */
#define	TRY_AGAIN		2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY		3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA			4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS		NO_DATA		/* no address, look for MX record */

/* Global definition =============================================================================*/
INT32 azx_gnu_h_errno;

/* Local statics ================================================================================*/
static struct M2MB_SOCKET_BSD_HOSTENT g_he;
static UINT8 g_CID = 0;


/* Function definition ==========================================================================*/

void azx_gnu_setgCID(UINT8 c)
{
	g_CID = c;
}

UINT8 azx_gnu_getgCID(void)
{
	return g_CID;
}

/*
 * Return a string containing some additional information after a
 * host name or address lookup error - azx_gnu_gethostbyname() or gethostbyaddr().
 *
 * This is only compiled if the local host does not provide it--recent
 * versions of BIND supply this function.
 */
const char *azx_gnu_hstrerror(int err)
{
	if (err == 0)
		return("no error");

	if (err == HOST_NOT_FOUND)
		return("Unknown host");

	if (err == TRY_AGAIN)
		return("Hostname lookup failure");

	if (err == NO_RECOVERY)
		return("Unknown server error");

	if (err == NO_DATA)
        return("No address associated with name");

	return("unknown error");
}


struct M2MB_SOCKET_BSD_HOSTENT *azx_gnu_gethostbyname(const char *name)
{
	struct M2MB_SOCKET_BSD_HOSTENT *result;
	char buf[64];

	if(0 == m2mb_socket_bsd_get_host_by_name_2_r_cid(name, M2MB_SOCKET_BSD_AF_INET, &g_he, buf, sizeof(buf), &result, &azx_gnu_h_errno, azx_gnu_getgCID()))
	{
		return result;
	}
	else
	{
		return (struct M2MB_SOCKET_BSD_HOSTENT *)NULL;
	}
}

int azx_gnu_gethostbyname_r(char *name, struct M2MB_SOCKET_BSD_HOSTENT *he, CHAR *buf, SIZE_T buflen,
	    			struct M2MB_SOCKET_BSD_HOSTENT **result, int *h_errnop)
{

	return (int) m2mb_socket_bsd_get_host_by_name_2_r_cid(name, M2MB_SOCKET_BSD_AF_INET, he, buf, (SIZE_T) buflen, result, (INT32*)h_errnop, azx_gnu_getgCID());
}


int azx_gnu_socket_bsd_socket_global_cid( INT32 domain, INT32 type, INT32 protocol )
{
	M2MB_SOCKET_BSD_SOCKET s = m2mb_socket_bsd_socket(domain, type, protocol);
	if (-1 == s)
	{
		return -1;
	}
	else
	{

		if ( m2mb_socket_set_cid( s, azx_gnu_getgCID() ) == 0 )
		{
			return (int) s;
		}
		else
		{
			return -1;
		}
	}
}


int azx_gnu_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)
{
	int retvalue = 0;
	struct M2MB_SOCKET_BSD_TIMEVAL m2m_tv;
	m2m_tv.m_tv_sec = timeout->tv_sec;
	m2m_tv.m_tv_usec = timeout->tv_usec;

	if (writefds != NULL)
	{
		AZX_LOG_TRACE("checking write...\r\n");
		return 1; //fake response in write mode
	}
	AZX_LOG_TRACE("starting select...with sec timeout: %d\r\n", timeout->tv_sec);
	retvalue = m2mb_socket_bsd_select(nfds, readfds, writefds, exceptfds, &m2m_tv);
	AZX_LOG_TRACE("select result: %d\r\n\r\n", retvalue);
	return retvalue;
}

