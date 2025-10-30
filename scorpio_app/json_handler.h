/**
 * @file json_handler.h
 * @author Shameem
 * @brief This file contains function prototypes which are called in http_server_app.c and https_client_app.c
 * @date 2025-05-09
 * @copyright Copyright (c) 2025
 */

/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

/************************************************************* Header includes ***************************************************************/
#include "scorpio_app.h"

/******************************************************* function prototypes ******************************************************/
char* extract_json_payload(const uint8_t *request);

void segregate_cert_url_json_payload(const uint8_t *json_payload);
void segregate_IP_config_json_payload(const uint8_t *json_payload);
void segregate_GSM_conf_json_payload(const uint8_t *json_payload);

uint32_t get_content_length(const uint8_t *request,const char *fetch_str);

#endif

