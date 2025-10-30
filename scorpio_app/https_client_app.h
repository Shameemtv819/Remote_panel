/**
 * @file    https_client_app.h
 * @author Shameem
 * @brief  This file contains function prototypes which have scope globally
 * @date   2025-05-09
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef HTTPS_CLIENT_APP_H
#define HTTPS_CLIENT_APP_H

/************************************************************* Header includes ***************************************************************/
#include "scorpio_app.h"


/******************************************************* function prototypes ******************************************************/
void wolf_ssl_task(void );
void ip_task_process(uint8_t service, uint8_t *tx_data, uint16_t tx_data_len);
void generate_GET_request(uint8_t *ac_sendline, uint8_t *server_file_path);
int32_t vsocketmanagertask(void );
void netif_status_callback(struct netif *netif);
int32_t Msocketmanagertask(void);


#endif /* __MAIN_H */
