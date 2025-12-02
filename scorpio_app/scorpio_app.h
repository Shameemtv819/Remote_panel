/**
 * @file scorpio_app.h
 * @author Vetrivel
 * @brief 
 * @date 2025-01-30
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef GAS_PANEL_APP_H
#define GAS_PANEL_APP_H
/******************************************************** Header includes **********************************************************/
#include "main.h"

// middleware includes
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "event_groups.h"
#include "queue.h"
#include "FreeRTOS.h"
#include "time.h"

/******lwip*******/
#include "lwip.h"
#include "tcp.h"
#include "api.h"
#include "sockets.h"
#include "ip_addr.h"
#include "netdb.h"
#include "sntp.h"
#include "ethernetif.h"

#include "inet.h"
#include "udp.h"
#include "pbuf.h"

/***wolfssl***/
#include "wolfssl/wolfcrypt/settings.h"
#include <wolfssl/wolfcrypt/logging.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/types.h>
#include <wolfssl/wolfcrypt/asn.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/ecc.h>

//wolf mqtt
#include "mqtt_client.h"
#include "mqtt_socket.h"

// bsp includes
#include "scorpio_bsp.h"

// app includes
#include "ca_cert.h"
#include "html_page.h"
#include "task.h"
#include "flash.h"
#include "memory_map.h"
#include "lte_app.h"
#include "network_time.h"
#include "mcu_comm_app.h"
#include "json_handler.h"
#include "https_client_app.h"
#include "http_server_app.h"
#include "mqtt.h"
#include "mqtt_app.h"


/*********************************************************** Macros ****************************************************************/

/*********************************************************** typedef ***************************************************************/

/******************************************************** Extern variables *********************************************************/
#endif
