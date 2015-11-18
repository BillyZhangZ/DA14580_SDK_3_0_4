/**
 ****************************************************************************************
 *
 * @file hpss_task.h
 *
 * @brief Header file - HPSSTASK.
 *
 * Copyright (C) zhiyang.zhang 2015-2016
 *
 *
 ****************************************************************************************
 */

#ifndef HPSS_TASK_H_
#define HPSS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup HTTP Proxy Server Task

 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_HPS_SERVER)

#include <stdint.h>
#include "ke_task.h"
#include "hpss.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of HPSS task instances
#define HPSS_IDX_MAX                 (1)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the HPSS task
enum
{
    /// Disabled State
    HPSS_DISABLED,
    /// Idle state
    HPSS_IDLE,
    /// Connected state
    HPSS_CONNECTED,

    /// Number of defined states.
    HPSS_STATE_MAX
};

/// Messages for HPS Sever
enum
{
    /// Start the HPS Sever
    HPSS_ENABLE_REQ = KE_FIRST_MSG(TASK_HPSS),
    /// Disconnected inform
    HPSS_DISABLE_IND,

    /// HTTP Send Status Code Noftification
    HPSS_HTTP_SEND_STATUS_REQ,
	/// HTTP Send Status Code Noftification confirm
	HPSS_HTTP_SEND_STATUS_CFM,

    ///Add a HPS instance into the database
    HPSS_CREATE_DB_REQ,
    ///Inform APP of database creation status
    HPSS_CREATE_DB_CFM,
	///Inform APP of status code notification or indication
	HPSS_CFG_INDNTF_IND,
	///HPS URI Data Indication
	HPSS_URI_IND,
	///HPS Header Data Indication
	HPSS_HEADER_IND,
	///HPS Body Data Indication
	HPSS_BODY_IND,
	///HPS Control Point Data Indication
	HPSS_CNTL_PT_IND,
	///HPS Status code client configuration indication
	HPSS_STATUS_CODE_NTF_CFG_IND,

    /// Error Indication
    HPSS_ERROR_IND,
};


/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref HPSS_ENABLE_REQ message
struct hpss_enable_req
{
    /// Connection Handle
    uint16_t conhdl;
    /// Security level
    uint8_t sec_lvl;

    /// Saved LLS alert level to set in ATT DB
    uint8_t lls_alert_lvl;
    /// TX Power level
    int8_t txp_lvl;
};

/// Parameters of the @ref HPSS_DISABLE_IND message
struct hpss_disable_ind
{
    /// Connection Handle
    uint16_t conhdl;
    /// dummy now
    uint8_t  dummy;
};

/////Parameters of the @ref HPSS_HTTP_SEND_STATUS_REQ message
struct hpss_status_code_send_req
{
    ///Connection handle
    uint16_t conhdl;
    ///Heart Rate measurement
    struct hpss_status_code status_code;
};

///Parameters of the @ref HPSS_HTTP_SEND_STATUS_CFM message
struct hpss_status_code_send_cfm
{
    ///Connection handle
    uint16_t conhdl;
    ///Status
    uint8_t status;
};

/// Parameters of the @ref HPSS_CREATE_DB_REQ message
struct hpss_create_db_req
{
    uint8_t features;
};

/// Parameters of the @ref HPSS_CREATE_DB_CFM message
struct hpss_create_db_cfm
{
    /// Status
    uint8_t status;
};

/// Parameters of the @ref HPSS_URI_IND message
struct hpss_uri_ind
{
	/// Connection handle
	uint16_t conhdl;
	///	Character code
	uint8_t char_code;

	uint16_t len;

	uint8_t uri[HPS_URI_MAX_LEN];
};

/// Parameters of the @ref HPSS_HEADER_IND message
struct hpss_header_ind
{
	/// Connection handle
	uint16_t conhdl;
	///	Character code
	uint8_t char_code;

	uint16_t len;

	uint8_t header[HPS_HEADER_MAX_LEN];
};

/// Parameters of the @ref HPSS_BODY_IND message
struct hpss_body_ind
{
	/// Connection handle
	uint16_t conhdl;
	///	Character code
	uint8_t char_code;

	uint16_t len;

	uint8_t body[HPS_BODY_MAX_LEN];
};

/// Parameters of the @ref HPSS_CNTL_PT_IND message
struct hpss_cntl_pt_ind
{
	/// Connection handle
	uint16_t conhdl;
	///	Character code
	uint8_t char_code;

	uint16_t len;

	uint8_t cntl_pt[HPS_CNTL_PT_MAX_LEN];
};

///Parameters of the @ref HPSS_STATUS_CODE_NTF_CFG message
struct hpss_cfg_indntf_ind
{
    ///Connection handle
    uint16_t conhdl;
    ///Stop/notify/indicate value to configure into the peer characteristic
    uint16_t cfg_val;
};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler hpss_state_handler[HPSS_STATE_MAX];
extern const struct ke_state_handler hpss_default_handler;
extern ke_state_t hpss_state[HPSS_IDX_MAX];

#endif //CFG_PRF_HPSS

/// @} HPSSTASK

#endif // HPSS_TASK_H_
