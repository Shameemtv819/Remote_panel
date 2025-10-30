/**
 * @file lte_app.h
 * @author Vetrivel
 * @brief 
 * @date 2025-01-28
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef LTE_APP_H
#define LTE_APP_H
/******************************************************** Header includes **********************************************************/
#include "scorpio_app.h"
/*********************************************************** Macros ****************************************************************/
#define SEND_FROM_ISR                         (0U)
#define SEND_FROM_THREAD                      (1U)

/*********************************************************** typedef ***************************************************************/
typedef struct lte_credentials
{
  uint8_t cloud_api_post[200];
  uint8_t cloud_api_get[100];
  uint8_t cloud_api_firmware[200];  
} lte_credentials_def;

typedef struct at_cmd
{
  uint8_t at_tx_msg[10][200];
  uint8_t at_rx_msg[10][200];
  uint32_t cmd_state;
  uint8_t cmd_count;
} at_cmd_def;

typedef enum lte_cmd_state
{
  LTE_INIT_FLAG                           = 0x01U,
  LTE_SMS_FLAG                            = 0x02U,
  LTE_CLOUD_FLAG                          = 0x04U,
  LTE_CLOUD_POST_FLAG                     = 0x08U,
  LTE_CLOUD_FW_DOWNLOAD_FLAG              = 0x10U, // b/w stm and lte
  LTE_CLOUD_FW_DOWNLOAD_READ_FLAG         = 0x20U, // b/w stm and lte
  LTE_CLOUD_FW_UPLOAD_FLAG                = 0x40U, // b/w stm and stm
  LTE_SWITCH_SIM_SLOT                     = 0x80U, // b/w stm and lte
} lte_cmd_state_def;

typedef struct firmware_upgrade
{
  uint32_t value_to_read; 
  uint32_t start_value;
  uint16_t packet_number;
  uint8_t retry_count;
  uint8_t mask_bytes;
} firmware_upgrade_def;

typedef enum en_lte_process
{
  LTE_SMS_QUEUE           = 0x01,
  LTE_CLOUD_GET_QUEUE           ,  
  LTE_CLOUD_POST_QUEUE          , 
  LTE_INIT_QUEUE                ,
  LTE_PDP_ACT_QUEUE             , 
  LTE_CLOUD_FW_DOWNLOAD_QUEUE   , 
  LTE_SWITCH_SIM2,
  LTE_SWITCH_SIM1,
} en_lte_process_def;

typedef enum eth_cmd_state
{
  ETH_CLOUD_FW_UPLOAD_FLAG = 0x01,
  ETH_RECEIVED_IP                ,   
   
} eth_cmd_state_def;

typedef enum en_eth_process
{ 
  ETH_CLOUD_FW_DOWNLOAD_QUEUE     = 0X01, 
	ETH_CLOUD_POST_QUEUE                  ,
} en_eth_process_def;

typedef enum
{ 
  FLASH_WRITE_QUEUE     = 0X01,
  FLASH_ERASE_QUEUE           ,
  FLASH_WRITE_END_OF_QUEUE    ,        
} en_flash_process_def;

/******************************************************** Extern variables *********************************************************/

/******************************************************* function prototypes ******************************************************/
void lte_init(void);
void lte_pdp_activate(void);
void lte_config_ssl_cmds(void);
void set_eth_event_bits_from_isr(void);
void lte_config_ssl_cert(void);
void lte_task_process(uint8_t service, uint8_t *tx_data, uint16_t tx_data_len);
void lte_send_queue_isr(uint8_t *source_data, uint8_t event_type, uint8_t source_size, uint8_t thread_isr_mode);

uint8_t lte_module_reset (void);

#endif
