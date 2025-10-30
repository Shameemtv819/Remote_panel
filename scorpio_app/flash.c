/**
 * @file flash.c
 * @author Vetrivel
 * @brief
 * @date 2025-05-12
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "flash.h"

/***************************************************************** Macros *******************************************************************/
#define FLASH_DB_DEFAULT_CONFIG_VAL                               (0x23U)
/*************************************************************** Variables ******************************************************************/
static flash_elements_def flash;
static const scp_cnf default_config = {
    .IP = {
        .ip_enable = 1,
        .ip_addr = "192.168.1.99\0",
        .gate_way = "192.168.1.88\0",
        .net_mask = "255.255.255.00\0"},

    .GSM = {.u8_gsm_enable = 1,
      .i8_phone_no1[0] = "+918973048450\0", .i8_phone_no1[1] = "+918973048450\0", .i8_phone_no1[2] = "+918973048450\0", 
      .i8_phone_no1[3] = "+918973048450\0", .i8_phone_no1[4] = "+918973048450\0"},
    .https = {.cert = "-----BEGIN CERTIFICATE-----\n"
"MIIEXjCCA0agAwIBAgITB3MSTNQG0mfAmRzdKZqfODF5hTANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTIyMDgyMzIyMjYwNFoXDTMwMDgyMzIyMjYwNFowPDEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEcMBoGA1UEAxMTQW1hem9uIFJT\n"
"QSAyMDQ4IE0wMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALd/pVko\n"
"8vuM475Tf45HV3BbCl/B9Jy89G1CRkFjcPY06WA9lS+7dWbUA7GtWUKoksr69hKM\n"
"wcMsNpxlw7b3jeXFgxB09/nmalcAWtnLzF+LaDKEA5DQmvKzuh1nfIfqEiKCQSmX\n"
"Xh09Xs+dO7cm5qbaL2hhNJCSAejciwcvOFgFNgEMR42wm6KIFHsQW28jhA+1u/M0\n"
"p6fVwReuEgZfLfdx82Px0LJck3lST3EB/JfbdsdOzzzg5YkY1dfuqf8y5fUeZ7Cz\n"
"WXbTjujwX/TovmeWKA36VLCz75azW6tDNuDn66FOpADZZ9omVaF6BqNJiLMVl6P3\n"
"/c0OiUMC6Z5OfKcCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8CAQAwDgYD\n"
"VR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAdBgNV\n"
"HQ4EFgQUVdkYX9IczAHhWLS+q9lVQgHXLgIwHwYDVR0jBBgwFoAUhBjMhTTsvAyU\n"
"lC4IWZzHshBOCggwewYIKwYBBQUHAQEEbzBtMC8GCCsGAQUFBzABhiNodHRwOi8v\n"
"b2NzcC5yb290Y2ExLmFtYXpvbnRydXN0LmNvbTA6BggrBgEFBQcwAoYuaHR0cDov\n"
"L2NydC5yb290Y2ExLmFtYXpvbnRydXN0LmNvbS9yb290Y2ExLmNlcjA/BgNVHR8E\n"
"ODA2MDSgMqAwhi5odHRwOi8vY3JsLnJvb3RjYTEuYW1hem9udHJ1c3QuY29tL3Jv\n"
"b3RjYTEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqGSIb3DQEBCwUAA4IB\n"
"AQAGjeWm2cC+3z2MzSCnte46/7JZvj3iQZDY7EvODNdZF41n71Lrk9kbfNwerK0d\n"
"VNzW36Wefr7j7ZSwBVg50W5ay65jNSN74TTQV1yt4WnSbVvN6KlMs1hiyOZdoHKs\n"
"KDV2UGNxbdoBYCQNa2GYF8FQIWLugNp35aSOpMy6cFlymFQomIrnOQHwK1nvVY4q\n"
"xDSJMU/gNJz17D8ArPN3ngnyZ2TwepJ0uBINz3G5te2rdFUF4i4Y3Bb7FUlHDYm4\n"
"u8aIRGpk2ZpfXmxaoxnbIBZRvGLPSUuPwnwoUOMsJ8jirI5vs2dvchPb7MtI1rle\n"
"i02f2ivH2vxkjDLltSpe2fiC\n"
"-----END CERTIFICATE-----\0",
              .url = "ipgsm.emcus.co.in\0"

    },
    .version = {.scorpio_version = "SCRP_0.99\0", .gemini_version = "firmware_AURA_AURA_v00.00.1111\0",.gsm_number = "2255225522\0"}};

static void flash_open(void);
static void flash_close(void);
static void flash_write_sslcert_conf(void);

/*************************************************************** Functions ******************************************************************/

/**
 * @brief 
 * @author Vetrivel
 */
void flash_init(void)
{
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                         FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
  /* Unlock the Flash to enable the flash control register access*/
  flash_write_sslcert_conf();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                         FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
}

/**
 * @brief 
 * @author Vetrivel
 */
