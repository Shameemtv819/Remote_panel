/**
 * @file mcu_comm_uart.c
 * @author Vetrivel
 * @brief 
 * @date 2025-03-18
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "mcu_comm_uart.h"

#if(MCU_COMM_ENABLE)
/*************************************************************** Variables *******************************************************************/
UART_HandleTypeDef UART_MCU_COMM;
uart_elements_def uart_mcu_comm;

/*************************************************************** Functions *******************************************************************/
/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
void mcu_comm_uart_init(void)
 {
 
  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  UART_MCU_COMM.Instance = UART_MCU_COMM_INSTANCE;
  UART_MCU_COMM.Init.BaudRate = UART_MCU_COMM_BAUDRATE;
  UART_MCU_COMM.Init.WordLength = UART_WORDLENGTH_8B;
  UART_MCU_COMM.Init.StopBits = UART_STOPBITS_1;
  UART_MCU_COMM.Init.Parity = UART_PARITY_NONE;
  UART_MCU_COMM.Init.Mode = UART_MODE_TX_RX;
  UART_MCU_COMM.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UART_MCU_COMM.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&UART_MCU_COMM) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */
  HAL_UART_Receive_IT(&UART_MCU_COMM, uart_mcu_comm.rx_buff, 1u);
  /* USER CODE END USART6_Init 2 */
}

/**
 * @brief Get the mcu comm peripheral object
 * @author Vetrivel
 * @return UART_HandleTypeDef* 
 */
UART_HandleTypeDef *get_mcu_comm_peripheral(void)
{
  return &UART_MCU_COMM;
}

/**
 * @brief 
 * @author Vetrivel
 * @param size 
 */
void mcu_comm_receive_it(uint8_t *pData, uint16_t size)
{
  HAL_UART_Abort_IT(&UART_MCU_COMM);
  HAL_UART_Receive_IT(&UART_MCU_COMM, (uint8_t *)pData, size); // +4 bytes = crc, EOF
}

/**
 * @brief 
 * @author Vetrivel
 * @param buff 
 */
void mcu_comm_transmit(uint8_t *buff, uint16_t message_len)
{
	buff_clr((uint8_t *)&uart_mcu_comm, sizeof(uart_mcu_comm));
  mcu_comm_receive_it((uint8_t *)uart_mcu_comm.rx_buff, 1u);
  HAL_UART_Transmit(&UART_MCU_COMM, (uint8_t *)buff, 
                                 message_len, HAL_MAX_DELAY);
  //print_hex(buff, message_len);
}

#else
#error "mcu_comm_uart.c file not included"
#endif
