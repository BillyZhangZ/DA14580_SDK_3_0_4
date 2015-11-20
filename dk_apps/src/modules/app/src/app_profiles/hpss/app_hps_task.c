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

//#define TEST_NTF
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

static int8_t construct_http_request(char *request, const char *uri, const char *header, const char *body, int8_t method);

static uint8_t http_uri[HPS_URI_MAX_LEN] = {0}, http_header[HPS_HEADER_MAX_LEN], http_body[HPS_BODY_MAX_LEN];
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
	memset(http_uri, 0, HPS_URI_MAX_LEN);
	memcpy(http_uri, param->uri, param->len);
#ifdef TEST_NTF
	arch_printf("receive http uri :%s\n", param->uri);
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
	memset(http_header, 0, HPS_HEADER_MAX_LEN);
	memcpy(http_header, param->header, param->len);
#ifdef TEST_NTF
	arch_printf("receive http header :%s\n", param->header);
  	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ, 
														TASK_HPSS,
														TASK_APP,
														hpss_status_code_send_req);

	req->conhdl = param->conhdl;
	req->status_code.status_code = 0x55AA;
	req->status_code.data_status = 0x02;
	ke_msg_send(req);
#endif
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
	memset(http_body, 0, HPS_BODY_MAX_LEN);
	memcpy(http_body, param->body, param->len);
#ifdef TEST_NTF
	arch_printf("receve http body :%s\n", param->body);
  	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ, 
														TASK_HPSS,
														TASK_APP,
														hpss_status_code_send_req);

	req->conhdl = param->conhdl;
	req->status_code.status_code = 0x55AA;
	req->status_code.data_status = 0x03;
	ke_msg_send(req);
#endif
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
	int8_t request[HPS_URI_MAX_LEN + HPS_HEADER_MAX_LEN + HPS_BODY_MAX_LEN] = {0};//fix me, maybe length is not reasonable
	request[0] = 'A';
	request[1] = 'T';
	request[2] = '+';
	construct_http_request((char *)&request + 3, (const char *)http_uri, (const char *)http_header, (const char *)http_body, HTTP_GET);
	///assume '\0' is end
	request[strlen(request)] = '\r';
	request[strlen(request)] = '\n';
	arch_printf("%s", request);
#ifdef TEST_NTF
	arch_printf("receive http control porint :%u\n",param->cntl_pt[0]);
  	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ, 
														TASK_HPSS,
														TASK_APP,
														hpss_status_code_send_req);

	req->conhdl = param->conhdl;
	req->status_code.status_code = 0x55AA;
	req->status_code.data_status = 0x04;
	ke_msg_send(req);
#endif
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
#ifdef TEST_NTF
	arch_printf("Set client configuration :%d\n",param->cfg_val);
  	struct hpss_status_code_send_req *req = KE_MSG_ALLOC(HPSS_HTTP_SEND_STATUS_REQ, 
														TASK_HPSS,
														TASK_APP,
														hpss_status_code_send_req);

	req->conhdl = param->conhdl;
	req->status_code.status_code = 0x01;
	req->status_code.data_status = 0x02;
	ke_msg_send(req);
#endif
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Construct http request with uri, header, body.
 *
 * @param[out] request     constructed http request string.
 * @param[in] uri     http uri string.
 * @param[in] header  http header string.
 * @param[in] method    http method.
 *
 * @return status, 0 succeed, -1 error.
 ****************************************************************************************
 */
static int8_t construct_http_request(char *request, const char *uri, const char *header, const char *body, int8_t method)
{
	 if(request == NULL) return -1;

	 switch(method)
	 {
	 	case HTTP_GET:
			strcat(request, uri);
		break;
		#if 0
		case HTTP_POST:
		break;
		case HTTP_PUT:
		break;
		case HTTP_DELETE:
		break;
		#endif
		default:
		//not supported currently
		break;
	 }
	 return 0;
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

#endif //(BLE_HPS_SERVER)

/// @} APP
