/**
 * @file bsp_relay.c
 * @author shameem
 * @brief
 * @date 2025-06-04
 * @copyright Copyright (c) 2025
 */

/************************************************************* Header includes ***************************************************************/
#include "bsp_relay.h"
/***************************************************************** Macros ********************************************************************/

/*************************************************************** Variables *******************************************************************/
GPIO_TypeDef *realy_port[2] = {RL_1_PORT,RL_2_PORT};
uint16_t relay_pins[2]       = {RL_1_PIN,RL_2_PIN};
/************************************************************ static Variables ***************************************************************/

/*************************************************************** Functions *******************************************************************/


/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
void relay_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(RL_1_PORT, RL_1_PIN, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(RL_2_PORT, RL_2_PIN, GPIO_PIN_RESET);

    /*Configure GPIO pins : relay 1 */
    GPIO_InitStruct.Pin = RL_1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(RL_1_PORT, &GPIO_InitStruct);

    /*Configure GPIO pin : relay 2 */
    GPIO_InitStruct.Pin = RL_2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(RL_2_PORT, &GPIO_InitStruct);
}

 /**
  * @brief This funtion will change state of relay as per the param.
  * @param u8_relay_no : relay number like RELAY_1 or RELAY_2
  * @param u8_state    : GPIO_PIN_SET   = NO (normally open)
  *                      GPIO_PIN_RESET = NC (normally closed)
  * @retval None
  */
 void change_relay_state(uint8_t u8_relay_no,uint8_t u8_state)
 {
    HAL_GPIO_WritePin((GPIO_TypeDef*)realy_port[u8_relay_no], relay_pins[u8_relay_no], (GPIO_PinState)u8_state);
 }
 