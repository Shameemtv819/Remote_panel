/**
 * @file common.c
 * @author Vetrivel
 * @brief 
 * @date 2025-01-30
 * @copyright Copyright (c) 2025
 */

/************************************************************* Header includes ***************************************************************/
#include "common.h"

/***************************************************************** Macros ********************************************************************/
#define CRC_POLYNOMIAL_CCITT                    (0x1021U) 
#define CRC_CCITT_SEED_VALUE                    (0xFFFFU) 

/*************************************************************** Functions ******************************************************************/
/**
 * @brief A utility function to reverse a string
 * @author Vetrivel
 * @param str 
 * @param length 
 */
void string_reverse(char str[], int32_t length)
{
	int32_t start = 0;
	int32_t end = length - 1;
	while (start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		end--;
		start++;
	}
}

/**
 * @brief This function converts integer to string
 * @author Vetrivel
 * @param num integer to be converted
 * @param str buffer where the converted string need to be stored
 * @param base dec/hec/ @ref base_definitions
 * @return char* 
 */
char* int_to_str(int32_t num, char* str, int32_t base)
{
	int32_t i32_local_num = num;
	int32_t i = 0;
	uint8_t isNegative = false;

	do{ /*misra-c compliance to return in one are/at end of the function*/
		
		/* Handle 0 explicitly, otherwise empty string is
		* printed for 0 */
		if (i32_local_num == 0) {
			str[i] = '0'; i++;
			str[i] = '\0';
			break;
		}

		// In standard itoa(), negative numbers are handled
		// only with base 10. Otherwise numbers are
		// considered unsigned.
		if ( (i32_local_num < 0) && (base == 10) ) {
			isNegative = true;
			i32_local_num = -i32_local_num;
		}

		// Process individual digits
		while (i32_local_num != 0) {
			int32_t rem = i32_local_num % base;
			str[i] = (rem > 9) ? ((rem - 10) + 'a') : (rem + '0');
			i++;
			i32_local_num = i32_local_num / base;
		}

		// If number is negative, append '-'
		if (isNegative)
		{
			str[i] = '-'; 
			i++;
		}
		str[i] = '\0'; // Append string terminator

		// Reverse the string
		string_reverse(str, i);
	}while(0);
	
	return str;
}



/****************************************************************************************************************
 * @brief 		Copies data from the source buffer to the destination buffer.
 * \n					This function copies a specified number of bytes from a source buffer to a destination buffer.
 * \n					It checks if the destination buffer is large enough to hold the data being copied. If not, 
 * \n					it returns false , indicating failure. Otherwise, it performs the copy and returns 1,
 * \n					indicating success.
 * @author 		Selvakumar G
 *
 * @param			pu8_dest_buff 		 : Pointer to the destination buffer where data will be copied.
 * @param			u32_dest_buff_size : Size of the destination buffer in bytes.
 * @param			pku8_source_buff 	 : Pointer to the source buffer from which data will be copied.
 * @param			u32_copy_data_len  : Number of bytes to copy from the source buffer to the destination buffer.
 *
 * @return 1 if the copy is successful, 0 if the destination buffer is not large enough.
 ****************************************************************************************************************/
 
uint8_t buffer_cpy(uint8_t *pu8_dest_buff, uint32_t u32_dest_buff_size, 
											const uint8_t *pku8_source_buff, uint32_t u32_copy_data_len )
{
	
	uint8_t u8_status = false;
	uint32_t u32_index;
	
	/*Condition to avoid buffer overflow error*/
	if ( (u32_dest_buff_size >= u32_copy_data_len) && (NULL!= pku8_source_buff ) && ( NULL != pu8_dest_buff ) )
	{
		
		for ( u32_index = 0x00u ; u32_index < u32_copy_data_len ; u32_index++ )
		{
				pu8_dest_buff[u32_index] = pku8_source_buff[u32_index];
		}
		u8_status = true;
	}
	else
	{
		/*Misra-C Compliance*/
	}
	return u8_status;
}

/****************************************************************************************************************
 * @brief 		Clears a buffer by filling it with a specified character.
 *\n 					This function clears a buffer by filling each byte with a specified character.
 *\n 					It accepts a pointer to the buffer, the character to fill (as an integer), and
 *\n 					the size of the buffer.
 * @author 		Selvakumar G
 *
 * @param 		pv_buff Pointer  : to the buffer to be cleared.
 * @param 		i32_char         : Integer value of the character to fill the buffer with.
 * @param 		u32_size         : Size of the buffer in bytes.
 * 
 * @return 		Returns the base address of the buffer (`pv_buff`).
 ****************************************************************************************************************/

void * memory_set( void *pv_buff , uint8_t u8_char , size_t u32_size )
{
	
	uint8_t *un_pointer_list;
	
	un_pointer_list = (uint8_t *)pv_buff;
	
	while( u32_size-- )
	{
		*un_pointer_list = u8_char ;
		 un_pointer_list++ ;
	}
	
	return pv_buff ;
}

/**
 * @brief 
 * @author Vetrivel
 * @param buff 
 * @return uint8_t 
 */
uint8_t str_to_hex (const uint8_t buff)
{
	uint8_t hex_val = 0;
	if (buff >= 'A' && buff <= 'F')
	{
		hex_val = buff - 55; // ascii value of (A) - 10. Since value of A is 10
	}
	else if (buff >= 'a' && buff <= 'f')
	{
		hex_val = buff - 87; // ascii value of (a) - 10. since value of a is 10
	}
	else 
	{
		hex_val = buff - ASCII_NUMBERS_START_VAL;
	}
	return hex_val;
}

