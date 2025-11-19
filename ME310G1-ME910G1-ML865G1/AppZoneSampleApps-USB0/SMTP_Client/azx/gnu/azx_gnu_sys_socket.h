/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
  azx_gnu_sys_socket.h

  @brief
  gnu definition

  @details
  Porting from gnu to azx

  @note

  @author Moreno Floris
  @author Norman Argiolas

  @date
  11/02/2020
*/

#ifndef HDR_AZX_GNU_SYS_SOCKET_H_
#define HDR_AZX_GNU_SYS_SOCKET_H_

/* Global defines ================================================================================*/

/* closesocket() is a windows function. Linux uses the close() function for both
   files and sockets because a socket is a file descriptor after all.
    In AppZone though they're not the same, because M2MB_SOCKET_BSD_SOCKET is a INT32
    and M2MB_T_FS_HANDLE is a INT32
    Therefore look for all the close() calls in your code and rename to closesocket()
    the ones that refer to sockets, or find some other way around it.
    YOU CAN NOT CLOSE A SOCKET WITH THE close() FUNCTION BECAUSE ITS' MAPPED TO m2mb_fs_close()
*/
#define HAVE_CLOSESOCKET

/**   Structs     **/
#define sockaddr          M2MB_SOCKET_BSD_SOCKADDR
#define sockaddr_in       M2MB_SOCKET_BSD_SOCKADDR_IN
#define sockaddr_in6      M2MB_SOCKET_BSD_SOCKADDR_IN6
#define hostent           M2MB_SOCKET_BSD_HOSTENT
#define h_addr            h_addr_list[0] /* Address, for backward compatibility.*/
#define in_addr           M2MB_SOCKET_BSD_IN_ADDR
#define in6_addr          M2MB_SOCKET_BSD_IPV6_ADDR
#define s6_addr           addr8_s
#undef fd_set
#define fd_set 			  M2MB_SOCKET_BSD_FD_SET_T
/**   End of structs  **/

/**   Functions         **/
#define socket            m2mb_socket_bsd_socket
#define closesocket       m2mb_socket_bsd_close
#define shutdown(a,b)     UNUSED_2(a,b) /*dummy function*/
#define ntohs             m2mb_socket_bsd_ntohs
#define ntohl             m2mb_socket_bsd_ntohl
#define htonl             m2mb_socket_bsd_htonl
#define htons             m2mb_socket_bsd_htons
#define setsockopt        m2mb_socket_bsd_set_sock_opt
#define inet_addr         m2mb_socket_bsd_inet_addr
#define inet_pton         m2mb_socket_bsd_inet_pton
#define recvfrom          m2mb_socket_bsd_recv_from
#define recv              m2mb_socket_bsd_recv
#define sendto            m2mb_socket_bsd_send_to
#define send              m2mb_socket_bsd_send
#define getsockname       m2mb_socket_bsd_get_sock_name
#define getsockopt        m2mb_socket_bsd_get_sock_opt
#define bind              m2mb_socket_bsd_bind
#define connect           m2mb_socket_bsd_connect
#define listen            m2mb_socket_bsd_listen
#define accept            m2mb_socket_bsd_accept
#define inet_ntoa(a)      m2mb_socket_bsd_addr_str(a.s_addr)
#define select            azx_gnu_select

#undef	fd_set
#define	fd_set		M2MB_SOCKET_BSD_FD_SET_T
#undef	fds_bits
#define	fds_bits	fd_array


#undef getaddrinfo
#define getaddrinfo azx_gnu_getaddrinfo

#undef freeaddrinfo
#define freeaddrinfo azx_gnu_freeaddrinfo

/** Macros **/
#define _SS_MAXSIZE       128  /* Implementation specific max size */
#define _SS_ALIGNSIZE     (sizeof (INT64)) /* Implementation specific desired alignment */

/*
   Definitions used for sockaddr_storage structure paddings design.
*/
#define _SS_PAD1SIZE   (_SS_ALIGNSIZE - sizeof (sa_family_t))
#define _SS_PAD2SIZE   (_SS_MAXSIZE - (sizeof (sa_family_t)+ _SS_PAD1SIZE + _SS_ALIGNSIZE))
/**End of Macros **/

#ifdef FD_SET
  #undef FD_SET
#endif
#define FD_SET(n, p)    m2mb_socket_bsd_fd_set_func(n, p)

#ifdef FD_CLR
  #undef FD_CLR
#endif
#define FD_CLR(n, p)    m2mb_socket_bsd_fd_clr_func(n, p)

#ifdef FD_ISSET
  #undef FD_ISSET
#endif
#define FD_ISSET(n, p)  m2mb_socket_bsd_fd_isset_func(n, p)

#ifdef FD_ZERO
  #undef FD_ZERO
#endif
#define FD_ZERO(p)    m2mb_socket_bsd_fd_zero_func(p)

/** End of Functions    **/

