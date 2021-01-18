/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 	 azx_zlib.h

 @brief
 	 zlib definition

 @details
 	 Porting from zlib to azx

 @note
	The 'zlib' compression library provides in-memory compression and
	decompression functions, including integrity checks of the uncompressed data.
	This version of the library supports only one compression method (deflation)
	but other algorithms will be added later and will have the same stream
	interface.

	Compression can be done in a single step if the buffers are large enough,
	or can be done by repeated calls of the compression function.  In the latter
	case, the application must provide more input and/or consume the output
	(providing more output space) before each call.

	The compressed data format used by default by the in-memory functions is
	the zlib format, which is a zlib wrapper documented in RFC 1950, wrapped
	around a deflate stream, which is itself documented in RFC 1951.

	The library also supports reading and writing files in gzip (.gz) format
	with an interface similar to that of stdio using the functions that start
	with "gz".  The gzip format is different from the zlib format.  gzip is a
	gzip wrapper, documented in RFC 1952, wrapped around a deflate stream.

	This library can optionally read and write gzip and raw deflate streams in
	memory as well.

	The zlib format was designed to be compact and fast for use in memory
	and on communications channels.  The gzip format was designed for single-
	file compression on file systems, has a larger header than zlib to maintain
	directory information, and uses a different, slower check method than zlib.

	The library does not install any signal handler.  The decoder checks
	the consistency of the compressed data, so the library should never crash
	even in the case of corrupted input.
	\n
 	Dependencies:
		zlib.h

 @author Norman Argiolas

 @date
 	 13/02/2020
 */

#ifndef HDR_AZX_ZLIB_ZLIB_H_
#define HDR_AZX_ZLIB_ZLIB_H_

/* Include files =============================================================*/
#include "zlib.h"


/* Local defines =============================================================*/

/**\name gunzip status struct
 * \brief  	Information returned by azx_zlib_gunzip function.
 *  @{ */
typedef enum
{
	AZX_GUNZIP_SUCCESS = 0,
	AZX_GUNZIP_MEMORY_ERROR,
	AZX_GUNZIP_INTERNAL_ERROR,
	AZX_GUNZIP_GENERIC_ERROR,
	AZX_GUNZIP_DATA_ERROR,
	AZX_GUNZIP_TRAILING_GARBAGE_ERROR,
	AZX_GUNZIP_INVALID_NAME_FILE,
	AZX_GUNZIP_CANNOT_OPEN_FILE,
	AZX_GUNZIP_CANNOT_CREATE_FILE,
	AZX_GUNZIP_CANNOT_READ_FILE,
	AZX_GUNZIP_CANNOT_WRITE_FILE,
	AZX_GUNZIP_UNEXPECTED_END_FILE

} AZX_ZLIB_GUNZIP_STATUS_E;

/** @} */

/** \addtogroup  zlibUsage
 @{ */

/**\name zlib stream struct
 * \brief  	stream information passed to and from zlib routines.
 *  @{ */
#define azx_zlib_z_stream  z_stream
#define azx_zlib_z_streamp z_streamp
/** @} */


/**\name zlib header struct
 * \brief  	gzip header information passed to and from zlib routines.  See RFC 1952
  	  	  	for more details on the meanings of these fields.
 *  @{ */
