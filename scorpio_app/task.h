/**
 * @file task.h
 * @author Vetrivel
 * @brief 
 * @date 2025-02-03
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef TASK_H
#define TASK_H
/******************************************************** Header includes **********************************************************/
#include "scorpio_app.h"
/***********************************************************  Macros ***************************************************************/
#define EVENT_IP_DYNAMIC   (1 << 0)

/*********************************************************** typedef ***************************************************************/
typedef struct lte
{
  uint8_t service; // sms/ send to cloud
  uint16_t tx_data_len;
  uint8_t tx_data[60];
} queue_data_def; // max length can be 16 words in threadx

typedef enum relay_state
{
  FIRE_RELAY,
  FAULT_RELAY,
  RESET_RELAY,
} relay_state_def;

/******************************************************** Extern variables *********************************************************/
extern EventGroupHandle_t event_lte_rx;
extern EventGroupHandle_t event_mcu_comm_rx;
extern EventGroupHandle_t event_eth_rx;
extern EventGroupHandle_t event_ip_stat;

extern QueueHandle_t queue_lte;
extern QueueHandle_t queue_eth;
extern QueueHandle_t queue_flash;
extern QueueHandle_t queue_ip_conf;
extern QueueHandle_t queue_relay;

extern osThreadId_t mqtt_task_handle;

/******************************************************* Function prototypes *******************************************************/
void app_init (void);
#endif
/************************************************ Define to prevent recursive inclusion ********************************************/

/******************************************************** Header includes **********************************************************/

/*********************************************************** Macros ****************************************************************/

/*********************************************************** typedef ***************************************************************/




/******************************************************** Extern variables *********************************************************/
/******************************************************* function prototypes ******************************************************/
