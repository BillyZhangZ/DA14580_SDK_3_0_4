/**
 ****************************************************************************************
 *
 * @file hpss.h
 *
 * @brief Header file - HPSS.
 *
 * Copyright (C) zhiyang.zhang 2015-2016
 *
 *
 ****************************************************************************************
 */

#ifndef HPSS_H_
#define HPSS_H_

/**
 ****************************************************************************************
 * @addtogroup HTTP Proxy Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_HPS_SERVER)

#include "ke_task.h"
#include "atts.h"
#include "prf_types.h"


#define HPS_URI_MAX_LEN 			512
#define HPS_HEADER_MAX_LEN			256
#define HPS_BODY_MAX_LEN			256
#define HPS_CNTL_PT_MAX_LEN			1
#define HPS_STATUS_CODE_LEN			3
#define HPS_SECURITY_LEN			1

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// HPS Handles offsets
enum
{
    HPS_IDX_SVC,

    HPS_IDX_URI_CHAR,
    HPS_IDX_URI_VAL,

	HPS_IDX_HEADER_CHAR,
	HPS_IDX_HEADER_VAL,

	HPS_IDX_BODY_CHAR,
	HPS_IDX_BODY_VAL,

	HPS_IDX_CNTL_PT_CHAR,
	HPS_IDX_CNTL_PT_VAL,

	HPS_IDX_STATUS_CODE_CHAR,
	HPS_IDX_STATUS_CODE_VAL,
	HPS_IDX_STATUS_CODE_CFG,

	HPS_IDX_SECURITY_CHAR,
	HPS_IDX_SECURITY_VAL,

    HPS_IDX_NB,
};

///Characteristics Code for Write Indications
enum
{
    HPS_ERR_CHAR,
    HPS_URI_CHAR,
    HPS_HEADER_CHAR,
	HPS_BODY_CHAR,
	HPS_CNTL_PT_CHAR,
	HPS_STATUS_CODE_CHAR,
	HPS_SECURITY_CHAR,
};

enum
{
    HPSS_STATUS_CODE_NTF_CFG       = 0x01,
};

/// Pointer to the connection clean-up function											  
#define HPS_CLEANUP_FNCT        (NULL)

#define HPSS_IS_SUPPORTED(mask) \
    (((hpss_env.features & mask) == mask))
    

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// HPS server environment variable
struct hpss_env_tag
{
    /// Connection Information
    struct prf_con_info con_info;

    /// HPS Start Handle
    uint16_t hps_shdl;
	
	///Database configuration
    uint8_t features;
};

struct hpss_status_code
{
	uint16_t status_code;
	uint8_t  data_status;
};
/*
 * HPS PROFILE ATTRIBUTES DECLARATION
 ****************************************************************************************
 */

/// HPS Database Description
extern const struct attm_desc hpss_att_db[HPS_IDX_NB];

/*
 *  HPS PROFILE ATTRIBUTES VALUES DECLARATION
 ****************************************************************************************
 */

/// HTTP Proxy Service
extern const att_svc_desc_t hps_svc;
///  HTTP Proxy Service - URI Characteristic
extern const struct att_char_desc hps_uri_char;
///  HTTP Proxy Service - Header Characteristic
extern const struct att_char_desc hps_header_char;
/// HTTP Proxy Service - Body Characteristic
extern const struct att_char_desc hps_body_char;
/// HTTP Proxy Service - Control Point Characteristic
extern const struct att_char_desc hps_cntl_pt_char;
/// HTTP Proxy Service - Status Code Characteristic
extern const struct att_char_desc hps_status_code_char;
/// HTTP Proxy Service - Security Characteristic
extern const struct att_char_desc hps_security_char;

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern struct hpss_env_tag hpss_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the HPSS module.
 * This function performs all the initializations of the HPSS module.
 ****************************************************************************************
 */
void hpss_init(void);

/**
 ****************************************************************************************
 * @brief Disable role.
 ****************************************************************************************
 */
void hpss_disable(uint16_t conhdl); 

#endif //CFG_PRF_HPSS

/// @} HPSS

#endif // HPSS_H_
