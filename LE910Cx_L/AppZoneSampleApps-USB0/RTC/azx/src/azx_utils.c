/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

#include <string.h>
#include <stdio.h>
#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_power.h"

#include "azx_log.h"

#include "azx_utils.h"

void azx_sleep_ms(UINT32 ms)
{
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS(ms) );
}


#define MIN(i,j) (((i) < (j)) ? (i) : (j))
const CHAR* azx_hex_dump(const void* data, UINT32 len)
{
  static CHAR buff[AZX_UTILS_HEX_DUMP_BUFFER_SIZE];
  UINT32 i;

  memset(buff, 0, AZX_UTILS_HEX_DUMP_BUFFER_SIZE);
  if (data == NULL)
  {
    snprintf(buff, AZX_UTILS_HEX_DUMP_BUFFER_SIZE, "(null)");
    goto end;
  }

  len = MIN(len, AZX_UTILS_HEX_DUMP_BUFFER_SIZE/5);

  for (i = 0; i < len; ++i)
  {
    snprintf(buff + 5*i, AZX_UTILS_HEX_DUMP_BUFFER_SIZE - 5*i,
				"0x%02x ", ((const UINT8*)data)[i]);
  }

end:
  return buff;
}

void azx_reboot_now(void)
{
  M2MB_POWER_HANDLE h = NULL;

  if(M2MB_RESULT_SUCCESS == m2mb_power_init(&h, NULL, NULL))
  {
    AZX_LOG_DEBUG("It's dead, Jim!\r\n");
    AZX_LOG_INFO("Rebooting device\r\n");
    m2mb_power_reboot(h);
  }
}

void azx_shutdown_now(void)
{
  M2MB_POWER_HANDLE h = NULL;

  if(M2MB_RESULT_SUCCESS == m2mb_power_init(&h, NULL, NULL))
  {
    AZX_LOG_DEBUG("It's dead, Jim!\r\n");
    AZX_LOG_INFO("Shutting down device\r\n");
    m2mb_power_shutdown(h);
  }
}
