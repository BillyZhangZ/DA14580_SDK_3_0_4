/**
 ****************************************************************************************
 *
 * @file hpss_task.c
 *
 * @brief HTTP Proxy Server Task implementation.
 *
 * Copyright (C) zhiyang.zhang 2015-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HPSS
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_HPS_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm_util.h"
#include "atts_util.h"
#include "hpss.h"
#include "hpss_task.h"
#include "attm_cfg.h"
#include "prf_utils.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HPSS_CREATE_DB_REQ message.
 * The handler adds HPS into the database.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int hpss_create_db_req_handler(ke_msg_id_t const msgid,
                                       struct hpss_create_db_req const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    //Database Creation Status
    uint8_t status;

    //Save Profile ID
    hpss_env.con_info.prf_id = TASK_HPSS;

    /*---------------------------------------------------*
     * HTTP Proxy Service Creation
     *---------------------------------------------------*/

    //Add Service Into Database
    status = attm_svc_create_db(&hpss_env.hps_shdl, NULL, HPS_IDX_NB, NULL,
                               dest_id, &hpss_att_db[0]);
    //Disable HPS
    attmdb_svc_set_permission(hpss_env.hps_shdl, PERM(SVC, DISABLE));

    //Go to Idle State
    if (status == ATT_ERR_NO_ERROR)
    {
        //If we are here, database has been fulfilled with success, go to idle state
        ke_state_set(TASK_HPSS, HPSS_IDLE);
    }

    //Send CFM to application
    struct hpss_create_db_cfm * cfm = KE_MSG_ALLOC(HPSS_CREATE_DB_CFM, src_id,
                                                    TASK_HPSS, hpss_create_db_cfm);
    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Enable the HPS Server role, used after connection.
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int hpss_enable_req_handler(ke_msg_id_t const msgid,
                                    struct hpss_enable_req const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    uint16_t value = 0;
    // Keep source of message, to respond to it further on
    hpss_env.con_info.appid = src_id;
    // Store the connection handle for which this profile is enabled
    hpss_env.con_info.conidx = gapc_get_conidx(param->conhdl);

    // Check if the provided connection exist
    if (hpss_env.con_info.conidx == GAP_INVALID_CONIDX)
    {
        // The connection doesn't exist, request disallowed
        prf_server_error_ind_send((prf_env_struct *)&hpss_env, PRF_ERR_REQ_DISALLOWED,
                                  HPSS_ERROR_IND, HPSS_ENABLE_REQ);
    }
    else
    {
        // Enable HPS + Set Security Level
        attmdb_svc_set_permission(hpss_env.hps_shdl, param->sec_lvl);
		        //Set HR Meas. Char. NTF Configuration in DB
        attmdb_att_set_value(hpss_env.hps_shdl + HPS_IDX_STATUS_CODE_CFG, sizeof(uint16_t),
                             (uint8_t *)&value);
//fix me?
        //Set LLS Alert Level to specified value
       // attmdb_att_set_value(hpss_env.hps_shdl + LLS_IDX_ALERT_LVL_VAL,
       //                     sizeof(uint8_t), (uint8_t *)&param->lls_alert_lvl);

        // Go to Connected state
        ke_state_set(TASK_HPSS, HPSS_CONNECTED);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GL2C_CODE_ATT_WR_CMD_IND message.
 * The handler will 
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_write_cmd_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_write_cmd_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
	uint16_t value = 0x0000;
    uint8_t char_code = HPS_ERR_CHAR;
    uint8_t status = PRF_APP_ERROR;

    if (KE_IDX_GET(src_id) == hpss_env.con_info.conidx)
    {
        if (param->handle == hpss_env.hps_shdl + HPS_IDX_URI_VAL)
        {
            char_code = HPS_URI_CHAR;
        }
		else if (param->handle == hpss_env.hps_shdl + HPS_IDX_HEADER_VAL)
        {
            char_code = HPS_HEADER_CHAR;
        }
		else if (param->handle == hpss_env.hps_shdl + HPS_IDX_BODY_VAL)
        {
            char_code = HPS_BODY_CHAR;
        }
		else if (param->handle == hpss_env.hps_shdl + HPS_IDX_CNTL_PT_VAL)
        {
            char_code = HPS_CNTL_PT_CHAR;
        }
		//this case is processed seperately
       	else if (param->handle == hpss_env.hps_shdl + HPS_IDX_STATUS_CODE_CFG)
        {
           //Extract value before check
            memcpy(&value, &(param->value), sizeof(uint16_t));

            if ((value == PRF_CLI_STOP_NTFIND) || (value == PRF_CLI_START_NTF))
            {
				 status = PRF_ERR_OK;
                if (value == PRF_CLI_STOP_NTFIND)
                {
                    hpss_env.features &= ~HPSS_STATUS_CODE_NTF_CFG;
                }
                else //PRF_CLI_START_NTF
                {
                    hpss_env.features |= HPSS_STATUS_CODE_NTF_CFG;
                }
            }
            else
            {
                status = PRF_APP_ERROR;
            }

            if (status == PRF_ERR_OK)
            {
                //Update the attribute value
                attmdb_att_set_value(param->handle, sizeof(uint16_t), (uint8_t *)&value);
                if(param->last)
                {
                    //Inform APP of configuration change
                    struct hpss_cfg_indntf_ind * ind = KE_MSG_ALLOC(HPSS_CFG_INDNTF_IND,
                                                                    hpss_env.con_info.appid, TASK_HPSS,
                                                                    hpss_cfg_indntf_ind);

                    ind->conhdl = gapc_get_conhdl(hpss_env.con_info.conidx);
                    memcpy(&ind->cfg_val, &value, sizeof(uint16_t));

                    ke_msg_send(ind);
                }
            }
			 //If HPS, send write response
            if (param->response != 0x00)
            {
                // Send Write Response
                atts_write_rsp_send(hpss_env.con_info.conidx, param->handle, status);
            }
			return  (KE_MSG_CONSUMED);
        }
        if (char_code != HPS_ERR_CHAR)
        {
            //Save value in DB
            attmdb_att_set_value(param->handle, sizeof(uint8_t), (uint8_t *)&param->value[0]);

            status = PRF_ERR_OK;
           
		    switch(char_code)
			{
				case  HPS_URI_CHAR:
					if ((param->length + param->offset) <= HPS_URI_MAX_LEN)
					{
						// First part of the uri value
						if (param->offset == 0)
						{
							// Set value in the database
							attmdb_att_set_value(param->handle, param->length, (uint8_t *)&param->value[0]);
						}
						else
						{
							// Complete the value stored in the database
							attmdb_att_update_value(param->handle, param->length, param->offset,
													(uint8_t *)&param->value[0]);
						}
						if(param->last)
		                {
		                	uint16_t len = 0;
							uint8_t *data = NULL;
	                        // Get complete uri value and length
	                        attmdb_att_get_value(param->handle, &len, &data);
							// Inform APP. Allocate the URI value change indication
							struct hpss_uri_ind *ind = KE_MSG_ALLOC(HPSS_URI_IND,
														hpss_env.con_info.appid, TASK_HPSS,
														hpss_uri_ind);

	                       	
							// Fill in the parameter structure
	                		ind->conhdl = gapc_get_conhdl(hpss_env.con_info.conidx);
							ind->char_code = HPS_URI_CHAR;
							ind->len = len;
							
							memcpy(&ind->uri, data, len);

	                        ke_msg_send(ind);
		                }
					}
					break;
				case  HPS_HEADER_CHAR:
					if ((param->length + param->offset) <= HPS_HEADER_MAX_LEN)
					{
						// First part of the uri value
						if (param->offset == 0)
						{
							// Set value in the database
							attmdb_att_set_value(param->handle, param->length, (uint8_t *)&param->value[0]);
						}
						else
						{
							// Complete the value stored in the database
							attmdb_att_update_value(param->handle, param->length, param->offset,
													(uint8_t *)&param->value[0]);
						}
						if(param->last)
		                {
		                	uint16_t len = 0;
							uint8_t *data = NULL;
	                        // Get complete uri value and length
	                        attmdb_att_get_value(param->handle, &len, &data);
							// Inform APP. Allocate the URI value change indication
							struct hpss_header_ind *ind = KE_MSG_ALLOC(HPSS_HEADER_IND,
														hpss_env.con_info.appid, TASK_HPSS,
														hpss_header_ind);

	                       	
							// Fill in the parameter structure
	                		ind->conhdl = gapc_get_conhdl(hpss_env.con_info.conidx);
							ind->char_code = HPS_HEADER_CHAR;
							ind->len = len;
							
							memcpy(&ind->header, data, len);

	                        ke_msg_send(ind);
		                }
					}
					break;
				case  HPS_BODY_CHAR:
					if ((param->length + param->offset) <= HPS_BODY_MAX_LEN)
					{
						// First part of the uri value
						if (param->offset == 0)
						{
							// Set value in the database
							attmdb_att_set_value(param->handle, param->length, (uint8_t *)&param->value[0]);
						}
						else
						{
							// Complete the value stored in the database
							attmdb_att_update_value(param->handle, param->length, param->offset,
													(uint8_t *)&param->value[0]);
						}
						if(param->last)
		                {
		                	uint16_t len = 0;
							uint8_t *data = NULL;
	                        // Get complete uri value and length
	                        attmdb_att_get_value(param->handle, &len, &data);
							// Inform APP. Allocate the URI value change indication
							struct hpss_body_ind *ind = KE_MSG_ALLOC(HPSS_BODY_IND,
														hpss_env.con_info.appid, TASK_HPSS,
														hpss_body_ind);

	                       	
							// Fill in the parameter structure
	                		ind->conhdl = gapc_get_conhdl(hpss_env.con_info.conidx);
							ind->char_code = HPS_BODY_CHAR;
							ind->len = len;
							
							memcpy(&ind->body, data, len);

	                        ke_msg_send(ind);
		                }
					}
					break;
				case  HPS_CNTL_PT_CHAR:
					if ((param->length + param->offset) <= HPS_CNTL_PT_MAX_LEN)
					{
						// First part of the uri value
						if (param->offset == 0)
						{
							// Set value in the database
							attmdb_att_set_value(param->handle, param->length, (uint8_t *)&param->value[0]);
						}
						else
						{
							// Complete the value stored in the database
							attmdb_att_update_value(param->handle, param->length, param->offset,
													(uint8_t *)&param->value[0]);
						}
						if(param->last)
		                {
		                	uint16_t len = 0;
							uint8_t *data = NULL;
	                        // Get complete uri value and length
	                        attmdb_att_get_value(param->handle, &len, &data);
							// Inform APP. Allocate the URI value change indication
							struct hpss_cntl_pt_ind *ind = KE_MSG_ALLOC(HPSS_CNTL_PT_IND,
														hpss_env.con_info.appid, TASK_HPSS,
														hpss_cntl_pt_ind);

	                       	
							// Fill in the parameter structure
	                		ind->conhdl = gapc_get_conhdl(hpss_env.con_info.conidx);
							ind->char_code = HPS_CNTL_PT_CHAR;
							ind->len = len;
							
							memcpy(&ind->cntl_pt, data, len);

	                        ke_msg_send(ind);
		                }
					}
					break;
				default:
				break;
			}
            //If HPS, send write response
            if (param->response != 0x00)
            {
                // Send Write Response
                atts_write_rsp_send(hpss_env.con_info.conidx, param->handle, status);
            }
        }
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Disconnection indication to HTTP Server.
 * 
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                        struct gapc_disconnect_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    // Check Connection Handle
    if (KE_IDX_GET(src_id) == hpss_env.con_info.conidx)
    {
        // Abnormal reason 
        if ((param->reason != CO_ERROR_REMOTE_USER_TERM_CON) &&
            (param->reason != CO_ERROR_CON_TERM_BY_LOCAL_HOST))
        {
        }

        // In any case, inform APP about disconnection
        hpss_disable(param->conhdl);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HPSS_HTTP_SEND_STATUS_REQ message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int hpss_status_code_send_req_handler(ke_msg_id_t const msgid,
                                      struct hpss_status_code_send_req const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    // Status
    uint8_t status = PRF_ERR_OK;

    if(param->conhdl == gapc_get_conhdl(hpss_env.con_info.conidx))
    {
//fix me
        // Check if notifications are enabled
        if(HPSS_IS_SUPPORTED(HPSS_STATUS_CODE_NTF_CFG))
        {
            //Update value in DB
            attmdb_att_set_value(hpss_env.hps_shdl + HPS_IDX_STATUS_CODE_VAL,
                                 HPS_STATUS_CODE_LEN, (uint8_t *)&param->status_code);//fix me, LSB?

            //send notification through GATT
            prf_server_send_event((prf_env_struct *)&hpss_env, false,
                    hpss_env.hps_shdl + HPS_IDX_STATUS_CODE_VAL);
        }
        //notification not enabled, simply don't send anything
        else
        {
            status = PRF_ERR_NTF_DISABLED;
        }
    }
    else
    {
        status = PRF_ERR_INVALID_PARAM;
    }

    if (status != PRF_ERR_OK)
    {
        // Value has not been sent
        hpss_status_code_send_cfm_send(status);
    }
    return (KE_MSG_CONSUMED);
}
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Disabled State handler definition.
const struct ke_msg_handler hpss_disabled[] =
{
    {HPSS_CREATE_DB_REQ,   (ke_msg_func_t) hpss_create_db_req_handler },
};

/// Idle State handler definition.
const struct ke_msg_handler hpss_idle[] =
{
    {HPSS_ENABLE_REQ,      (ke_msg_func_t) hpss_enable_req_handler },
};

/// Connected State handler definition.
const struct ke_msg_handler hpss_connected[] =
{
    {GATTC_WRITE_CMD_IND,    (ke_msg_func_t) gattc_write_cmd_ind_handler},
	{HPSS_HTTP_SEND_STATUS_REQ,    (ke_msg_func_t) hpss_status_code_send_req_handler},
};

/// Default State handlers definition
const struct ke_msg_handler hpss_default_state[] =
{
    {GAPC_DISCONNECT_IND,   (ke_msg_func_t) gapc_disconnect_ind_handler},
};

/// Specifies the message handler structure for every input state.
const struct ke_state_handler hpss_state_handler[HPSS_STATE_MAX] =
{
    [HPSS_DISABLED]    = KE_STATE_HANDLER(hpss_disabled),
    [HPSS_IDLE]        = KE_STATE_HANDLER(hpss_idle),
    [HPSS_CONNECTED]   = KE_STATE_HANDLER(hpss_connected),
};

/// Specifies the message handlers that are common to all states.
const struct ke_state_handler hpss_default_handler = KE_STATE_HANDLER(hpss_default_state);

/// Defines the place holder for the states of all the task instances.
ke_state_t hpss_state[HPSS_IDX_MAX] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

#endif //CFG_PRF_HPSS

/// @} HPSSTASK
