/**
 * @file mcu_comm_app.c
 * @author Vetrivel
 * @brief 
 * @date 2025-02-13
 * @copyright Copyright (c) 2025
 */

/************************************************************* Header includes ***************************************************************/
#include "mcu_comm_app.h"
/***************************************************************** Macros ********************************************************************/

/************************************************************* static variables **************************************************************/
/**************************************************************** Variables ******************************************************************/
static uint8_t tx_buff[UART_LTE_RX_BUFF_SIZE] = {0};
/************************************************************ function prototypes ************************************************************/
/************************************************************ function definitions ***********************************************************/

/**
 * @brief This function is used to transmit data frame through debug uart. This comm. port 
 *        need to be changed to master /slave controller comm. port.      
 * @author Vetrivel
 */
void send_data_frame(uint16_t u16_cmd, uint16_t u16_payload_len, uint8_t *pu8_payload, uint16_t u16_packet_num)
{
#if (!DEV_DEBUG_PROTOCOL)
  data_frame_def st_data_frame    = {0};
  st_data_frame.u16_sof           = MCU_COMM_SOF;
  st_data_frame.u16_cmd		        = u16_cmd;														
  st_data_frame.u16_payload_len	  = u16_payload_len;	
  st_data_frame.u16_packet_num    = u16_packet_num;

  buff_cpy(tx_buff, (uint8_t *)&st_data_frame, 8u, 8u);
  if(u16_payload_len != NULL)
  {
    buff_cpy(&tx_buff[8u], pu8_payload, u16_payload_len, u16_payload_len);
  }

  st_data_frame.pu8_payload       = pu8_payload; // debug purpose
  st_data_frame.u16_crc           = calculate_crc((uint32_t*)&tx_buff,((uint32_t)u16_payload_len + 8u));
  st_data_frame.u16_eof	          = MCU_COMM_EOF;	

  buff_cpy(&tx_buff[8u + u16_payload_len], (uint8_t *)&st_data_frame.u16_crc, 4u, 4u);
  mcu_comm_transmit((uint8_t *)&tx_buff, ((uint32_t)u16_payload_len + 8u + 4u)); 
#endif
}
