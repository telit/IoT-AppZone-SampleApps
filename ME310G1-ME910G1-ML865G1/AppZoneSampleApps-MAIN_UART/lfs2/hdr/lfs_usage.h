/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file lfs_usage.h

  @brief Project: app_azx_lib_lfs on C:\Telit\SampleApp\m2mb_sample_apps_SDK_1.0.5\ME310G1-W1\AppZoneSampleApps-MAIN_UART\app_azx_lib_lfs\hdr\lfs_usage.h.

  @note File created on: Oct 21, 2020

  @author NormanAr

*/

#ifndef HDR_LFS_USAGE_H_
#define HDR_LFS_USAGE_H_

typedef enum
{
	TEST_OK 	= 0,
	TEST_ERROR	= 1
} TEST_RESULT_E;

TEST_RESULT_E ramDiskDemo 	(void);
TEST_RESULT_E flashDiskDemo	(void);


#endif /* HDR_LFS_USAGE_H_ */