/**   Types   **/
#ifndef _IN_ADDR_T_DECLARED
typedef UINT32 in_addr_t;
#endif
typedef char *__caddr_t;
typedef UINT8 uint8_t;

/**   End of Types  **/

/**   Constants     **/
#define AF_INET         M2MB_SOCKET_BSD_AF_INET
#define AF_INET6        M2MB_SOCKET_BSD_AF_INET6
#define PF_INET         M2MB_SOCKET_BSD_PF_INET
#define PF_INET6        AF_INET6
#define IPPROTO_TCP     M2MB_SOCKET_BSD_IPPROTO_TCP
#define IPPROTO_UDP     M2MB_SOCKET_BSD_IPPROTO_UDP
#define AF_UNSPEC       M2MB_SOCKET_BSD_AF_UNSPEC
#define SOCK_STREAM     M2MB_SOCKET_BSD_SOCK_STREAM
#define SOCK_DGRAM      M2MB_SOCKET_BSD_SOCK_DGRAM
#define SO_SNDBUF       M2MB_SOCKET_BSD_SO_SNDBUF
#define SO_RCVBUF       M2MB_SOCKET_BSD_SO_RCVBUF
#define SO_REUSEADDR    M2MB_SOCKET_BSD_SO_REUSEADDR
#define SO_LINGER       M2MB_SOCKET_BSD_SO_LINGER
#define INADDR_ANY      M2MB_SOCKET_BSD_INADDR_ANY
#define SOL_SOCKET      M2MB_SOCKET_BSD_SOL_SOCKET
#define SO_ERROR        M2MB_SOCKET_BSD_SO_ERROR

#ifdef FD_SETSIZE
  #undef FD_SETSIZE
#endif
#define FD_SETSIZE      M2MB_SOCKET_BSD_FD_SETSIZE

/* Possible values for `ai_flags' field in `addrinfo' structure.  */
#ifndef AI_PASSIVE
  #define AI_PASSIVE      0x0001  /* Socket address is intended for `bind'.  */
#endif
#ifndef AI_CANONNAME
  #define AI_CANONNAME    0x0002  /* Request for canonical name.  */
#endif
#ifndef AI_NUMERICSERV
  #define AI_NUMERICSERV  0x0400  /* Don't use name resolution.  */
#endif


#ifndef EAI_FAIL
#define	EAI_FAIL		 4	/* non-recoverable failure in name resolution */
#endif
#ifndef EAI_MEMORY
#define	EAI_MEMORY		 6	/* memory allocation failure */
#endif
#ifndef EAI_NODATA
#define	EAI_NODATA		 7	/* no address associated with host */
#endif
#ifndef EAI_NONAME
#define	EAI_NONAME		 8	/* host nor service provided, or not known */
#endif
#ifndef EAI_SYSTEM
#define	EAI_SYSTEM		11	/* system error returned in errno */
#endif

/** From in.h --> **/
#define INET_ADDRSTRLEN   16
#define INET6_ADDRSTRLEN  46
/** <-- From in.h **/
//struct in6_addr in6addr_any; //TODO: initialize this to the ipv6 wildcard address :: or 0:0:0:0:0:0:0:0
/** From if.h --> **/
#define IF_NAMESIZE       16
/** <-- From if.h **/

/** End of Defines  **/

/* Utilities for M2MB*/

/* Global definition =============================================================================*/

/** Wrappers        **/
typedef unsigned int socklen_t;
typedef UINT8 sa_family_t;

struct sockaddr_storage
{
  sa_family_t __ss_family; /* address family */
  /* Following fields are implementation specific */
  char __ss_pad1[_SS_PAD1SIZE];
  /* 6 byte pad, this is to make implementation */
  /* specific pad up to alignment field that */
  /* follows explicit in the data structure */
  INT64 __ss_align; /* field to force desired structure */
  /* storage alignment */
  char __ss_pad2[_SS_PAD2SIZE];
  /* 112 byte pad to achieve desired size, */
  /* _SS_MAXSIZE value minus size of ss_family */
  /* __ss_pad1, __ss_align fields is 112 */
};
struct addrinfo {
               int              ai_flags;
               int              ai_family;
               int              ai_socktype;
               int              ai_protocol;
               socklen_t        ai_addrlen;
               struct M2MB_SOCKET_BSD_SOCKADDR *ai_addr;
               char            *ai_canonname;
               struct addrinfo *ai_next;
           };

#define INET6_ADDRSTRLEN 46

/** End of Wrappers **/

/* Function prototypes ====================================================================*/

/**   Wrappers    **/