#define azx_zlib_gz_header  gz_header
#define azx_zlib_gz_headerp gz_headerp
/** @} */


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	Gets lib version
 @details
	For compatibility with versions < 1.0.2
 @note
 	See azx_zlib_zlibVersion
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_zlib_version zlib_version


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	Gets lib version

 @details
	The application can compare zlibVersion and ZLIB_VERSION for consistency.
   	If the first character differs, the library code actually used is not
    compatible with the zlib.h header file used by the application.  This check
    is automatically made by deflateInit and inflateInit.

 @note
 	For compatibility with versions < 1.0.2

 @return
	Lib version

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_zlibVersion zlibVersion


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflate function

 @details

 @param[in] strm
 	 z_streamp strm
 @param[in] flush
 	 int flush

 @note

 @return
	Z_OK if success

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflate deflate


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateEnd function

 @details

 @param[in] strm
 	 z_streamp strm

 @note

 @return
	Z_OK if success, BUSY_STATE or Z_DATA_ERROR otherwise

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateEnd deflateEnd


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
   inflate function

 @details
   inflate() uses a state machine to process as much input data and generate as
   much output data as possible before returning.

 @param[in] strm
 	 z_streamp strm
 @param[in] flush
 	 int flush

 @note
	inflate() uses a state machine to process as much input data and generate as
   much output data as possible before returning.  The state machine is
   structured roughly as follows:

    for (;;) switch (state) {
    ...
    case STATEn:
        if (not enough input data or output space to make progress)
            return;
        ... make progress ...
        state = STATEm;
        break;
    ...
    }

   so when inflate() is called again, the same case is attempted again, and
   if the appropriate resources are provided, the machine proceeds to the
   next state.  The NEEDBITS() macro is usually the way the state evaluates
   whether it can proceed or should return.  NEEDBITS() does the return if
   the requested bits are not available.  The typical use of the BITS macros
   is:

        NEEDBITS(n);
        ... do something with BITS(n) ...
        DROPBITS(n);

   where NEEDBITS(n) either returns from inflate() if there isn't enough
   input left to load n bits into the accumulator, or it continues.  BITS(n)
   gives the low n bits in the accumulator.  When done, DROPBITS(n) drops
   the low n bits off the accumulator.  INITBITS() clears the accumulator
   and sets the number of available bits to zero.  BYTEBITS() discards just
   enough bits to put the accumulator on a byte boundary.  After BYTEBITS()
   and a NEEDBITS(8), then BITS(8) would return the next byte in the stream.

   NEEDBITS(n) uses PULLBYTE() to get an available byte of input, or to return
   if there is no input available.  The decoding of variable length codes uses
   PULLBYTE() directly in order to pull just enough bytes to decode the next
   code, and no more.

   Some states loop until they get enough input, making sure that enough
   state information is maintained to continue the loop where it left off
   if NEEDBITS() returns in the loop.  For example, want, need, and keep
   would all have to actually be part of the saved state in case NEEDBITS()
   returns:

    case STATEw:
        while (want < need) {
            NEEDBITS(n);
            keep[want++] = BITS(n);
            DROPBITS(n);
        }
        state = STATEx;
    case STATEx:

   As shown above, if the next state is also the next case, then the break
   is omitted.

   A state may also return if there is not enough output space available to
   complete that state.  Those states are copying stored data, writing a
   literal byte, and copying a matching string.

   When returning, a "goto inf_leave" is used to update the total counters,
   update the check value, and determine whether any progress has been made
   during that inflate() call in order to return the proper return code.
   Progress is defined as a change in either strm->avail_in or strm->avail_out.
   When there is a window, goto inf_leave will update the window with the last
   output written.  If a goto inf_leave occurs in the middle of decompression
   and there is no window currently, goto inf_leave will create one and copy
   output to the window for the next call of inflate().

   In this implementation, the flush parameter of inflate() only affects the
   return code (per zlib.h).  inflate() always writes as much as possible to
   strm->next_out, given the space available and the provided input--the effect
   documented in zlib.h of Z_SYNC_FLUSH.  Furthermore, inflate() always defers
   the allocation of and copying into a sliding window until necessary, which
   provides the effect documented in zlib.h for Z_FINISH when the entire input
   stream available.  So the only thing the flush parameter actually does is:
   when flush is set to Z_FINISH, inflate() cannot return Z_OK.  Instead it
   will return Z_BUF_ERROR if it has not reached the end of the stream.

 @return
	Return from inflate(), updating the total counts and the check value.
    If there was no progress during the inflate() call, return a buffer
    error.  Call updatewindow() to create and/or update the window state.
    Note: a memory error from inflate() is non-recoverable.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflate inflate


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateEnd function

 @details

 @param[in] strm
 	 z_streamp strm

 @note

 @return
	Z_OK if success, Z_STREAM_ERROR otherwise

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateEnd inflateEnd


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateSetDictionary function

 @details
 	 Sets dictionary

 @param[in] strm
 	 z_streamp strm
 @param[in] dictionary
 	 const Bytef *dictionary
 @param[in] dictLength
 	 uInt  dictLength

 @note

 @return
	Z_OK if success, Z_STREAM_ERROR otherwise

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateSetDictionary deflateSetDictionary


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateGetDictionary function

 @details
 	 Gets dictionary

 @param[in] strm
 	 z_streamp strm
 @param[in] dictionary
 	 const Bytef *dictionary
 @param[in] dictLength
 	 uInt  dictLength

 @note

 @return
	Z_OK if success, Z_STREAM_ERROR otherwise

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateGetDictionary deflateGetDictionary


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateCopy function

 @details
 	Sets the destination stream as a complete copy of the source stream.

 @param[in] dest
 	 z_streamp dest
 @param[in] source
 	 z_streamp source

 @note
   This function can be useful when several compression strategies will be
   tried, for example when there are several ways of pre-processing the input
   data with a filter.  The streams that will be discarded should then be freed
   by calling deflateEnd.  Note that deflateCopy duplicates the internal
   compression state which can be quite large, so this strategy is slow and can
   consume lots of memory.

 @return
	deflateCopy returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_STREAM_ERROR if the source stream state was inconsistent
   (such as zalloc being Z_NULL).  msg is left unchanged in both source and
   destination.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateCopy deflateCopy


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateReset function

 @details
 	This function is equivalent to deflateEnd followed by deflateInit.

 @param[in] dest
 	 z_streamp dest
 @param[in] source
 	 z_streamp source

 @note
	This function is equivalent to deflateEnd followed by deflateInit, but
   does not free and reallocate the internal compression state.  The stream
   will leave the compression level and any other attributes that may have been
   set unchanged.

 @return
	deflateReset returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being Z_NULL).

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateReset deflateReset


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateParams function

 @details
 	Dynamically update the compression level and compression strategy.

 @param[in] strm
 	 z_streamp strm
 @param[in] level
 	 int level
 @param[in] strategy
 	 int strategy

 @note
     Dynamically update the compression level and compression strategy.  The
   interpretation of level and strategy is as in deflateInit2().  This can be
   used to switch between compression and straight copy of the input data, or
   to switch to a different kind of input data requiring a different strategy.
   If the compression approach (which is a function of the level) or the
   strategy is changed, and if any input has been consumed in a previous
   deflate() call, then the input available so far is compressed with the old
   level and strategy using deflate(strm, Z_BLOCK).  There are three approaches
   for the compression levels 0, 1..3, and 4..9 respectively.  The new level
   and strategy will take effect at the next call of deflate().

     If a deflate(strm, Z_BLOCK) is performed by deflateParams(), and it does
   not have enough output space to complete, then the parameter change will not
   take effect.  In this case, deflateParams() can be called again with the
   same parameters and more output space to try again.

     In order to assure a change in the parameters on the first try, the
   deflate stream should be flushed using deflate() with Z_BLOCK or other flush
   request until strm.avail_out is not zero, before calling deflateParams().
   Then no more input data should be provided before the deflateParams() call.
   If this is done, the old level and strategy will be applied to the data
   compressed before deflateParams(), and the new level and strategy will be
   applied to the the data compressed after deflateParams().

 @return
	deflateParams returns Z_OK on success, Z_STREAM_ERROR if the source stream
   state was inconsistent or if a parameter was invalid, or Z_BUF_ERROR if
   there was not enough output space to complete the compression of the
   available input data before a change in the strategy or approach.  Note that
   in the case of a Z_BUF_ERROR, the parameters are not changed.  A return
   value of Z_BUF_ERROR is not fatal, in which case deflateParams() can be
   retried with more output space.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateParams deflateParams


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateTune function

 @details
 	Fine tune deflate's internal compression parameters.

 @param[in] strm
 	 z_streamp strm

 @param[in] good_length
 	  int good_length

 @param[in] max_lazy
 	 int max_lazy

 @param[in] nice_length
 	 int nice_length

 @param[in] max_chain
 	 int max_chain


 @note
	Fine tune deflate's internal compression parameters.  This should only be
   used by someone who understands the algorithm used by zlib's deflate for
   searching for the best matching string, and even then only by the most
   fanatic optimizer trying to squeeze out the last compressed bit for their
   specific input data.  Read the deflate.c source code for the meaning of the
   max_lazy, good_length, nice_length, and max_chain parameters.

 @return
	deflateTune() can be called after deflateInit() or deflateInit2(), and
   returns Z_OK on success, or Z_STREAM_ERROR for an invalid deflate stream.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateTune deflateTune


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateBound function

 @details
  This function could be more sophisticated to provide closer upper bounds for
  every combination of windowBits and memLevel. But even the conservative
  upper bound of about 14% expansion does not seem onerous for output buffer
  allocation.

 @param[in] strm
 	 z_streamp strm

 @param[in] good_length
 	  uLong sourceLen

 @note
  For the default windowBits of 15 and memLevel of 8, this function returns
  a close to exact, as well as small, upper bound on the compressed size.
  They are coded as constants here for a reason--if the #define's are
  changed, then this function needs to be changed as well.  The return
  value for 15 and 8 only works for those exact settings.

  For any setting other than those defaults for windowBits and memLevel,
  the value returned is a conservative worst case for the maximum expansion
  resulting from using fixed blocks instead of stored blocks, which deflate
  can emit on compressed data for some combinations of the parameters.

 @return
 	 The return value for 15 and 8 only works for those exact settings.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateBound deflateBound


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflatePending function

 @details

 @param[in] strm
 	 z_streamp strm

 @param[in] pending
 	  unsigned *pending

 @param[in] bits
 	 int *bits

 @note
	deflatePending() returns the number of bytes and bits of output that have
   been generated, but not yet provided in the available output.  The bytes not
   provided would be due to the available output space having being consumed.
   The number of bits of output not provided are between 0 and 7, where they
   await more bits to join them in order to fill out a full byte.  If pending
   or bits are Z_NULL, then those values are not set.

 @return
   deflatePending returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflatePending deflatePending


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflatePrime function

 @details

 @param[in] strm
 	 z_streamp strm

 @param[in] pending
 	  unsigned *pending

 @param[in] bits
 	 int *bits

 @note
     deflatePrime() inserts bits in the deflate output stream.  The intent
   is that this function is used to start off the deflate output with the bits
   leftover from a previous deflate stream when appending to it.  As such, this
   function can only be used for raw deflate, and must be used before the first
   deflate() call after a deflateInit2() or deflateReset().  bits must be less
   than or equal to 16, and that many of the least significant bits of value
   will be inserted in the output.

 @return
     deflatePrime returns Z_OK if success, Z_BUF_ERROR if there was not enough
   room in the internal buffer to insert the bits, or Z_STREAM_ERROR if the
   source stream state was inconsistent.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflatePrime deflatePrime


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateSetHeader function

 @details

 @param[in] strm
 	 z_streamp strm

 @param[in] head
	gz_headerp head

 @note
     deflateSetHeader() provides gzip header information for when a gzip
   stream is requested by deflateInit2().  deflateSetHeader() may be called
   after deflateInit2() or deflateReset() and before the first call of
   deflate().  The text, time, os, extra field, name, and comment information
   in the provided gz_header structure are written to the gzip header (xflag is
   ignored -- the extra flags are set according to the compression level).  The
   caller must assure that, if not Z_NULL, name and comment are terminated with
   a zero byte, and that if extra is not Z_NULL, that extra_len bytes are
   available there.  If hcrc is true, a gzip header crc is included.  Note that
   the current versions of the command-line version of gzip (up through version
   1.3.x) do not support header crc's, and will report that it is a "multi-part
   gzip file" and give up.

     If deflateSetHeader is not used, the default gzip header has text false,
   the time set to zero, and os set to 255, with no extra, name, or comment
   fields.  The gzip header is returned to the default state by deflateReset().

 @return
     deflateSetHeader returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateSetHeader deflateSetHeader


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateSetDictionary function

 @details

 @param[in] strm
 	 z_streamp strm

 @param[in] dictionary
	const Bytef *dictionary

  @param[in] dictLength
	uInt  dictLength

 @note


 @return
	Z_OK, Z_MEM_ERROR, Z_DATA_ERROR, Z_STREAM_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateSetDictionary inflateSetDictionary


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateGetDictionary function

 @details

 @param[in] strm
 	 z_streamp strm

 @param[in] dictionary
	const Bytef *dictionary

  @param[in] dictLength
	uInt  dictLength

 @note


 @return
	Z_OK, Z_STREAM_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateGetDictionary inflateGetDictionary


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateSync function

 @details

 @param[in] strm
 	 z_streamp strm

 @note


 @return
	Z_OK, Z_DATA_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateSync inflateSync


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateCopy function

 @details

 @param[in] dest
 	 z_streamp dest

 @param[in] source
 	 z_streamp source

 @note


 @return
	Z_OK, Z_STREAM_ERROR, Z_MEM_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateCopy inflateCopy


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateReset function

 @details

 @param[in] strm
 	 z_streamp strm

 @note


 @return
	if (inflateStateCheck(strm)) return Z_STREAM_ERROR
	otherwise return inflateResetKeep(strm)
	if succes Z_OK

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateReset inflateReset


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateReset2 function

 @details

 @param[in] strm
 	 z_streamp strm

 @param[in] windowBits
 	 int windowBits

 @note


 @return
	if (inflateStateCheck(strm)) return Z_STREAM_ERROR
	otherwise return inflateResetKeep(strm)
	if succes Z_OK

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateReset2 inflateReset2


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflatePrime function

 @details

 @param[in] strm
 	 z_streamp strm

 @param[in] bits
 	 int bits

 @param[in] value
 	 int value

 @note


 @return
	Z_OK, Z_STREAM_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflatePrime inflatePrime


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateMark function

 @details

 @param[in] strm
 	 z_streamp strm

 @note

 @return
	return the long ZEXPORT infalte mark value

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateMark inflateMark


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateGetHeader function

 @details

 @param[in] strm
 	 z_streamp strm

 @param[in] head
 	 gz_headerp head

 @note

 @return
	Z_OK, Z_STREAM_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateGetHeader inflateGetHeader



