/**
 * @file memory_map.h
 * @author Vetrivel
 * @brief 
 * @date 2025-05-12
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H
/******************************************************** Header includes **********************************************************/
#include "scorpio_app.h"
/*********************************************************** Macros ****************************************************************/

/*********************************************************** typedef ***************************************************************/
#define SIZE_128KB                                 (0x20000)
#define SIZE_256KB                                 (0x40000)
#define SIZE_512KB                                 (0x80000)
#define SIZE_1024KB                                (0x100000)

#define INT_FLASH_BASE                             (0X08000000UL)
#define INT_FLASH_END                              (INT_FLASH_BASE + SIZE_1024KB)

#define INT_FLASH_SSL_CERT_ADDR_SIZE               (INT_FLASH_BASE + SIZE_1024KB - SIZE_128KB) // last sector
#define INT_FLASH_SSL_CERT_ADDR                    (INT_FLASH_SSL_CERT_ADDR_SIZE + 2U) // 0x080E0002

#endif