/*-----------------------------------------------------------------------------------------------*/
#ifndef __MINGW32__
/**
  @brief
    get network host entry

  @details
    implements a "standard" Unix version of gethostbyname2_r() but with parameter "cid"
    to specify the PDP context to be used for transferring data

  @param[in] name
    either a hostname, or an IPv4 address in standard dot notation or an IPv6 address in colon notation

  @return
    0 on success, non-zero on error, additional error information is stored in *h_errnop instead of h_errno

  @note
    <Notes>

  @b
    Example
  @code
    <C code example>
  @endcode
*/
struct M2MB_SOCKET_BSD_HOSTENT *azx_gnu_gethostbyname( const char *name );
#endif
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    get network host entry

  @details
    implements a "standard" Unix version of gethostbyname2_r() but with parameter "cid"
    to specify the PDP context to be used for transferring data

  @param[in] name
    either a hostname, or an IPv4 address in standard dot notation or an IPv6 address in colon notation
  @param[in] ret
    pre-allocated struct where to store the result
  @param[in] buf
    pre-allocated buffer where to store additional data
  @param[in] buflen
    the size of buf
  @param[in] result
    pointer to a hostent pointer that is set to ret on success and set to zero on error
  @param[in] h_errnop
    pointer to an int where to store errors (instead of modifying the global h_errno)

  @return
    0 on success, non-zero on error, additional error information is stored in *h_errnop instead of h_errno

  @note
    <Notes>

  @b
    Example
  @code
    <C code example>
  @endcode
*/
int azx_gnu_gethostbyname_r( char *name, struct M2MB_SOCKET_BSD_HOSTENT *he,
                           CHAR *buf, SIZE_T buflen, struct M2MB_SOCKET_BSD_HOSTENT **result,
                           int *h_errnop );

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    create an endpoint for communication

  @details
    creates an unbound socket in a communications domain,
    and returns a file descriptor that can be used in later function calls that operate on socket

  @param[in] domain
    specifies the communications domain in which a socket is to be created:
    M2MB_SOCKET_BSD_AF_INET  - IPv4 Internet protocols
    M2MB_SOCKET_BSD_AF_INET6  - IPv6 Internet protocols
  @param[in] type
    specifies the type of socket to be created:
    M2MB_SOCKET_BSD_SOCK_STREAM     - TCP
    M2MB_SOCKET_BSD_SOCK_DGRAM      - UDP
  @param[in] protocol
    specifies a particular protocol to be used with the socket:
    M2MB_SOCKET_BSD_IPPROTO_TCP
    M2MB_SOCKET_BSD_IPPROTO_UDP
    specifying a protocol of 0 causes m2mb_socket_bsd_socket() to use an unspecified default protocol
    appropriate for the requested socket type

  @return
    On success, a handle for the new socket is returned. On error, -1 is returned

  @note
    <Notes>

  @b
    Example
  @code
    <C code example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
int azx_gnu_socket_bsd_socket_global_cid( INT32 domain, INT32 type, INT32 protocol );

INT32 azx_gnu_socket_bsd_get_sock_name( M2MB_SOCKET_BSD_SOCKET s,
                                        struct M2MB_SOCKET_BSD_SOCKADDR *name, INT32 *namelen );

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    synchronous I/O multiplexing

  @details
    This function is used to prune a provided list of sockets for those that are: ready to read, ready to write or have errors

  @param[in] nfds
    nfds is the highest-numbered file descriptor in any of the three sets, plus 1
  @param[in] readfds
    set of file descriptors that will be watched to see if characters become available for reading.
    On exit, the set is modified in place to indicate which file descriptors actually changed status.
  @param[in] writefds
    set of file descriptors that will be watched to see if a write will not block.
    On exit, the set is modified in place to indicate which file descriptors actually changed status.
  @param[in] exceptfds
    set of file descriptors that will be watched for exceptions.
    On exit, the set is modified in place to indicate which file descriptors actually changed status.
  @param[in] timeout
    specifies the minimum interval that m2mb_socket_bsd_select() should block waiting for a file descriptor to become ready

  @return
    number of sockets which had an event occur at and become ready

  @note
    <Notes>

  @b
    Example
  @code
    <C code example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
int azx_gnu_select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                    const struct timeval *timeout );

/**   End of Wrappers **/

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
   Sets CID

  @details
    Sets CID

  @param[in] c
      Selected CID number

  @return

  @note
    <Notes>

  @b
    Example
  @code
    <C code example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
void azx_gnu_setgCID( UINT8 c );

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
   Returns CID

  @details
    Returns CID

  @param[in]

  @return
    Retrurns current CID

  @note
    <Notes>

  @b
    Example
  @code
    <C code example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
UINT8 azx_gnu_getgCID( void );

/** From netdb.h --> **/
extern INT32 azx_gnu_h_errno;

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
   Return information after a host name or address lookup error

  @details
    Return a string containing some additional information after a host name or address lookup error - azx_gnu_gethostbyname() or gethostbyaddr().

  @param[in] err

  @return
    Retrurns current CID

  @note
    This is only compiled if the local host does not provide it--recent versions of BIND supply this function.

  @b
    Example
  @code
    <C code example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
const char *azx_gnu_hstrerror( int err );
#define hstrerror azx_gnu_hstrerror
/** <-- From netdb.h **/
int azx_gnu_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);

void azx_gnu_freeaddrinfo (struct addrinfo *ai);

#endif /* HDR_AZX_GNU_SYS_SOCKET_H_ */
