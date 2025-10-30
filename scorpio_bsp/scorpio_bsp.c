/**
 * @file scorpio_bsp.c
 * @author Vetrivel
 * @brief 
 * @date 2025-01-30
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "scorpio_bsp.h"
/************************************************************** weak functions ***************************************************************/
__weak void      lte_uart_init(void) {}
__weak void lte_send_at_cmds(uint8_t tx_cmd[], uint8_t rx_cmd[], uint16_t extra_data, uint16_t tx_len){}
__weak uint8_t   lte_verify_at_rx_cmds(const uint8_t rx_cmd[]){ return ERROR;}
__weak lte_uart_elements_def * get_lte_rx_buff(void){ return NULL; }

__weak void debug_uart_init(void){}
__weak void debug_msg(uint8_t *buff){}
__weak void print_hex(uint8_t *buff, uint16_t message_len){}
__weak void debug_receive_it(uint8_t *pData, uint16_t size){}
