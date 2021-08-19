/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */


/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "m2mb_i2c.h"

#include "azx_log.h"
#include "app_cfg.h"

#include "i2c_utils.h"


/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/
INT32 EEPROM_I2C_init(EEPROM_I2C_T* h)
{

  if(!h)
  {
    return EEPROM_I2C_RES_INVALID_HANDLE;
  }

  if(h->init == 1)
  {
    return EEPROM_I2C_RES_OK;
  }

  h->fd = 0;
  h->init = 0;

  CHAR dev_ID[64];
  M2MB_I2C_CFG_T config;

  /**************
   *  Configuring the IIC device.
   **************/
  AZX_LOG_INFO( "\nConfiguring the I2C device...\r\n" );

  //Create device name using device address in decimal base, left shifted by 1 bit
  sprintf(dev_ID, "/dev/I2C-%d", h->dev_addr );
  AZX_LOG_INFO("Opening I2C channel %s ( device address is 0x%02X )\r\n", dev_ID,  h->dev_addr );

  h->fd= m2mb_i2c_open( dev_ID, 0 );

  if (-1 == h->fd)
  {
    AZX_LOG_ERROR("cannot open I2C channel!\r\n");
    return EEPROM_I2C_RES_CANNOT_INIT;
  }

  config.sclPin = h->scl_pin;
  config.sdaPin = h->sda_pin;

  config.registerId = 0x00; /*placeholder*/


  if (m2mb_i2c_ioctl(h->fd, M2MB_I2C_IOCTL_SET_CFG, (void *)&config) != 0)
  {
    AZX_LOG_ERROR("cannot set I2C channel configuration\r\n");
    return EEPROM_I2C_RES_INTERNAL_ERROR;
  }

  h->init = 1;

  return EEPROM_I2C_RES_OK;
}

INT32 EEPROM_I2C_isInit(EEPROM_I2C_T* h)
{

  if(!h)
  {
    return 0;
  }

  if(h->init == 1)
  {
    return 1;
  }
  return 0;
}


INT32 EEPROM_I2C_deinit(EEPROM_I2C_T* h)
{

  if(!h)
  {
    return EEPROM_I2C_RES_INVALID_HANDLE;
  }

  if(h->init == 0)
  {
    return EEPROM_I2C_RES_OK;
  }

  m2mb_i2c_close(h->fd);
  h->fd = 0;
  h->init = 0;

  return EEPROM_I2C_RES_OK;
}



INT32 EEPROM_i2c_write(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 *pData, UINT16 size)
{
  INT32 i2c_res;

  UINT8 *buf;
  UINT32 bufsize = size + 2; //2 bytes of memaddress

  M2MB_I2C_CFG_T configrw = {0};
  M2MB_I2C_RDWR_IOCTL_DATA rdrw_data = {0};
  M2MB_I2C_MSG msgs[2];

  if(!hi2c || !pData)
  {
    return EEPROM_I2C_RES_INVALID_HANDLE;
  }
  if(hi2c->init != 1)
  {
    return EEPROM_I2C_RES_NOT_INIT;
  }

  /* Retrieve current channel config */
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&configrw);
  if (i2c_res != 0)
  {
    return EEPROM_I2C_RES_INTERNAL_ERROR;
  }
  configrw.registerId = 0;

  buf = (UINT8 *)m2mb_os_malloc(bufsize);
  if(!buf)
  {
    return EEPROM_I2CRES_CANNOT_ALLOCATE;
  }

  buf[0] = memAddress >> 8;
  buf[1] = memAddress & 0xFF;

  msgs[0].buf = (UINT8*)buf;
  msgs[0].flags = I2C_M_WR;
  msgs[0].len = bufsize; // 2 address bytes + data
  
  msgs[1].buf = NULL;
  msgs[1].flags = 0;
  msgs[1].len = 0;
  
  memcpy(buf + 2, pData, size );

  rdrw_data.msgs = &msgs[0];
  rdrw_data.nmsgs = 1;

  configrw.rw_param = &rdrw_data;
  AZX_LOG_TRACE("Trying to write %d bytes\r\n", size);
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_RDWR, (void *)&configrw);

  m2mb_os_free(buf);


  if (i2c_res != (INT32) bufsize) /*expected bytes*/
  {
    AZX_LOG_ERROR("Write failure, error: %d\r\n", i2c_res);
    return EEPROM_I2C_RES_WRITE_ERR;
  } else
  {
    return EEPROM_I2C_RES_OK;
  }

}


