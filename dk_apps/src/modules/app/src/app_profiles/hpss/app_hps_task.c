/**
 ****************************************************************************************
 *
 * @file app_hps_task.c
 *
 * @brief HTTP Proxy Service Application Task
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

#include "rwip_config.h"        // SW Configuration
#include <string.h>             // srtlen()

#if (BLE_HPS_SERVER)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "hpss_task.h"          // HTTP Proxy Service Server Task API
#include "hpss.h"               // HTTP Proxy Service Definitions
#include "app_hps.h"            // HTTP Proxy Service Application Definitions
#include "app_hps_task.h"       // HTTP Proxy Service Application Task API
#include "app_task.h"           // Application Task API
#include "app_console.h"


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles HPS Server profile database creation confirmation.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int hpss_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct hpss_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_DB_INIT)
    {
        if (param->status == CO_ERROR_NO_ERROR)
        {
            // Go to the idle state

        }

        // Inform the Application Manager
        struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
                                                           TASK_APP, TASK_APP,
                                                           app_module_init_cmp_evt);

        cfm->status = param->status;

        ke_msg_send(cfm);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles disable indication from the HPS Server profile.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int hpss_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct hpss_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles HPS Server profile URI data indication.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int hpss_uri_ind_handler(ke_msg_id_t const msgid,
                                      struct hpss_uri_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
#if 1
  	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ, 
														TASK_HPSS,
														TASK_APP,
														hpss_status_code_send_req);
	req->conhdl = param->conhdl;
	req->status_code.status_code = 0x55AA;
	req->status_code.data_status = 0x01;
	ke_msg_send(req);
#endif
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles HPS Server profile HEADER data indication.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int hpss_header_ind_handler(ke_msg_id_t const msgid,
                                      struct hpss_header_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
  	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ, 
														TASK_HPSS,
														TASK_APP,
														hpss_status_code_send_req);
	req->conhdl = param->conhdl;
	req->status_code.status_code = 0x55AA;
	req->status_code.data_status = 0x02;
	ke_msg_send(req);
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles HPS Server profile BODY data indication.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int hpss_body_ind_handler(ke_msg_id_t const msgid,
                                      struct hpss_body_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
  	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ, 
														TASK_HPSS,
														TASK_APP,
														hpss_status_code_send_req);
	req->conhdl = param->conhdl;
	req->status_code.status_code = 0x55AA;
	req->status_code.data_status = 0x03;
	ke_msg_send(req);
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles HPS Server profile Control Point data indication.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int hpss_cntl_pt_ind_handler(ke_msg_id_t const msgid,
                                      struct hpss_cntl_pt_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
  	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ, 
														TASK_HPSS,
														TASK_APP,
														hpss_status_code_send_req);
	req->conhdl = param->conhdl;
	req->status_code.status_code = 0x55AA;
	req->status_code.data_status = 0x04;
	ke_msg_send(req);
    return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief Handles HPS Server profile status code client configuration indication.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int hpss_status_code_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
                                      struct hpss_cfg_indntf_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
  	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ, 
														TASK_HPSS,
														TASK_APP,
														hpss_status_code_send_req);
	req->conhdl = param->conhdl;
	req->status_code.status_code = 0x01;
	req->status_code.data_status = 0x02;
	ke_msg_send(req);
    return (KE_MSG_CONSUMED);
}




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

#endif //(BLE_HPS_SERVER)

/// @} APP
