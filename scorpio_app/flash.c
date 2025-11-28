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
"MIIFzTCCBLWgAwIBAgIQA3po5rThP1jFvWL/fmpItDANBgkqhkiG9w0BAQsFADA8\n"
"MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRwwGgYDVQQDExNBbWF6b24g\n"
"UlNBIDIwNDggTTAzMB4XDTI1MDYwOTAwMDAwMFoXDTI2MDcwODIzNTk1OVowHDEa\n"
"MBgGA1UEAxMRaXBnc20uZW1jdXMuY28uaW4wggEiMA0GCSqGSIb3DQEBAQUAA4IB\n"
"DwAwggEKAoIBAQCHU4Unq5mNFVg0qMt8PUNat2wq0VawEKWrKJIy9a6dQT/qZ9n6\n"
"/+CBEIi0sotWVDQ5xFHWZRZXtBNqVEDwvIxYSSFvIiu1fvojOSTy6sE79joy3Ulr\n"
"UaqVWTKSdrHFvBr+NPUns4iBCQeb8bG5l3dmQg0Wu4zrumKc7MoSvozuxgz3aHlR\n"
"4AGB5QDZ5J++4cpnmlLNBB3Z/btuUs1fU+r84XCZ2ilpE0Y6WpkUSCUnDikUkkaK\n"
"eOUBNTQUAG7CfMzaYQhHXri6tWWa+ZcrLZvjs5OcFcoCR4vA+wOT2lz5/1oaxkBP\n"
"I9LgQVPf9Zsz4hXyS+lhk4LiV/ZclnPwMHohAgMBAAGjggLpMIIC5TAfBgNVHSME\n"
"GDAWgBRV2Rhf0hzMAeFYtL6r2VVCAdcuAjAdBgNVHQ4EFgQUxjF6F5cgBD9KKkjD\n"
"QAyOxTj0cCwwHAYDV0RBBUwE4IRaXBnc20uZW1jdXMuY28uaW4wEwYDVR0gBAww\n"
"CjAIBgZngQwBAgEwDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMB\n"
"BggrBgEFBQcDAjA7BgNVHR8ENDAyMDCgLqAshipodHRwOi8vY3JsLnIybTAzLmFt\n"
"YXpvbnRydXN0LmNvbS9yMm0wMy5jcmwwdQYIKwYBBQUHAQEEaTBnMC0GCCsGAQUF\n"
"BzABhiFodHRwOi8vb2NzcC5yMm0wMy5hbWF6b250cnVzdC5jb20wNgYIKwYBBQUH\n"
"MAKGKmh0dHA6Ly9jcnQucjJtMDMuYW1hem9udHJ1c3QuY29tL3IybTAzLmNlcjAM\n"
"BgNVHRMBAf8EAjAAMIIBfQYKKwYBBAHWeQIEAgSCAW0EggFpAWcAdgDXbX0Q0af1\n"
"d8LH6V/XAL/5gskzWmXh0LMBcxfAyMVpdwAAAZdUSD6XAAAEAwBHMEUCIQCOPTBD\n"
"ZD/ghwDWApio52TgxWLCnrX8MmaM5ouH9MDRtwIgJ7CGXGnKzOTJeqX7LguvbOhJ\n"
"8Q96AK+cmHzmZdHCr5IAdgDCMX5XRRmjRe5/ON6ykEHrx8IhWiK/f9W1rXaa2Q5S\n"
"zQAAAZdUSD7DAAAEAwBHMEUCIQCgH386//PgFj+zOEScYSSOnjGLqIuPHNRvJcRP\n"
"ffoengIgUN3nDJmkFMn/eibOzr8chJorbJcNr7UfBkvVbd6BgkAAdQCUTkOH+uzB\n"
"74HzGSQmqBhlAcfTXzgCAT9yZ31VNy4Z2AAAAZdUSD7UAAAEAwBGMEQCIChuIDcZ\n"
"QSl5/zfNQm3bnVBH2wP0nTg9F0jnxwdLd78NAiAUhhE4yUtZUXli/2w0fEImIJhc\n"
"l4GMNaPl2jts82uJMTANBgkqhkiG9w0BAQsFAAOCAQEApvpVEhBVNt11Q/SbyUVy\n"
"ZO8y7WTXCRTBCYRx/Ei5XY0epOUokXOwvz6ta5QPWQSR1IC5SGoaPd/F2zjv9Z8Z\n"
"8OnoE7FTkUqnEjlcFzMDxa3JlAkeuGrFg9rQJh9w7lrqF9Man5AvHoBGkkUH/fkd\n"
"OZUnpzHOBHRiiOSqbv54wRoYFZ3kuS5X/gHgWP8OelJOWsZNTIR1ytQzCgue2j1R\n"
"uloz9qm09jzuFQ4ZdLHPTFjO8ADg57Va3mNEjmWke55j5aamdQ6nULd5VLZtMXqu\n"
"K27MBbUrN8mKSYWhGt9A7elrD5uil9WUIF0NX34+1AcasZr+/h8EGw8v3C/LLVHB\n"
"FQ==\n"
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
