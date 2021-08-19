/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
  	  test_main.h

  @brief
  	  Project: SPI data flash

  @details

  @version
 	 1.0.0

  @note

  @author
  	  Norman Argiolas

  @note
  	  File created on: Aug 24, 2020
*/

#ifndef TEST_HDR_AZX_LFS_USAGE_H_
#define TEST_HDR_AZX_LFS_USAGE_H_

typedef enum
{
	TEST_OK 	= 0,
	TEST_ERROR	= 1
} TEST_RESULT_E;



TEST_RESULT_E runMainTest(void);

#endif /* TEST_HDR_AZX_LFS_USAGE_H_ */
