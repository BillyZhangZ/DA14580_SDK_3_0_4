/**
 ****************************************************************************************
 *
 * @file app_hps_task.h
 *
 * @brief Header file - APPDISTASK.
 *
 * Copyright (C) zhiyang.zhang 2015-2016
 *
 *
 ****************************************************************************************
 */

#ifndef APP_HPS_TASK_H_
#define APP_HPS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup APPHPSTASK Task
 * @ingroup APPHPS
 * @brief HTTP Proxy Service Application Task
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_HPS_SERVER)

#include "hpss_task.h"
#include "ke_task.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximal number of APP DIS Task instances
#define APP_HPS_IDX_MAX        (1)

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles HPS Server profile database creation confirmation.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int hpss_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct hpss_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles disable indication from the HPS Server profile.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int hpss_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct hpss_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);                                      
/**
 ****************************************************************************************
 * @brief Handles URI indication from the HPS Server profile.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int hpss_uri_ind_handler(ke_msg_id_t const msgid,
                                      struct hpss_uri_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
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
                                      ke_task_id_t const src_id);
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
                                      ke_task_id_t const src_id);
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
                                      ke_task_id_t const src_id);
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
                                      ke_task_id_t const src_id);

#endif //(BLE_HPS_SERVER)

/// @} APPHPSTASK

#endif //APP_HPS_TASK_H_
