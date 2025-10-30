/**
 * @file flash.h
 * @author Vetrivel
 * @brief 
 * @date 2025-05-12
 * @copyright Copyright (c) 2025
 */
 /************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef FLASH_H
#define FLASH_H
/******************************************************** Header includes **********************************************************/
#include "scorpio_app.h"
/*********************************************************** Macros ****************************************************************/
/*********************************************************** typedef ***************************************************************/
typedef struct flash_elements
{
  uint32_t address;
} flash_elements_def;

void flash_init(void);
void flash_task_process(uint8_t service, uint8_t *tx_data, uint16_t tx_data_len);

#endif