/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateBack function

 @details
	inflateBack() does a raw inflate with a single call using a call-back
	interface for input and output.  This is potentially more efficient than
	inflate() for file i/o applications, in that it avoids copying between the
	output and the sliding window by simply making the window itself the output
	buffer.  inflate() can be faster on modern CPUs when used with large
	buffers.  inflateBack() trusts the application to not change the output
	buffer passed by the output function, at least until inflateBack() returns.

 @param[in] strm
 	 z_streamp strm

 @param[in] in
 	 in_func in

 @param[in] in_desc
 	 void FAR *in_desc

 @param[in] out
 	 out_func out

 @param[in] out_desc
 	 void FAR *out_desc


 @note
   inflateBackInit() must be called first to allocate the internal state
   and to initialize the state with the user-provided window buffer.
   inflateBack() may then be used multiple times to inflate a complete, raw
   deflate stream with each call.  inflateBackEnd() is then called to free the
   allocated state.

     A raw deflate stream is one with no zlib or gzip header or trailer.
   This routine would normally be used in a utility that reads zip or gzip
   files and writes out uncompressed files.  The utility would decode the
   header and process the trailer on its own, hence this routine expects only
   the raw deflate stream to decompress.  This is different from the default
   behavior of inflate(), which expects a zlib header and trailer around the
   deflate stream.

     inflateBack() uses two subroutines supplied by the caller that are then
   called by inflateBack() for input and output.  inflateBack() calls those
   routines until it reads a complete deflate stream and writes out all of the
   uncompressed data, or until it encounters an error.  The function's
   parameters and return types are defined above in the in_func and out_func
   typedefs.  inflateBack() will call in(in_desc, &buf) which should return the
   number of bytes of provided input, and a pointer to that input in buf.  If
   there is no input available, in() must return zero -- buf is ignored in that
   case -- and inflateBack() will return a buffer error.  inflateBack() will
   call out(out_desc, buf, len) to write the uncompressed data buf[0..len-1].
   out() should return zero on success, or non-zero on failure.  If out()
   returns non-zero, inflateBack() will return with an error.  Neither in() nor
   out() are permitted to change the contents of the window provided to
   inflateBackInit(), which is also the buffer that out() uses to write from.
   The length written by out() will be at most the window size.  Any non-zero
   amount of input may be provided by in().

     For convenience, inflateBack() can be provided input on the first call by
   setting strm->next_in and strm->avail_in.  If that input is exhausted, then
   in() will be called.  Therefore strm->next_in must be initialized before
   calling inflateBack().  If strm->next_in is Z_NULL, then in() will be called
   immediately for input.  If strm->next_in is not Z_NULL, then strm->avail_in
   must also be initialized, and then if strm->avail_in is not zero, input will
   initially be taken from strm->next_in[0 ..  strm->avail_in - 1].

     The in_desc and out_desc parameters of inflateBack() is passed as the
   first parameter of in() and out() respectively when they are called.  These
   descriptors can be optionally used to pass any information that the caller-
   supplied in() and out() functions need to do their job.

     On return, inflateBack() will set strm->next_in and strm->avail_in to
   pass back any unused input that was provided by the last in() call.  The
   return values of inflateBack() can be Z_STREAM_END on success, Z_BUF_ERROR
   if in() or out() returned an error, Z_DATA_ERROR if there was a format error
   in the deflate stream (in which case strm->msg is set to indicate the nature
   of the error), or Z_STREAM_ERROR if the stream was not properly initialized.
   In the case of Z_BUF_ERROR, an input or output error can be distinguished
   using strm->next_in which will be Z_NULL only if in() returned an error.  If
   strm->next_in is not Z_NULL, then the Z_BUF_ERROR was due to out() returning
   non-zero.  (in() will always be called before out(), so strm->next_in is
   assured to be defined if out() returns non-zero.)  Note that inflateBack()
   cannot return Z_OK.

 @return
	ZEXTERN int ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateBack inflateBack


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateBackEnd function

 @details
 	 All memory allocated by inflateBackInit() is freed.

 @param[in] strm
 	 z_streamp strm

 @param[in] head
 	 gz_headerp head

 @note

 @return
	inflateBackEnd() returns Z_OK on success, or Z_STREAM_ERROR if the stream
    state was inconsistent.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateBackEnd inflateBackEnd


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	zlibCompileFlags function

 @details

 @note

 @return
	return flags

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_zlibCompileFlags zlibCompileFlags

