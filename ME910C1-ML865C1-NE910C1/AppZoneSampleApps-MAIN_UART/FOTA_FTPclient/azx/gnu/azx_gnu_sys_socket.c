/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
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
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_socket.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"

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
/*-----------------------------------------------------------*/
int azx_gnu_getaddrinfo(const char *node, const char *service,
		const struct addrinfo *hints, struct addrinfo **res)
{
	uint8_t cid = azx_gnu_getgCID();
	int32_t herr;
	int32_t tmp_ret;
	struct M2MB_SOCKET_BSD_HOSTENT he;
	struct M2MB_SOCKET_BSD_HOSTENT *phe;
	struct addrinfo * p = NULL;
	int num = 0;

	int32_t tmpbuf_size = 1024;
	char * tmpbuf = NULL;

	int returnStatus = 0;

	if (service != NULL && service[0] == '*' && service[1] == 0)
	{
		service = NULL;
	}


	if( node == NULL && service == NULL)
	{
		AZX_LOG_ERROR( "Parameter check failed: either node or service must be not NULL.\r\n" );
		returnStatus = EAI_SYSTEM;
	}
	else if( res == NULL )
	{
		AZX_LOG_ERROR("Parameter check failed: res is NULL.\r\n" );
		returnStatus = EAI_SYSTEM;
	}

	if( returnStatus == 0)
	{
		if (service && service[0])
		{
			char *c;

			num = strtoul (service, &c, 10);
			if (*c != '\0')
			{
				if (hints->ai_flags & AI_NUMERICSERV)
				{
					return EAI_NONAME;
				}
				num = -1;
			}
		}
		else
		{
			num = -1;
		}
	}

	if( returnStatus == 0)
	{
		tmpbuf = (char*)m2mb_os_malloc(sizeof(char) * tmpbuf_size);
		if( tmpbuf == NULL)
		{
			AZX_LOG_ERROR( "Cannot allocate buffer for get host by name.\r\n" );
			returnStatus = EAI_MEMORY;
		}
	}




	struct addrinfo **pai = &p;

	if( returnStatus == 0)
	{
		AZX_LOG_DEBUG("IPv6 resolve for <%s> on cid %u \r\n", node, cid);
		tmp_ret = m2mb_socket_bsd_get_host_by_name_2_r_cid( ( CHAR *) node, M2MB_SOCKET_BSD_AF_INET6, &he,
				tmpbuf,(SIZE_T) tmpbuf_size, &phe,
				(INT32*) &herr, cid);

		if( ( tmp_ret == 0 ) && ( phe != NULL ) && ( phe->h_addrtype == M2MB_SOCKET_BSD_AF_INET6 ) )
		{
			/* IPv6 host resolution attempt succeeded */
			uint32_t i = 0;
			size_t socklen = sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN6);
			INT32 addrinfo_alloc_size = sizeof(struct addrinfo) + socklen;
			while ( phe->h_addr_list[i] != NULL)
			{
				struct addrinfo *ai;
				ai = *pai = (struct addrinfo *)m2mb_os_malloc (addrinfo_alloc_size);

				if( ai == NULL)
				{
					AZX_LOG_ERROR( "Cannot allocate ai for get host by name.\r\n" );
					returnStatus = EAI_MEMORY;
				}
				else
				{
					memset(ai, 0, addrinfo_alloc_size);
					ai->ai_family = phe->h_addrtype; // M2MB_SOCKET_BSD_AF_INET6
					ai->ai_addrlen = socklen;
					ai->ai_socktype = hints->ai_socktype;
					ai->ai_protocol = hints->ai_protocol;
					ai->ai_addr = (struct M2MB_SOCKET_BSD_SOCKADDR *) ( (struct addrinfo *)(ai + 1)); /*move at the end of the structure*/
					ai->ai_next = NULL;
					struct M2MB_SOCKET_BSD_SOCKADDR_IN6 *sin6p = (struct M2MB_SOCKET_BSD_SOCKADDR_IN6 *) ai->ai_addr;
					sin6p->sin6_family = M2MB_SOCKET_BSD_AF_INET6;
					if(num != -1)
					{
						UINT16 port = (UINT16) (num % 0x10000); /*truncate to 16 bits */
						sin6p->sin6_port = m2mb_socket_bsd_htons(port);
					}
					else
					{
						sin6p->sin6_port = 0;
					}

					sin6p->sin6_flowinfo = 0;
					memcpy (&sin6p->sin6_addr,
							phe->h_addr_list[i], sizeof (struct M2MB_SOCKET_BSD_IPV6_ADDR));
					sin6p->sin6_scope_id = 0;
					AZX_LOG_TRACE("Server IPv6 address stored \r\n");
					pai = &(ai->ai_next);
					i++;
				}
			}
		}
		else
		{
			AZX_LOG_TRACE("Cannot resolve IPv6 \r\n");
		}

		AZX_LOG_DEBUG("IPv4 resolve for <%s> on cid %u \r\n", node, cid);
		tmp_ret = m2mb_socket_bsd_get_host_by_name_2_r_cid( ( CHAR *) node, M2MB_SOCKET_BSD_AF_INET, &he,
				tmpbuf,(SIZE_T) tmpbuf_size, &phe,
				(INT32*) &herr, cid);

		if( ( tmp_ret == 0 ) && ( phe != NULL ) && ( phe->h_addrtype == M2MB_SOCKET_BSD_AF_INET ) )
		{
			/* IPv4 host resolution attempt succeeded */
			uint32_t i = 0;
			size_t socklen = sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN);
			INT32 addrinfo_alloc_size = sizeof(struct addrinfo) + socklen;


			while ( phe->h_addr_list[i] != NULL)
			{
				struct addrinfo *ai;
				ai = *pai = (struct addrinfo *)m2mb_os_malloc (addrinfo_alloc_size);

				if( ai == NULL)
				{
					AZX_LOG_ERROR( "Cannot allocate ai for get host by name. \r\n" );
					returnStatus = EAI_MEMORY;
				}
				else
				{
					memset(ai, 0, sizeof(struct addrinfo));
					ai->ai_family = phe->h_addrtype; // M2MB_SOCKET_BSD_AF_INET4
					ai->ai_addrlen = socklen;
					ai->ai_socktype = hints->ai_socktype;
					ai->ai_protocol = hints->ai_protocol;
					ai->ai_addr = (struct M2MB_SOCKET_BSD_SOCKADDR *) ((struct addrinfo*)(ai + 1)); /*move at the end of the structure*/
					ai->ai_next = NULL;

					struct M2MB_SOCKET_BSD_SOCKADDR_IN *sinp = (struct M2MB_SOCKET_BSD_SOCKADDR_IN *) ai->ai_addr;

					if(num != -1)
					{
						UINT16 port = (UINT16) (num % 0x10000); /*truncate to 16 bits */
						sinp->sin_port = m2mb_socket_bsd_htons(port);
					}
					else
					{
						sinp->sin_port = 0;
					}

					sinp->sin_family = M2MB_SOCKET_BSD_AF_INET;
					memcpy (&sinp->sin_addr, phe->h_addr_list[i], sizeof (struct M2MB_SOCKET_BSD_IN_ADDR));
					memset (sinp->sin_zero, '\0', sizeof (sinp->sin_zero));
					AZX_LOG_TRACE("Server IPv4 address stored \r\n");

					pai = &(ai->ai_next);
					i++;
				}
			}
		}
		else
		{
			AZX_LOG_TRACE("Cannot resolve IPv4!");
		}
	}

	if(tmpbuf != NULL)
	{
		m2mb_os_free(tmpbuf);
	}

	if (p)
	{
		*res = p;
		return 0;
	}
  else
  {
    returnStatus = EAI_NONAME;
  }

	return returnStatus;

}

/*-----------------------------------------------------------*/

void azx_gnu_freeaddrinfo (struct addrinfo *ai)
{
	struct addrinfo *p;
	while (ai != NULL)
	{
		p = ai;
		ai = ai->ai_next;
		m2mb_os_free (p);
	}
}
/*-----------------------------------------------------------*/
