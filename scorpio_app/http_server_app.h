/**
 * @file http_server_app.h
 * @author shameem
 * @brief Contains all typedef used in config of IP/gsm gateway device
 * @date 2025-02-03
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef HTTP_SERVER_APP_H
#define HTTP_SERVER_APP_H

/******************************************************** Header includes **********************************************************/
#include "scorpio_app.h"

/******************************************************* Function prototypes *******************************************************/

/*********************************************************** typedef ***************************************************************/
typedef struct __attribute__((__packed__))
{
    uint8_t scorpio_version[30];
    uint8_t gemini_version[50];
    uint8_t gsm_number[11];
} scorpio_version;

typedef struct __attribute__((__packed__))
{
    uint8_t ip_enable;
    uint8_t ip_addr[20];
    uint8_t gate_way[20];
    uint8_t net_mask[20];
} ip_config;

typedef struct __attribute__((__packed__))
{
    uint8_t u8_gsm_enable;
    uint8_t i8_phone_no1[5][15];
} gsm_config;

typedef struct __attribute__((__packed__))
{
    uint8_t cert[3000];
    uint8_t url[50];
} cert_url;

/**
 * @brief
 * size :
 * 61    ip_config
 * 56    gsm_config
 * 3050  cert_url
 * 53    scorpio_version = 3220 bytes
 * @author Vetrivel
 * @return typedef struct
 */
typedef struct __attribute__((__packed__))
{
    ip_config IP;
    gsm_config GSM;
    cert_url https;
    scorpio_version version;
} scp_cnf;

extern scp_cnf *pf_config;

void http_server_serve(struct netconn *conn);

#endif /* __HTTPSERVER_NETCONN_H__ */
