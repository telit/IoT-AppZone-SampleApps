/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
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

 @date
 11/02/2020
 */

#ifndef HDR_AZX_GNU_STDIO_H_
#define HDR_AZX_GNU_STDIO_H_

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

//XXX: Warning! Magic numbers ahead //TODO add doxygen description
#define stdout (void*)(0x88888888)
#define stderr (void*)(0x99999999)

#ifndef APP_NAME /*Emulator*/
#define	fopen		m2mb_fs_fopen
#define fread		m2mb_fs_fread
#define fwrite		m2mb_fs_fwrite
#define	fclose		m2mb_fs_fclose
#define	fputs		m2mb_fs_fputs
#define	fgets		m2mb_fs_fgets
#define	fflush		m2mb_fs_fflush
#undef fileno
#define fileno		m2mb_fs_fileno
#define unlink		m2mb_fs_remove
#define FILE		M2MB_FILE_T

#define m2m_fputc 	m2mb_fs_fputc
#define m2m_fgetc 	m2mb_fs_fgetc

#define fseek		m2mb_fs_fseek
#define rewind(a)	(void)fseek(a, 0L, SEEK_SET)

#define	fprintf		azx_gnu_fprintf
#define truncate	azx_gnu_truncate
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
 Prints a autput

 @details
 Prints on the defined stream (UART or USB channel)

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

#ifdef __cplusplus
}
#endif

#endif /* HDR_AZX_GNU_STDIO_H_ */
