/**
 * @file debug_uart.h
 * @author Vetrivel
 * @brief 
 * @date 2025-01-28
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef DEBUG_UART_H
#define DEBUG_UART_H
/******************************************************** Header includes **********************************************************/
#include "main.h"

/*********************************************************** Macros ****************************************************************/

/*********************************************************** typedef ***************************************************************/

/******************************************************** Extern variables *********************************************************/

/******************************************************* function prototypes *******************************************************/
void debug_uart_init(void);
void debug_msg(uint8_t *buff);
void debug_println(uint8_t *msg);
void print_hex(uint8_t *buff, uint16_t message_len);
void debug_receive_it(uint8_t *pData, uint16_t size);
UART_HandleTypeDef *get_debug_peripheral(void);

#endif
