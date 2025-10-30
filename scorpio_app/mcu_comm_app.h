/**
 * @file mcu_comm_app.h
 * @author Vetrivel
 * @brief 
 * @date 2025-02-13
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion *******************************************/
#ifndef MCU_COMM_APP_H
#define MCU_COMM_APP_H
/******************************************************** Header includes **********************************************************/
#include "scorpio_app.h"

/*********************************************************** Macros ****************************************************************/
#define MCU_COMM_SOF                      (0XAA55)
#define MCU_COMM_EOF                      (0XEEBB)
#define MCU_COMM_CMD_FLASH_DELETE         (0X3000)
#define MCU_COMM_CMD_FW_UPG               (0X3001)
#define MCU_COMM_CMD_FW_UPG_NACK          (0X3003)

#define MCU_COMM_IP_CMD_FLASH_DELETE         (0X4000)

/*********************************************************** typedef ***************************************************************/
/******************************************************** Extern variables *********************************************************/
/******************************************************* function prototypes *******************************************************/
void send_data_frame(uint16_t u16_cmd, uint16_t u16_payload_len, uint8_t *pu8_payload, uint16_t u16_packet_num);

#endif
