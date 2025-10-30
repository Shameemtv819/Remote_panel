/**
 * @file scorpio_app.c
 * @author Vetrivel
 * @brief 
 * @date 2025-01-30
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "scorpio_app.h"

/************************************************************** weak functions ***************************************************************/
__weak void lte_init(void){}
__weak void lte_pdp_activate(void){}
__weak void lte_task_process(uint8_t service, uint8_t *tx_data, uint16_t tx_data_len){}

__weak void send_data_frame(uint16_t u16_cmd, uint16_t u16_payload_len, uint8_t *pu8_payload, uint16_t u16_packet_num){}
