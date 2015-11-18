/**
****************************************************************************************
*
* @file app_task.c
*
* @brief Handling of ble events and responses.
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

#include "app_task.h" 
#include "app.h" 
#include "queue.h" 
#include "console.h" 
#include "gap_task.h" 
#include "proxr_task.h"
#include "diss_task.h"
#include "proxr.h"
#include "proxr.h"
#include "llc_task.h" 
#include "smpc_task.h" 
#include "ble_msg.h" 

extern unsigned int proxm_trans_in_prog;

// application alert state structrure
app_alert_state alert_state;

/**
 ****************************************************************************************
 * @brief Extracts device name from adv data if present and copy it to app_env.
 *
 * @param[in] adv_data      Pointer to advertise data.
 * @param[in] adv_data_len  Advertise data length.
 * @param[in] dev_indx      Devices index in device list.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
void app_find_device_name(unsigned char * adv_data, unsigned char adv_data_len, unsigned char dev_indx)
{
    unsigned char indx = 0;

    while (indx < adv_data_len)
    {
        if (adv_data[indx+1] == 0x09)
        {
            memcpy(app_env.devices[dev_indx].data, &adv_data[indx+2], (size_t) adv_data[indx]);
            app_env.devices[dev_indx].data_len = (unsigned char ) adv_data[indx];
        }
        indx += (unsigned char ) adv_data[indx]+1;
    }

}

/**
 ****************************************************************************************
 * @brief Handles GAPM_CMP_EVT event for GAPM_SET_DEV_CONFIG_CMD.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapm_set_dev_config_completion_handler(ke_msg_id_t msgid,
                                           struct gapm_cmp_evt *param,
                                           ke_task_id_t dest_id,
                                           ke_task_id_t src_id)
{
    app_env.state = APP_CONNECTABLE;

    Sleep(100);

    app_adv_start(); // start advertising

    //ConsoleScan();

    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles ready indication from the GAP.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int gapm_device_ready_ind_handler(ke_msg_id_t msgid,
                                  struct gap_ready_evt *param,
                                  ke_task_id_t dest_id,
                                  ke_task_id_t src_id)
{
    // We are now in Connectable State
    if (dest_id == TASK_APP)
    {
        app_rst_gap();
    }
    
    return 0;
}

/**
 ****************************************************************************************
 * @brief Handles GAPM_ADV_REPORT_IND event.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int gapm_adv_report_ind_handler(ke_msg_id_t msgid,
                                struct gapm_adv_report_ind *param,
                                ke_task_id_t dest_id,
                                ke_task_id_t src_id)
{
    if (app_env.state != APP_SCAN)
        return -1;

    return 0;
}


/**
 ****************************************************************************************
 * @brief Handles Connection request indication event.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
unsigned int start_pair;
int gapc_connection_req_ind_handler(ke_msg_id_t msgid,
                                    struct gapc_connection_req_ind *param,
                                    ke_task_id_t dest_id,
                                    ke_task_id_t src_id)
{
    //start_pair = 1;

    if (app_env.state == APP_IDLE || app_env.state == APP_CONNECTABLE)
    {
        // We are now connected
        app_env.state = APP_CONNECTED;

        // Retrieve the connection index from the GAPC task instance for the connection
        app_env.proxr_device.device.conidx = KE_IDX_GET(src_id);
        
        // Retrieve the connection handle from the parameters
        app_env.proxr_device.device.conhdl = param->conhdl;

        /*
        // On Reconnection check if device is bonded and send pairing request. Otherwise it is not bonded.
        if (bdaddr_compare(&app_env.proxr_device.device.adv_addr, &param->conn_info.peer_addr))
        {
            if (app_env.proxr_device.bonded)
                start_pair = 0;
        }
        */
        memcpy(app_env.proxr_device.device.adv_addr.addr, param->peer_addr.addr, sizeof(struct bd_addr));
        app_env.proxr_device.rssi = 0xFF;
        app_env.proxr_device.txp = 0xFF;
        app_env.proxr_device.llv = 0xFF;                        

        // Send a request to read the LLC TX power so that we can update the PROXR service database.
        {
            struct llc_rd_tx_pw_lvl_cmd * req;
                    
            req = (struct llc_rd_tx_pw_lvl_cmd *) BleMsgAlloc(LLC_RD_TX_PW_LVL_CMD, TASK_LLC, TASK_APP,
                                                sizeof(struct llc_rd_tx_pw_lvl_cmd));
        
            req->conhdl = app_env.proxr_device.device.conhdl;
            req->type = TX_LVL_CURRENT;

            // Send the message
            BleSendMsg((void *) req);
        }

        app_connect_confirm(GAP_AUTH_REQ_NO_MITM_NO_BOND);

        app_dis_enable();

        ConsoleConnected(0);
        //START PAIRING REMOVED FROM HERE
    }

    return 0;
}