#ifndef Z_SOLO

/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	compress function

 @details
 	  Compresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer.  Upon entry, destLen is the total size
   of the destination buffer, which must be at least the value returned by
   compressBound(sourceLen).  Upon exit, destLen is the actual size of the
   compressed data.  compress() is equivalent to compress2() with a level
   parameter of Z_DEFAULT_COMPRESSION.

 @note

 @param[in] dest
 	 Bytef *dest

 @param[in] destLen
 	 uLongf *destLen

 @param[in] source
 	 const Bytef *source

 @param[in] sourceLen
 	 uLong sourceLen

 @return
	compress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_compress compress


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	compress2 function

 @details
   Compresses the source buffer into the destination buffer.  The level
   parameter has the same meaning as in deflateInit.  sourceLen is the byte
   length of the source buffer.  Upon entry, destLen is the total size of the
   destination buffer, which must be at least the value returned by
   compressBound(sourceLen).  Upon exit, destLen is the actual size of the
   compressed data.

 @note

 @param[in] dest
 	 Bytef *dest

 @param[in] destLen
 	 uLongf *destLen

 @param[in] source
 	 const Bytef *source

 @param[in] sourceLen
 	 uLong sourceLen

 @param[in] level
 	 int level

 @return
   compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer,
   Z_STREAM_ERROR if the level parameter is invalid.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_compress2 compress2


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	compressBound function

 @details

 @note
   If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.

  @param[in] sourceLen
 	 uLong sourceLen

 @return
	return uLong ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_compressBound compressBound


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	uncompress function

 @details
   Decompresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer.  Upon entry, destLen is the total size
   of the destination buffer, which must be large enough to hold the entire
   uncompressed data.  (The size of the uncompressed data must have been saved
   previously by the compressor and transmitted to the decompressor by some
   mechanism outside the scope of this compression library.) Upon exit, destLen
   is the actual size of the uncompressed data.

 @note

 @param[in] dest
 	 Bytef *dest

 @param[in] destLen
 	 uLongf *destLen

 @param[in] source
 	 const Bytef *source

 @param[in] sourceLen
 	 uLong sourceLen

 @return
  uncompress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer, or Z_DATA_ERROR if the input data was corrupted or incomplete.  In
   the case where there is not enough room, uncompress() will fill the output
   buffer with the uncompressed data up to that point.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_uncompress uncompress


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	uncompress2 function

 @details
   Same as uncompress, except that sourceLen is a pointer, where the
   length of the source is *sourceLen.  On return, *sourceLen is the number of
   source bytes consumed.

 @note
	see azx_zlib_uncompress
  	This library supports reading and writing files in gzip (.gz) format with
	an interface similar to that of stdio, using the functions that start with
	"gz".  The gzip format is different from the zlib format.  gzip is a gzip
	wrapper, documented in RFC 1952, wrapped around a deflate stream.

 @param[in] dest
 	 Bytef *dest

 @param[in] destLen
 	 uLongf *destLen

 @param[in] source
 	 const Bytef *source

 @param[in] sourceLen
 	 uLong sourceLen

 @return
	return flags

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_uncompress2 uncompress2