static void flash_open(void)
{
  if (HAL_FLASH_Unlock() != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief 
 * @author Vetrivel
 */
static void flash_close(void)
{
  if (HAL_FLASH_Lock() != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief Set the flash write address object
 * @author Vetrivel
 * @param address 
 */
void set_flash_write_address(uint32_t address)
{
  if((address >= INT_FLASH_SSL_CERT_ADDR_SIZE) && (address < INT_FLASH_END))
  {
    flash.address = address;
  }
  else
  {
    // error. accessing unintended memory region
  }
}

/**
 * @brief 
 * @author Vetrivel
 * @param TypeErase 
 * @param Banks 
 * @param Sector 
 * @param NbSectors 
 */
static void flash_erase(uint32_t TypeErase, uint32_t Banks, uint32_t Sector, uint32_t NbSectors)
{
  FLASH_EraseInitTypeDef st_flash_erase = {TypeErase, Banks, Sector,
                                            NbSectors, FLASH_VOLTAGE_RANGE_3}; 
  uint32_t SectorError = 0;
  flash_open();
  if( HAL_OK != HAL_FLASHEx_Erase(&st_flash_erase, &SectorError))
  {
    debug_msg("\r\nflash erase failed********* ");
    Error_Handler();
  }
  flash_close();
}

/**
 * @brief 
 * @author Vetrivel
 * @param address 
 * @param data 
 */
static void flash_write(uint32_t address, uint8_t *tx_data, uint16_t tx_data_len)
{
  flash_open();
  for (uint32_t i = 0; i < tx_data_len; i++)
  {
    if( HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, tx_data[i]))
    {
        debug_msg("\r\n program failed");
        Error_Handler();
    }
    address++;
  }
  flash.address = address;
  flash_close();
}

/**
 * @brief 
 * @author Vetrivel
 * @param address 
 * @param Sector 
 * @param data 
 */
void flash_write_default_data(uint8_t *address, uint32_t Sector, uint8_t *data)
{
  if ((address != NULL))// && (*address != 0)) // yet to implement
  {
    FLASH_EraseInitTypeDef st_flash_erase = {FLASH_TYPEERASE_SECTORS, 0, Sector,
                                             1, FLASH_VOLTAGE_RANGE_3};
    uint32_t SectorError = 0;
    flash_open();
    if (HAL_OK != HAL_FLASHEx_Erase(&st_flash_erase, &SectorError)) // erase sector 8
    {
      debug_msg("\r\nflash erase failed********* ");
      Error_Handler();
    }

    // marking default address is updated
    if( HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (uint32_t)address - 2u, FLASH_DB_DEFAULT_CONFIG_VAL))
    {
      debug_msg("\r\nprograming failed********* ");
      Error_Handler();      
    }

    for (uint32_t i = 0; i < sizeof(scp_cnf); i++)
    {
      if( HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (uint32_t)address, data[i]))
      {
        debug_msg("\r\nprograming failed********* ");
        Error_Handler();
      }
      address++;
    }
    debug_msg("\r\nwritten to flash\n");
    flash_close();
  }
}

/**
 * @brief 
 * @author Vetrivel
 */
void flash_write_sslcert_conf(void)
{
  uint8_t *default_config_flag = (uint8_t *)(INT_FLASH_SSL_CERT_ADDR - 2);
  uint8_t buff[30] = {0};
  sprintf((char *)buff, "%02X", (*default_config_flag));
  debug_msg(buff);
  if (*default_config_flag != FLASH_DB_DEFAULT_CONFIG_VAL)
  {
    debug_msg("\r\nwriting default conf");
    flash_write_default_data((uint8_t *)INT_FLASH_SSL_CERT_ADDR, FLASH_SECTOR_11, (uint8_t *)&default_config);
  }
  else
  {
    debug_msg("\r\ndefault already wrote");
  }
}

/**
 * @brief
 * @author Vetrivel
 * @param service
 * @param tx_data
 * @param tx_data_len
 */
void flash_task_process(uint8_t service, uint8_t *tx_data, uint16_t tx_data_len)
{
  // switch (service)
  // {
  //   case FLASH_ERASE_QUEUE:
  //     set_flash_write_address(INT_FLASH_SSL_CERT_ADDR); // rework need to set while sending queue
  //     flash_erase(FLASH_TYPEERASE_SECTORS, 0, tx_data[0], 1);
  //   break;

  //   case FLASH_WRITE_QUEUE:
  //     flash_write(flash.address, tx_data, tx_data_len);
  //   break;
  // }
  FLASH_EraseInitTypeDef st_flash_erase = {FLASH_TYPEERASE_SECTORS, 0, FLASH_SECTOR_11,
                                           1, FLASH_VOLTAGE_RANGE_3};
  uint32_t SectorError = 0;

  switch (service)
  {
    case FLASH_ERASE_QUEUE:
    {
      set_flash_write_address(INT_FLASH_SSL_CERT_ADDR);
      flash_open();
      if( HAL_OK != HAL_FLASHEx_Erase(&st_flash_erase, &SectorError)) // erase sector 8
      {
        debug_msg("\r\nflash erase failed********* ");
        Error_Handler();
      }
      if( HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (flash.address - 2u), FLASH_DB_DEFAULT_CONFIG_VAL))
      {
          debug_msg("\r\n program failed");
          Error_Handler();      
      }
      flash_close();
      break;
    }
    case FLASH_WRITE_QUEUE:
    case FLASH_WRITE_END_OF_QUEUE:
    {
      flash_open();
      for (uint32_t i = 0; i < tx_data_len; i++)
      {
        if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, flash.address, tx_data[i]))
        {
          debug_msg("\r\n program failed");
          Error_Handler();
        }
        (uint8_t *)flash.address++;
      }

    flash_close();
    break;
  }
  }
}
