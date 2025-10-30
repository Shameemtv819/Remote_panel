/**
 * @file bsp_lte.h
 * @author Vetrivel
 * @brief 
 * @date 2025-01-28
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef GSM_H
#define GSM_H
/******************************************************** Header includes **********************************************************/
#include "main.h"

/*********************************************************** Macros ****************************************************************/
#define UART_LTE_RX_BUFF_SIZE    (0x2100)

typedef struct __attribute__((__packed__))
{
  data_frame_def rx_data;
  uint8_t        rx_buff[UART_LTE_RX_BUFF_SIZE];
  uint8_t        rx_count;
  uint8_t        rx_data_len;
} lte_uart_elements_def;
/*********************************************************** typedef ***************************************************************/

/******************************************************** Extern variables *********************************************************/
extern lte_uart_elements_def uart_lte_buff;
/******************************************************* function prototypes *******************************************************/
void      lte_uart_init(void);
void      lte_send_at_cmds(uint8_t tx_cmd[], uint8_t rx_cmd[], uint16_t extra_data, uint16_t tx_len);
void      lte_recv_it(uint8_t rx_cmd[], uint16_t extra_data);
void      lte_peripheral_reset(void);
void 	    get_lte_register_status(uint8_t *buff);
void      lte_recv(uint16_t data_len);
uint8_t   lte_verify_at_rx_cmds(const uint8_t rx_cmd[]);
lte_uart_elements_def * get_lte_rx_buff(void);
UART_HandleTypeDef *get_lte_peripheral(void);


#endif
