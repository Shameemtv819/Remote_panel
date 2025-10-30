/**
 * @file mcu_comm_uart.h
 * @author Vetrivel
 * @brief 
 * @date 2025-03-18
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef MCU_COMM_UART_H
#define MCU_COMM_UART_H
/******************************************************** Header includes **********************************************************/
#include "main.h"

/*********************************************************** Macros ****************************************************************/

/*********************************************************** typedef ***************************************************************/
typedef struct __attribute__((__packed__))
{
  data_frame_def rx_data;
  uint8_t rx_buff[100];
  uint8_t rx_count;
  uint8_t rx_data_len;
} uart_elements_def;

/******************************************************** Extern variables *********************************************************/
extern uart_elements_def uart_mcu_comm;

/******************************************************* function prototypes *******************************************************/
void mcu_comm_uart_init(void);
void mcu_comm_receive_it(uint8_t *pData, uint16_t size);
void mcu_comm_transmit(uint8_t *buff, uint16_t message_len);
UART_HandleTypeDef *get_mcu_comm_peripheral(void);

#endif