/*-----------------------------------------------------------------------------------------------*/
/**\name zlib gzFile struct
 * \brief  	stream information passed to and from zlib routines.
 *  @{ */
	#define azx_zlib_gzFile gzFile
/** @} */
/*-----------------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzdopen function

 @details

 @note
 	see zlib.h

 @param[in] fd
    int fd

 @param[in] mode
 	const char *mode

 @return
	gzFile ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzdopen gzdopen


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzbuffer function

 @details

 @note
 	see zlib.h

 @param[in] fd
    int fd

 @param[in] mode
 	const char *mode

 @return
	gzFile ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzbuffer gzbuffer


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzsetparams function

 @details

 @note
  	 see zlib.h

 @param[in] file
 	 gzFile file

 @param[in] level
    int level

 @param[in] strategy
 	int strategy

 @return
	int ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzsetparams gzsetparams


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzread function

 @details

 @note
	see zlib.h

 @param[in] file
 	 gzFile file

 @param[in] buf
    voidp buf

 @param[in] len
 	unsigned int len

 @return
	int ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzread gzread


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzfread function

 @details

 @note
	see zlib.h

 @param[in] buf
 	 voidp buf

 @param[in] size
     z_size_t size

 @param[in] nitems
 	z_size_t nitems

 @param[in] file
 	 gzFile file

 @return
	z_size_t ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzfread gzfread


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzwrite function

 @details

 @note
	see zlib.h

 @param[in] file
 	 gzFile file

 @param[in] buf
    voidp buf

 @param[in] len
 	unsigned int len

 @return
	int ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzwrite gzwrite


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzfwrite function

 @details

 @note
	see zlib.h

 @param[in] buf
    voidp buf

 @param[in] size
 	z_size_t size

 @param[in] nitems
 	z_size_t nitems

 @param[in] file
 	gzFile file

 @return
	z_size_t ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzfwrite gzfwrite


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzprintf function

 @details

 @note

  @param[in] file
 	 gzFile file

  @param[in] format
 	 const char *format

  @param[in] ...
 	 variables parameters


 @return
	return int ZEXPORTVA

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzprintf gzprintf


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzputs function

 @details

 @note
	see zlib.h

 @param[in] buf
    voidp buf

 @param[in] str
 	const char *str

 @return
	int ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzputs gzputs


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzgets function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @param[in] buf
 	char *buf

 @param[in] len
 	int len

 @return
	char * ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzgets gzgets


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzputc function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @param[in] c
 	int c

 @return
	char * ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzputc gzputc


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzgetc function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @return
	char ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzgetc gzgetc


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzungetc function

 @details

 @note
	see zlib.h

 @param[in] c
 	int c

 @param[in] file
    gzFile file

 @return
	char ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzungetc gzungetc


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzflush function

 @details

 @note

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @param[in] c
 	int c

 @return
	char ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzflush gzflush


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzrewind function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @return
	char ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzrewind gzrewind


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzeof function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @return
	char ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzeof gzeof


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzdirect function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @return
	char ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzdirect gzdirect


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzclose function

 @details

 @note
   see zlib.h
   gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines.

 @param[in] file
    gzFile file

 @return
	return int ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzclose gzclose


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzclose_r function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @return
	char ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzclose_r gzclose_r


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzclose_w function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @return
	char ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzclose_w gzclose_w


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	zlibCompileFlags function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @param[in] errnum
    int *errnum

 @return
	return const char * ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzerror gzerror


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	zlibCompileFlags function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @return
	return void ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
	#define azx_zlib_gzclearerr gzclearerr

#endif /* !Z_SOLO */

