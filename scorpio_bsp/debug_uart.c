/**
 * @file debug_uart.c
 * @author Vetrivel
 * @brief 
 * @date 2025-01-30
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "debug_uart.h"

#if(DEBUG_ENABLE)
/*************************************************************** Variables *******************************************************************/
UART_HandleTypeDef UART_DEBUG;

/*************************************************************** Functions *******************************************************************/
/**
  * @brief UART_MCU_COMM_INSTANCE Initialization Function
  * @param None
  * @retval None
  */
void debug_uart_init(void)
{

  /* USER CODE BEGIN UART_MCU_COMM_INSTANCE_Init 0 */

  /* USER CODE END UART_MCU_COMM_INSTANCE_Init 0 */



    UART_DEBUG.Instance = USART2;
    UART_DEBUG.Init.BaudRate = 115200;
    UART_DEBUG.Init.WordLength = UART_WORDLENGTH_8B;
    UART_DEBUG.Init.StopBits = UART_STOPBITS_1;
    UART_DEBUG.Init.Parity = UART_PARITY_NONE;
    UART_DEBUG.Init.Mode = UART_MODE_TX_RX;
    UART_DEBUG.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UART_DEBUG.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&UART_DEBUG) != HAL_OK)
    {
        Error_Handler();  // Define your error handler
    }
  /* USER CODE BEGIN UART_MCU_COMM_INSTANCE_Init 1 */

  /* USER CODE END UART_MCU_COMM_INSTANCE_Init 1 */
  UART_DEBUG.Instance = UART_DEBUG_INSTANCE;
  UART_DEBUG.Init.BaudRate = UART_DEBUG_BAUDRATE;
  UART_DEBUG.Init.WordLength = UART_WORDLENGTH_8B;
  UART_DEBUG.Init.StopBits = UART_STOPBITS_1;
  UART_DEBUG.Init.Parity = UART_PARITY_NONE;
  UART_DEBUG.Init.Mode = UART_MODE_TX_RX;
  UART_DEBUG.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UART_DEBUG.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&UART_DEBUG) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART_MCU_COMM_INSTANCE_Init 3 */

  /* USER CODE END UART_MCU_COMM_INSTANCE_Init 3 */

}

/**
 * @brief Get the debug peripheral object
 * @author Vetrivel
 * @return UART_HandleTypeDef* 
 */
UART_HandleTypeDef *get_debug_peripheral(void)
{
  return &UART_DEBUG;
}

/**
 * @brief 
 * @author Vetrivel
 * @param buff 
 */
void debug_msg(uint8_t *buff)
{
  if(buff != NULL)
  {
   HAL_UART_Transmit(&UART_DEBUG, (uint8_t *)buff, 
               strlen((char *)buff), HAL_MAX_DELAY);
  }
}

/**********************************************************************************
  * @brief This function print the string through UART_MCU_COMM_INSTANCE and switc into next line 
  * @param char *msg - pointer to the string to be printed
  * @retval None
**********************************************************************************/
void debug_println(uint8_t *msg) 
{
  HAL_UART_Transmit(&UART_DEBUG, (uint8_t*)msg, strlen((char *)msg), HAL_MAX_DELAY);
  HAL_UART_Transmit(&UART_DEBUG, (uint8_t*)"\r\n", strlen("\r\n"), HAL_MAX_DELAY);
}

/**
 * @brief 
 * @author Vetrivel
 * @param size 
 */
void debug_receive_it(uint8_t *pData, uint16_t size)
{
  HAL_UART_Abort_IT(&UART_DEBUG);
  HAL_UART_Receive_IT(&UART_DEBUG, (uint8_t *)pData, size); // +4 bytes = crc, EOF
}

/**
 * @brief 
 * @author Vetrivel
 * @param buff 
 */
void print_hex(uint8_t *buff, uint16_t message_len)
{
  HAL_UART_Transmit(&UART_DEBUG, (uint8_t *)buff, 
                                  message_len, HAL_MAX_DELAY);
}

#else
#warning "debug_uart.c file not included"
#endif
