/**
 * @file common.h
 * @author Vetrivel
 * @brief 
 * @date 2025-01-30
 * @copyright Copyright (c) 2025
 */
/************************************************* Define to prevent recursive inclusion ********************************************/
#ifndef COMMON_H
#define COMMON_H
/********************************************************* Header includes **********************************************************/
#include "main.h"

/************************************************************ Macros ****************************************************************/
/* base_definitions */
#define BASE10	                                                		(10)
#define BASE16	                                                		(16)
#define ASCII_NUMBERS_START_VAL																			(48U)

#define RANDOM_NUM_WITHIN_RANGE(RAN_NUM, MIN_RANGE, MAX_RANGE)	(((RAN_NUM) % (((MAX_RANGE) - (MIN_RANGE)) +0x1U))+ MIN_RANGE)

/*This will return the size of decimal value based on the range*/
#define SIZE_OF_DECIMAL_VAL(DEC_VAL)    														( ( DEC_VAL<=0xFF       )  ? 1 :   \
																																			( DEC_VAL<=0xFFFF     )  ? 2 :   \
																																			( DEC_VAL<=0xFFFFFFFF )  ? 4 : 8 \
																																		)

/*********************************************************** typedef ****************************************************************/
/****************************************************** function prototypes *********************************************************/

void 				string_reverse(char str[], int32_t length);
char* 			int_to_str(int32_t num, char* str, int32_t base);
void * 			memory_set( void *pv_buff , uint8_t u8_char , size_t u32_size );
		
uint8_t 		str_to_hex (const uint8_t buff);
uint8_t 		buff_clr(uint8_t buff[], uint16_t size);
uint8_t 		buff_cmp(const uint8_t buff1[], const uint8_t buff2[], uint8_t size);
uint8_t 		calculate_xor_checksum(const uint8_t *pku8_data,uint32_t u32_len_data);
uint8_t 		buff_cpy(uint8_t dest_buff[], const uint8_t source_buff[], uint16_t dest_size, uint16_t source_size);
uint8_t 		buffer_cpy(uint8_t *pu8_dest_buff, uint32_t u32_dest_buff_size, 
																							const uint8_t *pku8_source_buff, uint32_t u32_copy_data_len );
uint16_t 		crc16(const uint8_t data[], uint32_t size);
uint8_t check_char( uint8_t *buff , uint8_t u8_char , uint32_t u32_size);

#endif