/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	adler32 function

 @details

 @note

 @param[in] adler
    uLong adler

 @param[in] buf
    const Bytef *buf

 @param[in] len
    uInt len

 @return
	return uLong ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_adler32 adler32


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	adler32_z function

 @details

 @note

 @param[in] adler
    uLong adler

 @param[in] buf
    const Bytef *buf

 @return
	return uLong ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_adler32_z adler32_z


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	crc32 function

 @details

 @note

 @param[in] crc
    uLong crc

 @param[in] buf
    const Bytef *buf

 @param[in] len
    uInt len

 @return
	return crc32_z(crc, buf, len);

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_crc32 crc32


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	crc32_z function

 @details

 @note

 @param[in] crc
    uLong crc

 @param[in] buf
    const Bytef *buf

 @param[in] len
    z_size_t len

 @return
	return unsigned long ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_crc32_z crc32_z


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateInit_ function

 @details

 @note

 @param[in] strm
    z_streamp strm

 @param[in] level
    int level

 @param[in] version
    const char *version

 @param[in] stream_size
 	 int stream_size

 @return
	return uLong ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateInit_ deflateInit_


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateInit_ function

 @details

 @note

 @param[in] strm
    z_streamp strm

 @param[in] version
    const char *version

 @param[in] stream_size
 	 int stream_size

 @return
	return uLong ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateInit_ inflateInit_


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateInit2_ function

 @details

 @note

 @param[in] strm
    z_streamp strm

 @param[in] level
    int level

 @param[in] version
    const char *version

 @param[in] stream_size
 	 int stream_size

 @return
	return ZEXTERN int ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateInit2_ deflateInit2_


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateInit2_ function

 @details

 @note

 @param[in] strm
    z_streamp strm

 @param[in] windowBits
    int windowBits

 @param[in] version
    const char* version

 @param[in] stream_size
 	 int stream_size

 @return
	return int ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateInit2_ inflateInit2_


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateBackInit_ function

 @details

 @note

 @param[in] strm
    z_streamp strm

 @param[in] windowBits
    int windowBits

 @param[in] window
	unsigned char FAR* window

 @param[in] version
	const char *version

 @param[in] stream_size
	int stream_size

 @return
	return uLong ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateBackInit_ inflateBackInit_

