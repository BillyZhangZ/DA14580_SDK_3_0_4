/**
****************************************************************************************
*
* @file app_hps_proj.h
*
* @brief HPS application header file.
*
* Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
*
* <bluetooth.support@diasemi.com> and contributors.
*
****************************************************************************************
*/

#ifndef APP_HPS_PROJ_H_
#define APP_HPS_PROJ_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief 
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "app_task.h"                  // application task
#include "gapc_task.h"                 // gap functions and messages
#include "gapm_task.h"                 // gap functions and messages
#include "app.h"                       // application definitions
#include "co_error.h"                  // error code definitions
#include "smpc_task.h"                 // error code definitions
 

#if (BLE_HPS_SERVER)
#include "app_hps.h"
#include "app_hps_task.h"
#endif


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/****************************************************************************
Define device name. Used in Advertise string
*****************************************************************************/

#define APP_DEVICE_NAME "ZHIYANG-HPS"

/**
 * Default Advertising data
 * --------------------------------------------------------------------------------------
 * x02 - Length
 * x01 - Flags
 * x06 - LE General Discoverable Mode + BR/EDR Not Supported
 * --------------------------------------------------------------------------------------
 * x03 - Length
 * x03 - Complete list of 16-bit UUIDs available
 * x09\x18 - Health Thermometer Service UUID
 *   or
 * x00\xFF - Nebulization Service UUID
 * --------------------------------------------------------------------------------------
 */

#define APP_ADV_DATA        "\x07\x01\x03\x18"
#define APP_ADV_DATA_LEN    (0)

/**
 * Default Scan response data
 * --------------------------------------------------------------------------------------
 * x09                             - Length
 * xFF                             - Vendor specific advertising type
 * x00\x60\x52\x57\x2D\x42\x4C\x45 - "RW-BLE"
 * --------------------------------------------------------------------------------------
 */
#define APP_SCNRSP_DATA         "\x02\xFF\x00"
#define APP_SCNRSP_DATA_LENGTH  (0)
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @} APP

#endif //APP_TEMPLATE_PROJ_H_