/**
 ****************************************************************************************
 * @brief Handles Discconnection indication event.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_disconnect_ind_handler(ke_msg_id_t msgid,
                                struct gapc_disconnect_ind *param,
                                ke_task_id_t dest_id,
                                ke_task_id_t src_id)
{
    if (param->conhdl == app_env.proxr_device.device.conhdl)
    {
        app_send_disconnect(TASK_PROXR, param->conhdl, param->reason);

        app_env.state = APP_IDLE;

        Sleep(100);

        printf("Device Disconnected\n");

        app_set_mode();   
    }

    return 0;
}

/**
 ****************************************************************************************
 * @brief Handles RSSI indication event.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_con_rssi_ind_handler(ke_msg_id_t msgid,
                              struct gapc_con_rssi_ind *param,
                              ke_task_id_t dest_id,
                              ke_task_id_t src_id)
{
    app_env.proxr_device.rssi = param->rssi;
    ConsoleConnected(1);

    return 0;
}


/**
 ****************************************************************************************
 * @brief Handle Bond indication.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_bond_ind_handler(ke_msg_id_t msgid,
                          struct gapc_bond_ind *param,
                          ke_task_id_t dest_id,
                          ke_task_id_t src_id)
{
    switch (param->info)
    {
        case GAPC_PAIRING_SUCCEED:
            if (param->data.auth | GAP_AUTH_BOND)
                app_env.proxr_device.bonded = 1;
                ConsoleConnected(0);
            break;

        case GAPC_IRK_EXCH:
            memcpy (app_env.proxr_device.irk.irk.key, param->data.irk.irk.key, KEY_LEN);
            memcpy (app_env.proxr_device.irk.addr.addr.addr, param->data.irk.addr.addr.addr, KEY_LEN);
            app_env.proxr_device.irk.addr.addr_type = param->data.irk.addr.addr_type;
            
            break;

        case GAPC_LTK_EXCH:
            app_env.proxr_device.ltk.ediv = param->data.ltk.ediv;
            memcpy (app_env.proxr_device.ltk.randnb.nb, param->data.ltk.randnb.nb, RAND_NB_LEN);
            app_env.proxr_device.ltk.key_size = param->data.ltk.key_size;
            memcpy (app_env.proxr_device.ltk.ltk.key, param->data.ltk.ltk.key, param->data.ltk.key_size);
            app_env.proxr_device.ltk.ediv = param->data.ltk.ediv;
            memcpy (app_env.proxr_device.ltk.randnb.nb, param->data.ltk.randnb.nb, RAND_NB_LEN);
            break;

        case GAPC_PAIRING_FAILED:
            app_env.proxr_device.bonded = 0;
            app_disconnect();
            break;
    }

    return 0;
}
 

/**
 ****************************************************************************************
 * @brief Handle reset GAP request completion event.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapm_reset_completion_handler(ke_msg_id_t msgid,
                                  struct gapm_cmp_evt *param,
                                  ke_task_id_t dest_id,
                                  ke_task_id_t src_id)
{
    // We are now in Connectable State
    if (dest_id == TASK_APP)
    {
        app_env.state = APP_IDLE;           
        alert_state.ll_alert_lvl = 2; // Link Loss default Alert Level is high
        alert_state.adv_toggle = 0; // clear advertise toggle
        app_diss_db_create();

        //app_set_mode(); //initialize gap mode 
    }
    
    return 0;
}

/**
 ****************************************************************************************
 * @brief Handles the GAPC_BOND_REQ_IND event.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_bond_req_ind_handler(ke_msg_id_t msgid,
                              struct gapc_bond_req_ind *param,
                              ke_task_id_t dest_id,
                              ke_task_id_t src_id)
{
    app_gap_bond_cfm(param);
    
    return 0;
}

/**
 ****************************************************************************************
 * @brief Handles the GAPC_ENCRYPT_REQ_IND event.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_encrypt_req_ind_handler(ke_msg_id_t const msgid,
                                 struct gapc_encrypt_req_ind * param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
    struct gapc_encrypt_cfm* cfm = BleMsgAlloc(GAPC_ENCRYPT_CFM, src_id, dest_id, sizeof (struct gapc_encrypt_cfm));

    if(((app_env.proxr_device.bonded)
        && (memcmp(&(app_env.proxr_device.ltk.randnb), &(param->rand_nb), RAND_NB_LEN) == 0)
        && (app_env.proxr_device.ltk.ediv == param->ediv)))
    {
        cfm->found = true;
        cfm->key_size = app_env.proxr_device.ltk.key_size;
        memcpy(&(cfm->ltk), &(app_env.proxr_device.ltk.ltk), app_env.proxr_device.ltk.key_size);
        // update connection auth
        app_connect_confirm(GAP_AUTH_REQ_NO_MITM_BOND);
    }
    else
    {
        cfm->found = false;
    }

    BleSendMsg(cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles the GAPC_ENCRYPT_IND event.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_encrypt_ind_handler(ke_msg_id_t const msgid,
                             struct gapc_encrypt_ind *param,
                             ke_task_id_t const dest_id,
                             ke_task_id_t const src_id)
{
    printf("Received GAPC_ENCRYPT_IND auth = %d\n", param->auth);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles Read Tx Power Level response from llc task.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int llc_rd_tx_pw_lvl_cmp_evt_handler(ke_msg_id_t const msgid,
                                     struct llc_rd_tx_pw_lvl_cmd_complete const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    if (param->status == CO_ERROR_NO_ERROR)
    {
        if (app_env.proxr_device.device.conhdl == param->conhdl)  
            alert_state.txp_lvl = param->tx_pow_lvl;
        else
            alert_state.txp_lvl = 0x00;
    }
            
    // Enable the Proximity Reporter profile
    app_proxr_enable();
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles the PROXR_CREATE_DB_CFM message.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int proxr_create_db_cfm_handler(ke_task_id_t const dest_id)
{
    // If state is not idle, ignore the message
    if (app_env.state == APP_IDLE)
    {                                                               
        app_set_mode(); // initialize gap mode 
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles the DISS_CREATE_DB_CFM message.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int diss_create_db_cfm_handler(ke_msg_id_t const msgid,
                               struct diss_create_db_cfm *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id)
{
    uint8_t len = strlen(APP_DIS_SW_REV);    

    if (param->status == CO_ERROR_NO_ERROR)
    {
        // Set Manufacturer Name value in the DB
        {
            struct diss_set_char_val_req *req_name = BleMsgAlloc(DISS_SET_CHAR_VAL_REQ,
                                                                 TASK_DISS, TASK_GTL,
                                                                 sizeof(struct diss_set_char_val_req) + APP_DIS_MANUFACTURER_NAME_LEN);

            // Fill in the parameter structure
            req_name->char_code     = DIS_MANUFACTURER_NAME_CHAR;
            req_name->val_len       = APP_DIS_MANUFACTURER_NAME_LEN;
            memcpy(&req_name->val[0], APP_DIS_MANUFACTURER_NAME, APP_DIS_MANUFACTURER_NAME_LEN);

            // Send the message
            BleSendMsg(req_name);
        }

        // Set Model Number String value in the DB
        {
            struct diss_set_char_val_req *req_mod = BleMsgAlloc(DISS_SET_CHAR_VAL_REQ,
                                                                TASK_DISS, TASK_GTL,
                                                                sizeof(struct diss_set_char_val_req) + APP_DIS_MODEL_NB_STR_LEN);

            // Fill in the parameter structure
            req_mod->char_code     = DIS_MODEL_NB_STR_CHAR;
            req_mod->val_len       = APP_DIS_MODEL_NB_STR_LEN;
            memcpy(&req_mod->val[0], APP_DIS_MODEL_NB_STR, APP_DIS_MODEL_NB_STR_LEN);

            // Send the message
            BleSendMsg(req_mod);
        }

        // Set System ID value in the DB
        {
            struct diss_set_char_val_req *req_id = BleMsgAlloc(DISS_SET_CHAR_VAL_REQ,
                                                               TASK_DISS, TASK_GTL,
                                                               sizeof(struct diss_set_char_val_req) + APP_DIS_SYSTEM_ID_LEN);

            // Fill in the parameter structure
            req_id->char_code     = DIS_SYSTEM_ID_CHAR;
            req_id->val_len       = APP_DIS_SYSTEM_ID_LEN;
            memcpy(&req_id->val[0], APP_DIS_SYSTEM_ID, APP_DIS_SYSTEM_ID_LEN);

            // Send the message
            BleSendMsg(req_id);
        }            
            
        // Set the software version in the DB
        {
            struct diss_set_char_val_req *req_id = BleMsgAlloc(DISS_SET_CHAR_VAL_REQ,
                                                               TASK_DISS, TASK_GTL,
                                                               sizeof(struct diss_set_char_val_req) + len);

            // Fill in the parameter structure
            req_id->char_code     = DIS_SW_REV_STR_CHAR;
            req_id->val_len       = len;
            memcpy(&req_id->val[0], APP_DIS_SW_REV, len);

            // Send the message
            BleSendMsg(req_id);
        }                        
    }

    
    // After DISS DB, create PROXR DB
    if (app_env.state == APP_IDLE)
    {
        app_proxr_db_create();
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles the PROXR_ALERT_IND indication.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
void proxr_alert_ind_handler(ke_msg_id_t msgid,
                             struct proxr_alert_ind *param,
                             ke_task_id_t dest_id,
                             ke_task_id_t src_id)
{
    if (param->alert_lvl == PROXR_ALERT_NONE)
        printf("ALERT STOPPED. Type:%d\n", param->char_code);
    else
        printf("ALERT STARTED. Type:%d Level:%d\n", param->char_code, param->alert_lvl);
}