INT32 EEPROM_i2c_write_byte(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 data)
{
  INT32 i2c_res;

  UINT8 buf[3]; //2 bytes of address, one of data


  M2MB_I2C_CFG_T configrw = {0};
  M2MB_I2C_RDWR_IOCTL_DATA rdrw_data = {0};
  M2MB_I2C_MSG msgs[2];

  if(!hi2c)
  {
    return EEPROM_I2C_RES_INVALID_HANDLE;
  }
  if(hi2c->init != 1)
  {
    return EEPROM_I2C_RES_NOT_INIT;
  }

  /* Retrieve current channel config */
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&configrw);
  if (i2c_res != 0)
  {
    return EEPROM_I2C_RES_INTERNAL_ERROR;
  }
  configrw.registerId = 0;

  buf[0] = memAddress >> 8;
  buf[1] = memAddress & 0xFF;
  buf[1] = data;


  msgs[0].buf = (UINT8*)buf;
  msgs[0].flags = I2C_M_WR;
  msgs[0].len = sizeof(buf); // 2 address bytes + data

  msgs[1].buf = NULL;
  msgs[1].flags = 0;
  msgs[1].len = 0;

  rdrw_data.msgs = &msgs[0];
  rdrw_data.nmsgs = 1;

  configrw.rw_param = &rdrw_data;
  AZX_LOG_TRACE("Trying to write %d bytes\r\n", configrw.rw_param->msgs[0].len);
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_RDWR, (void *)&configrw);

  if (i2c_res != sizeof(buf)) /*expected bytes*/
  {
    AZX_LOG_ERROR("Write failure, error: %d\r\n", i2c_res);
    return EEPROM_I2C_RES_WRITE_ERR;
  } else
  {
    return EEPROM_I2C_RES_OK;
  }

}



INT32 EEPROM_i2c_read(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 *pData, UINT16 size)
{
  INT32 i2c_res;

  UINT8 buf[2]; //2 bytes of address

  M2MB_I2C_CFG_T configrw = {0};
  M2MB_I2C_RDWR_IOCTL_DATA rdrw_data = {0};
  M2MB_I2C_MSG msgs[2];

  if(!hi2c || !pData)
  {
    return EEPROM_I2C_RES_INVALID_HANDLE;
  }
  if(hi2c->init != 1)
  {
    return EEPROM_I2C_RES_NOT_INIT;
  }

  /* Retrieve current channel config */
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&configrw);
  if (i2c_res != 0)
  {
    //AZX_LOG_ERROR("cannot get I2C channel configuration\r\n");
    return EEPROM_I2C_RES_INTERNAL_ERROR;
  }
  configrw.registerId = 0;


  buf[0] = memAddress >> 8;
  buf[1] = memAddress & 0xFF;

  msgs[0].buf = (UINT8*)buf;
  msgs[0].flags = I2C_M_WR;
  msgs[0].len = 2; // 2 address bytes

  msgs[1].buf = (UINT8*)pData;
  msgs[1].flags = I2C_M_RD;
  msgs[1].len = size; // 2 address bytes

  rdrw_data.msgs = &msgs[0];
  rdrw_data.nmsgs = 2;

  configrw.rw_param = &rdrw_data;
  AZX_LOG_TRACE("Trying to read %d bytes\r\n", size);
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_RDWR, (void *)&configrw);

  if (i2c_res != size) /*expected bytes*/
  {
    AZX_LOG_ERROR("Read failure, error: %d\r\n", i2c_res);
    return EEPROM_I2C_RES_READ_ERR;
  } else
  {
    return EEPROM_I2C_RES_OK;
  }

}

INT32 EEPROM_i2c_read_delayed_chunked(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 *pData, UINT16 size, UINT16 chunksize, UINT16 delay)
{
  INT32 i2c_res;

  UINT8 buf[2]; //2 bytes of address

  UINT16 chunk;
  UINT32 read = 0;

  M2MB_I2C_CFG_T configrw = {0};
  M2MB_I2C_RDWR_IOCTL_DATA rdrw_data = {0};
  M2MB_I2C_MSG msgs[2];


  if(!hi2c || !pData)
  {
    return EEPROM_I2C_RES_INVALID_HANDLE;
  }
  if(hi2c->init != 1)
  {
    return EEPROM_I2C_RES_NOT_INIT;
  }

  /* Retrieve current channel config */
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&configrw);
  if (i2c_res != 0)
  {
    return EEPROM_I2C_RES_INTERNAL_ERROR;
  }
  configrw.registerId = 0;


  buf[0] = memAddress >> 8;
  buf[1] = memAddress & 0xFF;

  msgs[0].buf = (UINT8*)buf;
  msgs[0].flags = I2C_M_WR;
  msgs[0].len = 2; // 2 address bytes

  chunk = (size > chunksize)? chunksize: size;

  msgs[1].buf = (UINT8*)pData;
  msgs[1].flags = I2C_M_RD;
  msgs[1].len = chunk;

  rdrw_data.msgs = &msgs[0];
  rdrw_data.nmsgs = 2;

  configrw.rw_param = &rdrw_data;
  AZX_LOG_TRACE("Trying to read %d bytes\r\n", chunk);
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_RDWR, (void *)&configrw);

  if (i2c_res != chunk) /*expected bytes*/
  {
    AZX_LOG_ERROR("Read failure, error: %d\r\n", i2c_res);
    return EEPROM_I2C_RES_READ_ERR;
  }

  read += chunk;


  while (size > read)
  {
    m2mb_os_taskSleep(M2MB_OS_MS2TICKS(delay));

    chunk = (size > chunksize)? chunksize: size;

    msgs[0].buf = NULL;
    msgs[0].flags = I2C_M_WR;
    msgs[0].len = 0;

    msgs[1].buf = (UINT8*)pData + read;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = chunksize ;

    rdrw_data.msgs = &msgs[0];
    rdrw_data.nmsgs = 2;

    configrw.rw_param = &rdrw_data;

    AZX_LOG_TRACE("Trying to read %d bytes\r\n", chunksize);
    i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_RDWR, (void *)&configrw);
    if (i2c_res != chunksize) /*expected bytes*/
    {
      AZX_LOG_ERROR("Read failure, error: %d\r\n", i2c_res);
      return EEPROM_I2C_RES_READ_ERR;
    }

    read += chunk;
  }


  return EEPROM_I2C_RES_OK;


}