#ifdef Z_PREFIX_SET

	#define azx_zlib_z_deflateInit(strm, level) z_deflateInit(strm, level)
	#define azx_zlib_z_inflateInit(strm) z_inflateInit(strm)
	#define azx_zlib_z_deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
				  z_deflateInit2(strm, level, method, windowBits, memLevel, strategy)
	#define azx_zlib_z_inflateInit2(strm, windowBits) z_inflateInit2(strm, windowBits)
	#define azx_zlib_ z_inflateBackInit(strm, windowBits, window) \
			  	  z_inflateBackInit(strm, windowBits, window)

#else

/**\name zlib macros
 * \brief  	deflateInit and inflateInit are macros to allow checking the zlib version and the compiler's view of z_stream.
   @{ */
	#define azx_zlib_deflateInit(strm, level) deflateInit(strm, level)
	#define azx_zlib_inflateInit(strm) inflateInit(strm)
	#define azx_zlib_deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
				  deflateInit2(strm, level, method, windowBits, memLevel, strategy)
	#define azx_zlib_inflateInit2(strm, windowBits) inflateInit2(strm, windowBits)
	#define azx_zlib_inflateBackInit(strm, windowBits, window) inflateBackInit(strm, windowBits, window)
/** @} */
#endif

#ifndef Z_SOLO

/**\name zlib struct gzFile_s
 * \brief  	This abbreviated structure exposes just enough for the gzgetc() macro.
 *  @{ */
	#define azx_zlib_gzFile_s gzFile_s
/** @} */

	#ifdef Z_PREFIX_SET
		#define azx_zlib_z_gzgetc(g) z_gzgetc(g)
	#else


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzgetc function

 @details
  gzgetc() macro and its supporting function and exposed data structure.  Note
  that the real internal state is much larger than the exposed structure.
  This abbreviated structure exposes just enough for the gzgetc() macro.  The
  user should not mess with these exposed elements, since their names or
  behavior could change in the future, perhaps even capriciously.  They can
  only be used by the gzgetc() macro.  You have been warned.

 @note

 @param[in] file
	gzFile file

 @return
	ZEXTERN int ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
		#define azx_zlib_gzgetc_m(g) gzgetc(g)


	#endif
	#ifdef Z_LARGE64
		#define azx_zlib_gzopen64 			gzopen64
		#define azx_zlib_gzseek64 			gzseek64
		#define azx_zlib_gztell64 			gztell64
		#define azx_zlib_gzoffset64 		gzoffset64
		#define azx_zlib_adler32_combine64 	adler32_combine64
		#define azx_zlib_crc32_combine64 	crc32_combine64
	#endif

	#if !defined(ZLIB_INTERNAL) && defined(Z_WANT64)
		#ifdef Z_PREFIX_SET
			#define azx_zlib_z_gzopen64 			z_gzopen64
			#define azx_zlib_z_gzseek64 			z_gzseek64
			#define azx_zlib_z_gztell64 			z_gztell64
			#define azx_zlib_z_gzoffset64 			z_gzoffset64
			#define azx_zlib_z_adler32_combine64 	z_adler32_combine64
			#define azx_zlib_z_crc32_combine64 		z_crc32_combine64
		#else
			#define azx_zlib_gzopen64 			gzopen64
			#define azx_zlib_gzseek64 			gzseek64
			#define azx_zlib_gztell64 			gztell64
			#define azx_zlib_gzoffset64 		gzoffset64
			#define azx_zlib_adler32_combine64  adler32_combine64
			#define azx_zlib_crc32_combine64 	crc32_combine64
		#endif
		#ifndef Z_LARGE64
			#define azx_zlib_gzopen64 			gzopen64
			#define azx_zlib_gzseek64 			gzseek64
			#define azx_zlib_gztell64 			gztell64
			#define azx_zlib_gzoffset64 		gzoffset64
			#define azx_zlib_adler32_combine64 	adler32_combine64
			#define azx_zlib_crc32_combine64 	crc32_combine64
		#endif
	#else

