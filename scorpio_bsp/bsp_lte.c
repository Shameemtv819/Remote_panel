/**
 * @file bsp_lte.c
 * @author Vetrivel
 * @brief 
 * @date 2025-01-30
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "bsp_lte.h"
/***************************************************************** Macros ********************************************************************/

#if(GSM_ENABLE)
/*************************************************************** Variables *******************************************************************/
static UART_HandleTypeDef UART_LTE;
/************************************************************ static Variables ***************************************************************/
lte_uart_elements_def uart_lte_buff;
/*************************************************************** Functions *******************************************************************/
/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void lte_uart_init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  UART_LTE.Instance = UART_LTE_INSTANCE;
  UART_LTE.Init.BaudRate = UART_LTE_BAUDRATE;
  UART_LTE.Init.WordLength = UART_WORDLENGTH_8B;
  UART_LTE.Init.StopBits = UART_STOPBITS_1;
  UART_LTE.Init.Parity = UART_PARITY_NONE;
  UART_LTE.Init.Mode = UART_MODE_TX_RX;
  UART_LTE.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UART_LTE.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&UART_LTE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}  

void lte_peripheral_reset(void)
{
  HAL_UART_DeInit(&UART_LTE);
  lte_uart_init();
}

UART_HandleTypeDef *get_lte_peripheral(void)
{
  return &UART_LTE;
}

void lte_recv_it(uint8_t rx_cmd[], uint16_t extra_data)
{
  HAL_UART_Abort_IT(&UART_LTE);
  buff_clr((uint8_t *)&uart_lte_buff, sizeof(uart_lte_buff));
  HAL_UART_Receive_IT(&UART_LTE, (uint8_t *)&uart_lte_buff.rx_buff[0], strlen((char *)rx_cmd) + extra_data);
}

/**
 * @brief 
 * @author Vetrivel
 * @param data_len 
 */
void lte_recv(uint16_t data_len)
{
  buff_clr((uint8_t *)&uart_lte_buff, sizeof(uart_lte_buff));
  HAL_UART_Receive(&UART_LTE, &uart_lte_buff.rx_buff[0], data_len, 500u);
}

/**
 * @brief 
 * @author Vetrivel
 * @param tx_cmd 
 * @param rx_cmd 
 * @param extra_data 
 */
void lte_send_at_cmds(uint8_t tx_cmd[], uint8_t rx_cmd[], uint16_t extra_data, uint16_t tx_len)
{
  if(tx_len == 0u)  
  {
    tx_len = strlen((char *)tx_cmd);
  }
  HAL_UART_Abort_IT(&UART_LTE);
  lte_recv(20);
  buff_clr((uint8_t *)&uart_lte_buff, sizeof(uart_lte_buff));
  HAL_UART_Receive_IT(&UART_LTE, &uart_lte_buff.rx_buff[0], (strlen((char *)rx_cmd) + extra_data));
  HAL_UART_Transmit(&UART_LTE, (uint8_t *)tx_cmd, 
                                tx_len, HAL_MAX_DELAY);
  // print_hex((uint8_t *)tx_cmd, tx_len);
}

/**
 * @brief Get the lte rx buff object
 * @author Vetrivel
 * @return uint8_t* 
 */
lte_uart_elements_def * get_lte_rx_buff(void)
{
 return &uart_lte_buff;
}

/**
 * @brief Get the lte register status object
 * @author Vetrivel
 * @param buff 
 */
void get_lte_register_status(uint8_t *buff)
{
  UART_HandleTypeDef *lteuart = get_lte_peripheral();
  sprintf((char *)buff, "\nSR:%d ,DR:%d ,BRR:%d ,CR1:%d ,CR2:%d ,CR3:%d ,GTPR:%d", 
  lteuart->Instance->SR, 
  lteuart->Instance->DR,  
  lteuart->Instance->BRR,
  lteuart->Instance->CR1,
  lteuart->Instance->CR2,
  lteuart->Instance->CR3,
  lteuart->Instance->GTPR);
}

/**
 * @brief 
 * @author Vetrivel
 * @param rx_cmd 
 * @return uint8_t 
 */
uint8_t lte_verify_at_rx_cmds(const uint8_t rx_cmd[])
{
  uint8_t status = ERROR;
  print_hex((uint8_t *)uart_lte_buff.rx_buff, strlen((char*)rx_cmd));
  if(!strncmp((char*)rx_cmd, (char*)uart_lte_buff.rx_buff, strlen((char*)rx_cmd)))
  {
    status = SUCCESS;
  }
  else if(strstr((char*)uart_lte_buff.rx_buff,"\r\nERROR")) 
  {
    status = ERROR;
  }
  else
  {
		status = ERROR;
		// incorrect data received. retry again
  }
  return status;
}
#else
#error "bsp_lte.c file not included"
#endif