/**
 * @brief 
 * @author Vetrivel
 * @param buff1 
 * @param buff2 
 * @param size 
 * @return uint8_t 
 */
uint8_t buff_cmp(const uint8_t buff1[], const uint8_t buff2[], uint8_t size)
{
    uint8_t status;
    uint8_t index;
    for (index = 0; index < size; index++)
    { 
        if (buff1[index] != buff2[index])
        {
            status = ERROR; // status = 1
            break;
        }
        else
        {
            status = SUCCESS; // status = 0
        }
    }
    return status;
}

/**
 * @brief 
 * @author Vetrivel
 * @param buff 
 * @param size 
 * @return uint8_t 
 */
uint8_t buff_clr(uint8_t buff[], uint16_t size)
{
    uint8_t status;
    uint16_t index;
    for (index = 0; index < size; index++)
    { 
        buff[index] = 0;
    }
    status = SET;
    return status;
}

/**
 * @brief 
 * @author Vetrivel
 * @param pv_buff 
 * @param u8_char 
 * @param u32_size 
 * @return uint8_t returns success only if all characters matches the char provided. if one char differs - it returns failure.
 */
uint8_t check_char( uint8_t *buff , uint8_t u8_char , uint32_t u32_size)
{
	uint8_t status = SUCCESS;
	while( u32_size-- )
	{
		if(*buff != u8_char)
		{ // NOT 0XFF;
			u32_size = 0U;
			status = ERROR;
		}
		buff++ ;
	}
	
	return status ;
}

/**
 * @brief 
 * @author Vetrivel
 * @param dest_buff 
 * @param source_buff 
 * @param dest_size 
 * @param source_size 
 * @return uint8_t 
 */
uint8_t buff_cpy(uint8_t dest_buff[], const uint8_t source_buff[], uint16_t dest_size, uint16_t source_size)
{
	uint8_t status;
	// destination buffer size should be greater than/ equal to source size.
	if ((dest_size < source_size) || (source_buff == NULL))
	{
		status = ERROR;
		// error condition
	}
	else
	{ // if dest_size is greater than or equal to source size
		// buff_clr(dest_buff, dest_size);
		memory_set( dest_buff , 0 , dest_size);
		uint16_t index;
		for (index = 0; index < source_size; index++)
		{ 
			dest_buff[index] = source_buff[index];
		}
		status = SUCCESS;
	}
    return status;
}

/**
 * @brief 
 * @author Vetrivel
 * @param num 
 * @param str 
 * @param base 
 * @return char* 
 */
char* itoa(int num, char* str, int base) {
  int i = 0;
  bool isNegative = false;

  if (num == 0) {
    str[i++] = '0';
    str[i] = '\0';
    return str;
  }
  
  if (num < 0 && base == 10) {
    isNegative = true;
    num = -num;
  }

  while (num != 0) {
    int rem = num % base;
    str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
    num = num / base;
  }

  if (isNegative) {
    str[i++] = '-';
  }

  str[i] = '\0';

  int start = 0;
  int end = i - 1;
  while (start < end) {
    char temp = str[start];
    str[start] = str[end];
    str[end] = temp;
    start++;
    end--;
  }
  return str;
}

/**
 * @brief       Calculate XOR-based CRC for a given data array.
 *              This function calculates a XOR-based CRC for the specified data array and length.
 * @author      Selvakumar G
 *
 * @param   pu8_data       : Pointer to the data array.
 * @param   u32_len_data   : Length of the data array.
 *
 * @return  Return the calculated XOR-based CRC value.
 */
uint8_t calculate_xor_checksum(const uint8_t *pku8_data,uint32_t u32_len_data)
{
	uint8_t u8_calculated_crc=0;
	for(uint32_t u32_itr=0; u32_itr<u32_len_data ; u32_itr++)
	{
		u8_calculated_crc^=pku8_data[u32_itr];
	}
	return u8_calculated_crc;
}

/**
 * @brief This function is used to calculate crc for the data passed as argument.
 * @param data : pointer to data to which crc has to be calculated
 * @param size : length of data
 * @retval None
 */
uint16_t crc16(const uint8_t data[], uint32_t size)
{
    // Initialize CRC value
    uint16_t crc = CRC_CCITT_SEED_VALUE;
    // // Calculate CRC for each byte in the data buffer
    // for (uint32_t i = 0; i < size; i++) {
		// 	uint8_t byte = data[i];
		// 	for (uint8_t j = 0; j < 8U; j++) {
		// 		if ((crc ^ byte) & 0x0001U) {
		// 				crc = (crc >> 1) ^ CRC_POLYNOMIAL_CCITT;
		// 		} else {
		// 				crc = crc >> 1;
		// 		}
		// 		byte = byte >> 1;
		// 	}
    // }
    // // Invert CRC and return
    // return ~crc & 0xFFFFU;

    for (uint32_t i = 0; i < size; i++) {
			crc ^= (data[i] << 8);  // XOR byte into upper CRC byte (MSB-first)

			for (uint8_t j = 0; j < 8; j++) {
					if (crc & 0x8000) {
							crc = (crc << 1) ^ CRC_POLYNOMIAL_CCITT;  // XOR with polynomial
					} else {
							crc = crc << 1;
					}
			}
	}

	return crc;
}