/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzopen function

 @details

 @note
	see zlib.h

 @param[in] path
    const char* path,

 @param[in] mode
    const char* mode

 @return
	return gzFile ZEXPOR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
		#define azx_zlib_gzopen 			gzopen


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzseek function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @param[in] offset
    z_off_t offset

 @param[in] whence
    int whence

 @return
	return z_off_t ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
		#define azx_zlib_gzseek 			gzseek


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gztell function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @return
	return z_off_t ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
		#define azx_zlib_gztell 			gztell


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzoffset function

 @details

 @note
	see zlib.h

 @param[in] file
    gzFile file

 @return
	return z_off_t ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
		#define azx_zlib_gzoffset 			gzoffset


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	adler32_combine function

 @details

 @note

 @param[in] adler1
    uLong adler1

 @param[in] adler2
    uLong adler2

 @param[in] len2
    z_off_t len2

 @return
	returnadler32_combine_(adler1, adler2, len2)

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
		#define azx_zlib_adler32_combine 	adler32_combine


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	crc32_combine function

 @details

 @note

 @param[in] crc1
    uLong crc1

 @param[in] crc2
     uLong crc2

 @param[in] len2
     z_off_t len2

 @return
	return crc32_combine_(crc1, crc2, len2)

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
		#define azx_zlib_crc32_combine 		crc32_combine

	#endif

#else /* Z_SOLO */

	#define azx_zlib_adler32_combine 	adler32_combine
	#define azx_zlib_crc32_combine	crc32_combine

#endif /* !Z_SOLO */


/* undocumented functions */


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	zError function

 @details
 	 exported to allow conversion of error code to string for compress() and uncompress()

 @note

 @param[in] err
    int err

 @return
	return ERR_MSG(err)

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_zError 			zError


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateSyncPoint function

 @details

 @note

 @param[in] strm
	z_streamp strm

 @return
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateSyncPoint 	inflateSyncPoint


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	get_crc_table function

 @details

 @note
 	This function can be used by asm versions of crc32()

 @return
	const z_crc_t FAR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_get_crc_table 		get_crc_table


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateUndermine function

 @details

 @note

 @param[in] strm
    z_streamp strm

 @param[in] subvert
    int subvert


 @return
	Z_OK, Z_DATA_ERROR, Z_STREAM_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateUndermine 	inflateUndermine



/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateValidate function
 function

 @details

 @note

 @param[in] strm
    z_streamp strm

 @param[in] check
    int check

 @return
	Z_OK, Z_STREAM_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateValidate 	inflateValidate


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateCodesUsed function

 @details

 @note

 @param[in] strm
    z_streamp strm

 @return
	unsigned long ZEXPORT

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateCodesUsed 	inflateCodesUsed


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	inflateResetKeep function

 @details

 @note

 @param[in] strm
    z_streamp strm

 @return
	Z_OK, Z_STREAM_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_inflateResetKeep 	inflateResetKeep


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	deflateResetKeep function

 @details

 @note
 	see deflate.c

 @param[in] strm
	z_streamp strm

 @return
	Z_OK, Z_STREAM_ERROR

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_zlib_deflateResetKeep 	deflateResetKeep


#if (defined(_WIN32) || defined(__CYGWIN__)) && !defined(Z_SOLO)
	#define azx_zlib_gzopen_w 	gzopen_w
#endif
#if defined(STDC) || defined(Z_HAVE_STDARG_H)
	#ifndef Z_SOLO

/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gzvprintf function

 @details

 @note
 	see zlib.h

 @param[in] file
    gzFile file

 @param[in] format
    const char *format

 @param[in] va
    va_list va

 @return
	return int ZEXPORTVA

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
		#define azx_zlib_gzvprintf 	gzvprintf

	#endif
#endif


/*-----------------------------------------------------------------------------------------------*/
/**
 @brief
	gunzip function

 @details
   Decompress the file inname to the file outnname, of if test is true, just
   decompress without writing and check the gzip trailer for integrity.  If
   inname is NULL or an empty string, read from stdin.  If outname is NULL or
   an empty string, write to stdout.  strm is a pre-initialized inflateBack
   structure.  When appropriate, copy the file attributes from inname to
   outname.

 @note

 @param[in] strm
    z_stream* strm

 @param[in] inname
     char* inname

 @param[in] outname
     char* outname

 @param[in] test
     int test

 @return
	gunzip() returns 1 if there is an out-of-memory error or an unexpected
    return code from gunpipe().  Otherwise it returns 0

 <b>Sample usage</b>
 @code
 	 <c code>
 @endcode
 @ingroup zlibUsage
 */
/*-----------------------------------------------------------------------------------------------*/
//#define azx_zlib_gunzip gunzip


/** @} */  //close zlibUsage


#endif /* HDR_AZX_ZLIB_ZLIB_H_ */

