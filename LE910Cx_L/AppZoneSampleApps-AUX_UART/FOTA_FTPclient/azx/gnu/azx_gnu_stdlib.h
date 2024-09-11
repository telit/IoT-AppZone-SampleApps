/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 azx_gnu_stdlib.h

 @brief
 gnu definition

 @details
 Porting from gnu to azx

 @note
 Dependencies:
 m2mb_types.h
 m2mb_os_api.h

 @author Moreno Floris
 @author Norman Argiolas

 @date
 11/02/2020
 */

#ifndef HDR_AZX_GNU_STDLIB_H_
#define HDR_AZX_GNU_STDLIB_H_

#ifdef __cplusplus
extern "C"
{
#endif

#undef exit

/* Include files ================================================================================*/

#include <stdlib.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"

/* Global defines ================================================================================*/
#ifndef malloc
#define malloc(a)		azx_gnu_malloc(a)
#endif

#ifndef calloc
#define calloc(a,b)		azx_gnu_calloc(a,b)
#endif

#ifndef free
#define free(a)			azx_gnu_free(a)
#endif

#ifndef realloc
#define realloc(a,b) 	azx_gnu_realloc(a,b)
#endif

#undef exit
#define exit azx_gnu_exit
/* Function prototypes ====================================================================*/

//azx_gnu_malloc etc etc  TODO add doxygen description

/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
 Allocates bytes of memory

 @details
 This function provides service to reserve memory space to the caller

 @param [in] size
 size in byte of memory to be allocated

 @return
 valid pointer in case of success

 @return
 NULL in case of error


 @note
 The performance of this service is a function of the requested block size and the
 amount of fragmentation in the heap. Hence, this service should not be
 used during time-critical task of execution.
 Allowed From
 Initialization and tasks
 Preemption Possible
 Yes

 @b
 //pointer to 10 UINT32
 UINT32 *pUint;
 pUint = ( UINT32 * )m2mb_os_malloc( 10 * sizeof(UINT32) );
 if ( pUint == NULL )
 exit(...)
 @code

 @endcode
 */
/*-----------------------------------------------------------------------------------------------*/
void * azx_gnu_malloc(size_t size);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Allocates bytes of memory and init space with 0

  @details
    This function provides service to reserve memory space to the caller and initialize it to 0

  @param [in] size
    size in byte of memory to be allocated and initialize

  @return
    valid pointer in case of success

  @return
    NULL in case of error

  @note
    The performance of this service is a function of the requested block size and the
    amount of fragmentation in the heap. Hence, this service should not be
    used during time-critical task of execution.
    Allowed From
     Initialization and tasks
    Preemption Possible
      Yes

  @b
    Example
  @code
    //pointer to 10 UINT32
    typedf struct
    {
      INT32 a;
      INT8  b;
      void *ptr;
    }GEN_T;

    GEN_T *pStruct;
    pStruct = ( GEN_T * )m2mb_os_calloc( sizeof(GEN_T) );
    if ( pStruct == NULL )
      exit(...)
    //all pStruct initialized to 0: pStruct->a = 0; pStruct->b = 0; pStruct->ptr = 0;
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
void * azx_gnu_calloc(size_t nitems, size_t size);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Free allocated memory

  @details
    This function provides service to free already allocated memory space

  @param [in] pMem
    pointer to memory where to release previous allocation

  @return
    M2MB_OS_SUCCESS in case of success

  @return
    M2MB_OS_PTR_ERROR
      Invalid memory area pointer
    M2MB_OS_CALLER_ERROR
      Invalid caller of this service

  @note
    The application must prevent using the memory area after it is released.
    Allowed From
     Initialization and tasks
    Preemption Possible
      Yes

  @b
    Example
  @code
    M2MB_OS_RESULT_E osRes;
    osRes = m2mb_os_free( pStruct );
    if ( osRes != M2MB_OS_SUCCESS )
      //...

  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
void azx_gnu_free(void *ptr);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Dynamic memory reallocation

  @details
    Changes the size of the memory block pointed to by ptr.

    The function may move the memory block to a new location (whose address is returned by the function).

    The content of the memory block is preserved up to the lesser of the new and old sizes, even if the block is moved to a new location. If the new size is larger, the value of the newly allocated portion is indeterminate.

    In case that ptr is a null pointer, the function behaves like malloc, assigning a new block of size bytes and returning a pointer to its beginning.

  @param [in]: ptr
    Pointer to a memory block previously allocated with malloc, calloc or realloc.

  @param [in]: size
    New size for the memory block, in bytes.

  @param [out]
    A pointer to the reallocated memory block, which may be either the same as ptr or a new location.

  @return
    M2MB_OS_SUCCESS in case of success
  @return
    others in case of error : see m2mb_os_types.h

  @note
	See: azx_gnu_malloc, azx_gnu_calloc, m2mb_os_calloc

  @b
    Example
  @code
	<C example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
void * azx_gnu_realloc(void * ptr, size_t size);


/**
  @brief
    exit process. remaps stdlib exit()

  @details
    quits the application

  @param [in]: status
    return code. dummy here



*/
void azx_gnu_exit(int status);

#ifdef __cplusplus
}
#endif

#endif /* HDR_AZX_GNU_STDLIB_H_ */

