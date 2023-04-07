/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 azx_gnu_stdio.h

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
 @author Fabio Pintus

 @date
 11/02/2020
 */

#ifndef HDR_AZX_GNU_STDIO_H_
#define HDR_AZX_GNU_STDIO_H_

#include <stdarg.h>
#include "m2mb_fs_stdio.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__ARMCLIB_VERSION) || defined(__MINGW32__)

#ifndef __off_t_defined
typedef long _off_t;
#endif

#ifndef __dev_t_defined
typedef short __dev_t;
#endif

#ifndef __uid_t_defined
typedef unsigned short __uid_t;
#endif
#ifndef __gid_t_defined
typedef unsigned short __gid_t;
#endif

#ifndef __off64_t_defined
typedef long long _off64_t;
#endif

#ifndef __CYGWIN__  /* which defines these types in it's own types.h. */
typedef _off_t  off_t;
#ifndef __MINGW32__
typedef __dev_t dev_t;
#endif
typedef __uid_t uid_t;
typedef __gid_t gid_t;
#endif

#endif

#undef stdin
#undef stdout
#undef stderr
#undef truncate

/* Global defines ================================================================================*/


#ifndef APP_NAME /*Emulator*/
#define	fopen		m2mb_fs_fopen
#define fread		m2mb_fs_fread
#define fwrite		m2mb_fs_fwrite
#define	fclose		m2mb_fs_fclose
#define	fputs		m2mb_fs_fputs
#define	fgets		m2mb_fs_fgets
#define	fflush		m2mb_fs_fflush
#undef fileno
#define fileno	m2mb_fs_fileno

#undef unlink
#define unlink  m2mb_fs_remove


#define FILE		M2MB_FILE_T


#undef clearerr
#define clearerr	azx_gnu_clearerr

//XXX: Warning! Magic numbers ahead
#define stdout (FILE*)(0x88888888)
#define stderr (FILE*)(0x99999999)
#define stdin (FILE*)(NULL)

#undef fputc
#define fputc 	m2mb_fs_fputc

#undef fgetc
#define fgetc 	m2mb_fs_fgetc

#define fseek		m2mb_fs_fseek
#define fseeko		m2mb_fs_fseek

#define rewind(a)	(void)fseek(a, 0L, SEEK_SET)

#define ftell		m2mb_fs_ftell
#define ftello		m2mb_fs_ftell

#define fdopen		azx_gnu_fdopen
#define	fprintf		azx_gnu_fprintf
#define	vfprintf	azx_gnu_vfprintf
#define truncate	azx_gnu_truncate
#undef feof
#define feof azx_gnu_feof


#undef printf
#define printf(a...) fprintf(stdout, a)

#undef vprintf
#define vprintf(f, a) vfprintf(stdout, f, a)

#define ungetc azx_gnu_ungetc

#undef ferror
#define ferror azx_gnu_ferror
#else  //emulator

#define fopen(x,y) (M2MB_FILE_T *)0xAAAAAAAA
#define fread(a,b,c,d) b*c
#define fwrite(a,b,c,d) b*c
#define fclose(x)
#define fputs(x,y)
#define fgets(x,y)
#define fflush(x)
#undef fileno
#define fileno(x)
#define unlink(x)
#define FILE M2MB_FILE_T

#define m2m_fputc(x,y)
#define m2m_fgetc(x,y)

#define fseek(a,b,c) 0
#define rewind(x)

#define fprintf(a,b,...)
#define truncate(x,y) 0
#endif
/* Function prototypes ====================================================================*/

/*-----------------------------------------------------------------------------------------------*/
/**

 @brief
 Prints data in a file stream.

 @details
 Prints on the defined stream (If stderr or stdout are passed, will print on defined UART or USB channel )

 @param [in] f: void* pointer
 @param [in] format: const char* pointer

 @return
 0 on SUCCESS
 -1 on FAILURE

 @note
 <Notes>

 @b
 Example
 @code
 <C code example>

 @endcode

 */
/*-----------------------------------------------------------------------------------------------*/
int azx_gnu_fprintf(void *f, const char *format, ...);

/**

 @brief
 Prints data in a file stream. Requires a variadic list

 @details
 Prints on the defined stream (If stderr or stdout are passed, will print on defined UART or USB channel )

 @param [in] f: void* pointer
 @param [in] format: const char* pointer
 @param [in] ap: variadic list retrieved with va_list()

 @return
 0 on SUCCESS
 -1 on FAILURE

 @note
 <Notes>

 @b
 Example
 @code
 <C code example>

 @endcode

 */
int azx_gnu_vfprintf(void *f, const char *format, va_list ap);
/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
 Truncate file.

 @details
 The function effect is that the regular file named by path will have a size which will be equal to length bytes.

 @param[in] path
 Name of file to be truncated.
 @param[in] length
 New size of the file.

 @return
 Upon successful completion returns 0. Otherwise, -1 is returned.

 @note
 <Notes>

 @b
 Example
 @code
 <C code example>
 @endcode
 */
/*-----------------------------------------------------------------------------------------------*/
int azx_gnu_truncate(const char* path, off_t length);

/*TODO*/
int azx_gnu_ungetc(void *f, int c);

int azx_gnu_ferror(void* f);
int azx_gnu_feof(void *f);

FILE* azx_gnu_fdopen(int, const char *);

void azx_gnu_clearerr(FILE* );
#ifdef __cplusplus
}
#endif

#endif /* HDR_AZX_GNU_STDIO_H_ */
