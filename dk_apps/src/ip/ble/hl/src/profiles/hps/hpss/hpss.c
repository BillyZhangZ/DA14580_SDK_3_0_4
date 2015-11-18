/**
 ****************************************************************************************
 *
 * @file hpss.c
 *
 * @brief HTTP Proxy Server Implementation.
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

#include "hpss.h"
#include "hpss_task.h"
#include "attm_db.h"
#include "gap.h"

/*
 * HPS PROFILE ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

/// HPS Database Description - Used to add attributes into the database
const struct attm_desc hpss_att_db[HPS_IDX_NB] =
{
    // HPS Service Declaration
    [HPS_IDX_SVC]                      =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), sizeof(hps_svc),
                                         sizeof(hps_svc), (uint8_t *)&hps_svc},

    //1  HPS URI Characteristic Declaration
    [HPS_IDX_URI_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(hps_uri_char),
                                         sizeof(hps_uri_char), (uint8_t *)&hps_uri_char},
    //  HPS URI Characteristic Value
    [HPS_IDX_URI_VAL]         =   {ATT_CHAR_HPS_URI, PERM(WR, ENABLE), HPS_URI_MAX_LEN,
                                         0, NULL},

	//2  HPS Header Characteristic Declaration
    [HPS_IDX_HEADER_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(hps_header_char),
                                         sizeof(hps_header_char), (uint8_t *)&hps_header_char},
    //  HPS Header Characteristic Value
    [HPS_IDX_HEADER_VAL]         =   {ATT_CHAR_HPS_HEADER, PERM(RD, ENABLE) | PERM(WR, ENABLE), HPS_HEADER_MAX_LEN,
                                         0, NULL},

	//3  HPS Body Characteristic Declaration
    [HPS_IDX_BODY_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(hps_body_char),
                                         sizeof(hps_body_char), (uint8_t *)&hps_body_char},
    //  HPS Body Characteristic Value
    [HPS_IDX_BODY_VAL]         =   {ATT_CHAR_HPS_BODY, PERM(RD, ENABLE) | PERM(WR, ENABLE), HPS_BODY_MAX_LEN,
																				0, NULL},

    //4  HPS Control Point Characteristic Value                                      
	[HPS_IDX_CNTL_PT_CHAR]         =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(hps_cntl_pt_char),
                                         sizeof(hps_cntl_pt_char), (uint8_t *)&hps_cntl_pt_char},
    //  HPS Control Point Characteristic Value
    [HPS_IDX_CNTL_PT_VAL]          =   {ATT_CHAR_HPS_CNTL_PT, PERM(WR, ENABLE), HPS_CNTL_PT_MAX_LEN,
                                         0, NULL},

	//5  HPS Status Code Characteristic Declaration
    [HPS_IDX_STATUS_CODE_CHAR]     =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(hps_status_code_char),
                                         sizeof(hps_status_code_char), (uint8_t *)&hps_status_code_char},
    //  HPS Status Code Characteristic Value
    [HPS_IDX_STATUS_CODE_VAL]      =   {ATT_CHAR_HPS_STATUS_CODE, PERM(NTF, ENABLE), HPS_STATUS_CODE_LEN,
                                         0, NULL},
    // HPS Status Code Characteristic - Client Characteristic Configuration Descriptor
    [HPS_IDX_STATUS_CODE_CFG]      =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE)|PERM(WR, ENABLE), sizeof(uint16_t),
                                            0, NULL},

	//6  HPS Security Characteristic Declaration
    [HPS_IDX_SECURITY_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(hps_security_char),
                                         sizeof(hps_security_char), (uint8_t *)&hps_security_char},
    //  HPS Security Characteristic Value
    [HPS_IDX_SECURITY_VAL]         =   {ATT_CHAR_HPS_SECURITY, PERM(RD, ENABLE), HPS_SECURITY_LEN,
                                         0, NULL},

};

/*
 *  HTTP Proxy PROFILE ATTRIBUTES VALUES DEFINTION
 ****************************************************************************************
 */

///HTTP Proxy Servicee
const att_svc_desc_t hps_svc     = ATT_SVC_HPS;

/// HTTP Proxy Service - URI Characteristic
const struct att_char_desc hps_uri_char = ATT_CHAR(ATT_CHAR_PROP_WR,
                                                                    0,
                                                                    ATT_CHAR_HPS_URI);
/// HTTP Proxy Service - Header Characteristic
const struct att_char_desc hps_header_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR,
                                                                    0,
                                                                    ATT_CHAR_HPS_HEADER);
/// HTTP Proxy Service - Body Characteristic
const struct att_char_desc hps_body_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR,
                                                                    0,
                                                                    ATT_CHAR_HPS_BODY);
/// HTTP Proxy Service - Control Point Characteristic
const struct att_char_desc hps_cntl_pt_char = ATT_CHAR(ATT_CHAR_PROP_WR,
                                                                    0,
                                                                    ATT_CHAR_HPS_CNTL_PT);
/// HTTP Proxy Service - Status Code Characteristic
const struct att_char_desc hps_status_code_char = ATT_CHAR(ATT_CHAR_PROP_NTF,
                                                                    0,
                                                                    ATT_CHAR_HPS_STATUS_CODE);
/// HTTP Proxy Service - Security Characteristic
const struct att_char_desc hps_security_char = ATT_CHAR(ATT_CHAR_PROP_RD,
                                                                    0,
                                                                    ATT_CHAR_HPS_SECURITY);
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

struct hpss_env_tag hpss_env __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

/// HPSS task descriptor
static const struct ke_task_desc TASK_DESC_HPSS = {hpss_state_handler, &hpss_default_handler, hpss_state, HPSS_STATE_MAX, HPSS_IDX_MAX};




/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void hpss_init(void)
{
    // Reset Environment
    memset(&hpss_env, 0, sizeof(hpss_env));

    // Create HPSS task
    ke_task_create(TASK_HPSS, &TASK_DESC_HPSS);

    ke_state_set(TASK_HPSS, HPSS_DISABLED);
}

void hpss_disable(uint16_t conhdl) 
{
    // Disable HPS in database
    attmdb_svc_set_permission(hpss_env.hps_shdl, PERM_RIGHT_DISABLE);

    struct hpss_disable_ind *ind = KE_MSG_ALLOC(HPSS_DISABLE_IND,
                                                 hpss_env.con_info.appid, TASK_HPSS,
                                                 hpss_disable_ind);

    // Fill in the parameter structure
    ind->conhdl            = conhdl;

    // Send the message
    ke_msg_send(ind);

    // Go to idle state
    ke_state_set(TASK_HPSS, HPSS_IDLE);
}

void hpss_status_code_send_cfm_send(uint8_t status)
{
    // Send CFM to APP that value has been sent or not
    struct hpss_status_code_send_cfm * cfm = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_CFM, hpss_env.con_info.appid,
                                                   TASK_HPSS, hpss_status_code_send_cfm);

    cfm->conhdl = gapc_get_conhdl(hpss_env.con_info.conidx);
    cfm->status = status;

    ke_msg_send(cfm);
}

#endif //CFG_PRF_HPSS

/// @} HPSS
