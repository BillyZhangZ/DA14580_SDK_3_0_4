/**
 ****************************************************************************************
 *
 * @file app_hps.c
 *
 * @brief HTTP Proxy Service Application entry point
 *
 * Copyright (C) zhiyang.zhang 2015-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_HPS_SERVER)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_hps.h"                 // HTTP Proxy Service Application Definitions
#include "app_hps_task.h"            // HTTP Proxy Service Application Task API
#include "app.h"                     // Application Definitions
#include "app_task.h"                // Application Task Definitions
#include "hpss_task.h"               // HTTP Proxy Service Functions
#include "app_console.h"

/*
 * LOCAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_hps_init(void)
{
	arch_printf("hps init\n");
    return;
}

void app_hps_create_db_send(void)
{
    // Add HPS in the database
    struct hpss_create_db_req *req = KE_MSG_ALLOC(HPSS_CREATE_DB_REQ,
                                                  TASK_HPSS, TASK_APP,
                                                  hpss_create_db_req);

    req->features = APP_HPS_FEATURES;

    // Send the message
    ke_msg_send(req);
}

void app_hps_enable_prf(uint16_t conhdl)
{
    // Allocate the message
    struct hpss_enable_req *req = KE_MSG_ALLOC(HPSS_ENABLE_REQ,
                                               TASK_HPSS, TASK_APP,
                                               hpss_enable_req);

    // Fill in the parameter structure
    req->conhdl             = conhdl;
    req->sec_lvl            = PERM(SVC, ENABLE);

    // Send the message
    ke_msg_send(req);

}

void app_hps_send_status_code(uint16_t conhdl, int16_t status_code, uint8_t data_status)
{
	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ,TASK_HPSS,TASK_APP,hpss_status_code_send_req);

    // Fill in the parameter structure
    req->conhdl             = conhdl;
	req->status_code.status_code = status_code;
	req->status_code.data_status = data_status;

	// Send the message
    ke_msg_send(req);
}
#endif //BLE_HPS_SERVER

/// @} APP
