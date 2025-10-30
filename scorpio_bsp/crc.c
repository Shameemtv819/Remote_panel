/**
 * @file crc.c
 * @author Vetrivel
 * @brief 
 * @date 2025-02-13
 * @copyright Copyright (c) 2025
 */

/************************************************************* Header includes ***************************************************************/
#include "crc.h"
/***************************************************************** Macros ********************************************************************/

/*************************************************************** Variables *******************************************************************/
/************************************************************ static Variables ***************************************************************/
static CRC_HandleTypeDef hcrc;
/*************************************************************** Functions *******************************************************************/
/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
*/
void crc_init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

 /**
  * @brief 
  * @author Vetrivel
  * @param pBuffer 
  * @param BufferLength 
  * @return uint32_t 
  */
uint32_t calculate_crc(uint32_t pBuffer[], uint32_t BufferLength)
{
#if(HARDWARE_CRC_ENABLE)
  return HAL_CRC_Calculate(&hcrc, pBuffer,BufferLength);	
#else
  return crc16((uint8_t*)pBuffer, BufferLength);
#endif
}