INT32 EEPROM_i2c_read_byte_from_address(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 *pData)
{
  INT32 i2c_res;

  UINT8 buf[2]; //2 bytes of address

  M2MB_I2C_CFG_T configrw = {0};
  M2MB_I2C_RDWR_IOCTL_DATA rdrw_data = {0};
  M2MB_I2C_MSG msgs[2];

  if(!hi2c || !pData)
  {
    return EEPROM_I2C_RES_INVALID_HANDLE;
  }
  if(hi2c->init != 1)
  {
    return EEPROM_I2C_RES_NOT_INIT;
  }

  /* Retrieve current channel config */
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&configrw);
  if (i2c_res != 0)
  {
    return EEPROM_I2C_RES_INTERNAL_ERROR;
  }
  configrw.registerId = 0;


  buf[0] = memAddress >> 8;
  buf[1] = memAddress & 0xFF;

  msgs[0].buf = (UINT8*)buf;
  msgs[0].flags = I2C_M_WR;
  msgs[0].len = 2; // 2 address bytes

  msgs[1].buf = (UINT8*)pData;
  msgs[1].flags = I2C_M_RD;
  msgs[1].len = 1; //data

  rdrw_data.msgs = &msgs[0];
  rdrw_data.nmsgs = 2;

  configrw.rw_param = &rdrw_data;
  AZX_LOG_TRACE("Trying to read 1 byte\r\n");
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_RDWR, (void *)&configrw);

  if (i2c_res != 1) /*expected bytes*/
  {
    AZX_LOG_ERROR("Read failure, error: %d\r\n", i2c_res);
    return EEPROM_I2C_RES_READ_ERR;
  } else
  {
    return EEPROM_I2C_RES_OK;
  }

}


INT32 EEPROM_i2c_read_byte(EEPROM_I2C_T *hi2c, UINT8 *pData)
{
  INT32 i2c_res;

  M2MB_I2C_CFG_T configrw = {0};
  M2MB_I2C_RDWR_IOCTL_DATA rdrw_data = {0};
  M2MB_I2C_MSG msgs[2];

  if(!hi2c || !pData)
  {
    return EEPROM_I2C_RES_INVALID_HANDLE;
  }
  if(hi2c->init != 1)
  {
    return EEPROM_I2C_RES_NOT_INIT;
  }

  /* Retrieve current channel config */
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&configrw);
  if (i2c_res != 0)
  {
    return EEPROM_I2C_RES_INTERNAL_ERROR;
  }
  configrw.registerId = 0;

  msgs[0].buf = NULL;
  msgs[0].flags = I2C_M_WR;
  msgs[0].len = 0; //do not write data

  msgs[1].buf = (UINT8*)pData;
  msgs[1].flags = I2C_M_RD;
  msgs[1].len = 1; //data

  rdrw_data.msgs = &msgs[0];
  rdrw_data.nmsgs = 2;

  configrw.rw_param = &rdrw_data;
  AZX_LOG_TRACE("Trying to read 1 byte\r\n");
  i2c_res = m2mb_i2c_ioctl(hi2c->fd, M2MB_I2C_IOCTL_RDWR, (void *)&configrw);

  if (i2c_res != 1) /*expected bytes*/
  {
    AZX_LOG_ERROR("Read failure, error: %d\r\n", i2c_res);
    return EEPROM_I2C_RES_READ_ERR;
  } else
  {
    return EEPROM_I2C_RES_OK;
  }

}
