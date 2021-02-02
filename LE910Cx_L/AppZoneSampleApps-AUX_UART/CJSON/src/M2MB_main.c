/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
     M2M_main.c

 @brief
     JSON object manipulation

 @details
     Sample application showing how to manipulate JSON objects

 @description
    Sample application showcasing how to manage JSON objects. Debug prints on AUX UART
    
 @version
     1.0.1

 @notes
     Start of Appzone: Entry point
     User code entry is in function M2M_main()

 @author Fabio Pintus
 @author Norman Argiolas

 @date
     04/02/2020
 */

/* Include files ================================================================================*/
#include "string.h"

#include "m2mb_types.h"
#include "azx_log.h"
#include "azx_utils.h"

#include "azx_cjson.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

/* Local statics ================================================================================*/
//  {
//    "name": "Atlantic Ocean",
//    "format" :
//    {
//      "type": "salt",
//      "volume": 310410900,
//      "depth": -8486,
//      "volume_percent": 23.3,
//      "tide": -3.5,
//      "calm":false,
//      "life": [ "plankton", "corals ", "fish ", "mammals"]
//    }
//  };
char my_json_ocean[] =
    "{\"name\":\"Atlantic Ocean\",\"format\":{\"type\":\"salt\",\"volume\":310410900,\"depth\":-8486,\"volume_percent\":23.3,\"tide\":-3.5,\"calm\":false,\"life\":[\"plankton\",\"corals\",\"fish\",\"mammals\"]}}";

/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
 \User Entry Point of Appzone

 \param [in] Module Id

 \details Main of the appzone user
 **************************************************************************************************/

void M2MB_main(int argc, char **argv)
{
  //suppress unused warning
  (void) argc;
  (void) argv;

  //user variables
  AZX_LOG_CFG_T log_cfg;

  AZX_CJSON_T * root;
  AZX_CJSON_T * nono;
  char * name;
  AZX_CJSON_T * format;
  char * typen;
  int volume;
  int depth;
  double percent;
  double tide;
  int calm;
  AZX_CJSON_T *life;
  char animals[200];
  AZX_CJSON_T *rootn, *cmd1, *cmd2, *params1, *params2;

  //dalay before application starting
  azx_sleep_ms(5000);

  /*Set log configuration */
  log_cfg.log_channel = LOG_CHANNEL; /* Defined in app_cfg.h */
  log_cfg.log_level = AZX_LOG_LEVEL_TRACE; /*Enable all logs*/
  log_cfg.log_colours = 0; /*Set to 1 to use coloured logs (not all terminals are compatible)*/

  azx_log_init(&log_cfg);

  //Print all levels with default macros
  AZX_LOG_INFO("Starting Logging demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);

  //start reading CJSON manipulation
  root = azx_cjson_parse(my_json_ocean);
  if (!root)
  {
    AZX_LOG_ERROR("failed getting root\r\n");
    return;
  }
  AZX_LOG_INFO("And here is what we got:\r\n%s\r\n", azx_cjson_print(root));

  // testing inexistent key
  nono = azx_cjson_getObjectItem(root, "inexistent");
  if (!nono)
  {
    AZX_LOG_INFO("inexistent key not found\r\n");
  }

  name = azx_cjson_getObjectItem(root, "name")->valuestring;
  if (!name)
  {
    AZX_LOG_ERROR("failed getting name\r\n");
    return;
  }
  AZX_LOG_INFO("name found: %s\r\n", name);

  format = azx_cjson_getObjectItem(root, "format");
  if (!format)
  {
    AZX_LOG_ERROR("failed getting format\r\n");
    return;
  }
  AZX_LOG_INFO("format found %s\r\n", format->valuestring);

  typen = azx_cjson_getObjectItem(format, "type")->valuestring;
  if (!typen)
  {
    AZX_LOG_ERROR("failed getting type\r\n");
    return;
  }

  volume = azx_cjson_getObjectItem(format, "volume")->valueint;
  depth = azx_cjson_getObjectItem(format, "depth")->valueint;
  percent = azx_cjson_getObjectItem(format, "volume_percent")->valuedouble;
  tide = azx_cjson_getObjectItem(format, "tide")->valuedouble;
  calm = azx_cjson_getObjectItem(format, "calm")->valueint;
  life = azx_cjson_getObjectItem(format, "life");


  memset(animals, 0, sizeof(animals));
  if (life && life->type == AZX_CJSON_ARRAY)
  {
    int i;
    for (i = 0; i < azx_cjson_getArraySize(life); i++)
    {
      strcat(animals, azx_cjson_getArrayItem(life, i)->valuestring);
      strcat(animals, ", ");
    }
    animals[strlen(animals) - 2] = 0;
  }

  AZX_LOG_INFO("Our JSON string contains info about an ocean named %s,\r\n",
      name);
  AZX_LOG_INFO(
      "has a volume of %d km^3 of %s water with %d meters max depth,\r\n",
      volume, typen, depth);
  AZX_LOG_INFO("represents %.1f%% of total oceans volume,\r\n", percent);
  AZX_LOG_INFO("has an average low tide of %.1f meters,\r\n", tide);
  AZX_LOG_INFO("hosts a huge number of living creatures such as %s,\r\n",
      animals);
  if (calm)
    AZX_LOG_INFO("and is always calm.\r\n");
  else
    AZX_LOG_INFO("and is not always calm.\r\n");
  azx_cjson_delete(root);

  AZX_LOG_INFO(
      "\r\nLet's build a TR50 command with a proprety.publish and an alarm.publish for MQTT (no auth).\r\n");


  //start writing CJSON manipulation
  rootn = azx_cjson_createObject();

  azx_cjson_addItemToObject(rootn, "1", cmd1 = azx_cjson_createObject());
  azx_cjson_addStringToObject(cmd1, "command", "property.publish");
  azx_cjson_addItemToObject(cmd1, "params", params1 = azx_cjson_createObject());
  azx_cjson_addStringToObject(params1, "thingKey", "mything");
  azx_cjson_addStringToObject(params1, "key", "mykey");
  azx_cjson_addNumberToObject(params1, "value", 123.144);

  azx_cjson_addItemToObject(rootn, "2", cmd2 = azx_cjson_createObject());
  azx_cjson_addStringToObject(cmd2, "command", "alarm.publish");
  azx_cjson_addItemToObject(cmd2, "params", params2 = azx_cjson_createObject());
  azx_cjson_addStringToObject(params2, "thingKey", "mything");
  azx_cjson_addStringToObject(params2, "key", "mykey");
  azx_cjson_addNumberToObject(params2, "state", 3);
  azx_cjson_addStringToObject(params2, "msg", "Message.");

  AZX_LOG_INFO("And here is what we got:\r\n%s\r\n", azx_cjson_print(rootn));
  azx_cjson_delete(rootn);

  AZX_LOG_INFO("END.\r\n");

  azx_log_deinit();

}

