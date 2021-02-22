/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    app_utils.h

  @brief
    applications update definitions

  @details
    
  @note
    Dependencies:
    m2mb_types.h

  @author
    

  @date
    19/03/2020
*/


#ifndef HDR_APP_UTILS_H_
#define HDR_APP_UTILS_H_

/* Global declarations ==========================================================================*/

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/

/**
  @brief Sets the new application as the new default

  This function will set the new_app application as the new default and reboot the module


  @param [in] new_app: the application name to be set as new default
  @param [in] old_app: the currently running application name to be deleted. Leave to NULL to keep the original file in the filesystem
  @return 0 in case of success, -1 in case of failure

 */
INT32 update_app(const CHAR* new_app, const CHAR* old_app);

#endif /* HDR_APP_UTILS_H_ */
