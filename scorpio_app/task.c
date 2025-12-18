/**
 * @file task.c
 * @author Vetrivel
 * @brief
 * @date 2025-05-14
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "task.h"
/***************************************************************** Macros ********************************************************************/
#define QUEUE_LENGTH 20
#define MAX_DHCP_TRIES 3

#define FIRE_START (1000)
#define FAULT_START (2000)
#define ALL_EVENTS_START (2000)
#define FIRE_END (1999)
#define FAULT_END (2999)
#define ALL_EVENTS_END (2999)

/*************************************************************** Variables ******************************************************************/

/* events */
static StaticEventGroup_t xEventGroupLTE;
static StaticEventGroup_t xEventGroupMCUcomm;
static StaticEventGroup_t xEventGroupETH;
static StaticEventGroup_t xEventGroupIP_stat;
EventGroupHandle_t event_lte_rx;
EventGroupHandle_t event_mcu_comm_rx;
EventGroupHandle_t event_eth_rx;
EventGroupHandle_t event_ip_stat;

/* queues*/
static uint8_t ltequeueStorageArea[sizeof(queue_data_def) * 30];
static uint8_t ethqueueStorageArea[sizeof(queue_data_def) * QUEUE_LENGTH];
static uint8_t flashqueueStorageArea[sizeof(queue_data_def) * QUEUE_LENGTH];
static uint8_t ip_confqueueStorageArea[sizeof(uint8_t) * QUEUE_LENGTH];
static uint16_t relayqueueArea[sizeof(uint16_t) * QUEUE_LENGTH];

static StaticQueue_t ltequeueStruct;
static StaticQueue_t ethqueueStruct;
static StaticQueue_t flashqueueStruct;
static StaticQueue_t ip_confqueueStruct;
static StaticQueue_t relay_queue_struct;

QueueHandle_t queue_lte;
QueueHandle_t queue_eth;
QueueHandle_t queue_flash;
QueueHandle_t queue_ip_conf;
QueueHandle_t queue_relay;

/*tasks*/
osThreadId_t ip_task_handle;
uint32_t ip_task_buffer[1024] = {0}; // 4kb
StaticTask_t ip_task_control_block;
const osThreadAttr_t ip_task_attributes = {
    .name = "ip task",
    .cb_mem = &ip_task_control_block,
    .cb_size = sizeof(ip_task_control_block),
    .stack_mem = &ip_task_buffer[0],
    .stack_size = sizeof(ip_task_buffer),
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t lte_task_handle;
uint32_t lte_task_buffer[256] = {0}; // 1kb
StaticTask_t lte_task_control_block;
const osThreadAttr_t lte_task_attributes = {
    .name = "lte task",
    .cb_mem = &lte_task_control_block,
    .cb_size = sizeof(lte_task_control_block),
    .stack_mem = &lte_task_buffer[0],
    .stack_size = sizeof(lte_task_buffer),
    .priority = (osPriority_t)osPriorityNormal,
};


osThreadId_t flash_task_handle;
uint32_t flash_task_buffer[256] = {0}; // 1kb
StaticTask_t flash_task_control_block;
const osThreadAttr_t flash_task_attributes = {
    .name = "flash task",
    .cb_mem = &flash_task_control_block,
    .cb_size = sizeof(flash_task_control_block),
    .stack_mem = &flash_task_buffer[0],
    .stack_size = sizeof(flash_task_buffer),
    .priority = (osPriority_t)osPriorityAboveNormal,
};

osThreadId_t dhcp_task_handle;
uint32_t dhcp_task_buffer[512] = {0}; // 2kb
StaticTask_t dhcp_task_control_block;
const osThreadAttr_t dhcp_task_attributes = {
    .name = "DHCP task",
    .cb_mem = &dhcp_task_control_block,
    .cb_size = sizeof(dhcp_task_control_block),
    .stack_mem = &dhcp_task_buffer[0],
    .stack_size = sizeof(dhcp_task_buffer),
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t relay_task_handle;
uint32_t relay_task_buffer[128] = {0}; // 512kb
StaticTask_t relay_task_control_block;
const osThreadAttr_t relay_task_attributes = {
    .name = "relay task",
    .cb_mem = &relay_task_control_block,
    .cb_size = sizeof(relay_task_control_block),
    .stack_mem = &relay_task_buffer[0],
    .stack_size = sizeof(relay_task_buffer),
    .priority = (osPriority_t)osPriorityNormal,
};

/* Definitions for defaultTask */
osThreadId_t http_server_task_handle;
uint32_t http_server_task_buffer[512] = {0}; // 2kb
StaticTask_t http_server_task_control_block;
const osThreadAttr_t http_server_task_attributes = {
    .name = "http server task",
    .cb_mem = &http_server_task_control_block,
    .cb_size = sizeof(http_server_task_control_block),
    .stack_mem = &http_server_task_buffer[0],
    .stack_size = sizeof(http_server_task_buffer),
    .priority = (osPriority_t)osPriorityNormal,
};

extern struct netif gnetif;

/****************************************************** function prototype **************************************************************/
void http_server_netconn_init(void);
/************************************************************** Tasks ******************************************************************/
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
void lte_task(void *argument)
{
  (void)argument;
  queue_data_def lte_data_recv = {0};
  /* Infinite loop */
  for (;;)
  {
#if (THREADX)
    if (TX_SUCCESS == tx_queue_receive(&queue_lte, lte_data_recv, TX_WAIT_FOREVER))
#elif (FREE_RTOS)
    if (pdTRUE == xQueueReceive(queue_lte, (queue_data_def *)&lte_data_recv, portMAX_DELAY))
#endif
    {
      lte_task_process(lte_data_recv.service, lte_data_recv.tx_data, lte_data_recv.tx_data_len);
    }
  }
}

/**
 * @brief
 * @author Vetrivel
 * @param argument
 */
void relay_thread(void *argument)
{
  uint16_t relay_queue_recv = {0};
  for (;;)
  {
    if (pdTRUE == xQueueReceive(queue_relay, (uint8_t *)&relay_queue_recv, portMAX_DELAY))
    {
      relay_queue_recv = relay_queue_recv / 1000;
      switch (relay_queue_recv)
      {
      case 1:
        change_relay_state(RELAY_1, GPIO_PIN_SET);
        break;
      case 2:
        change_relay_state(RELAY_2, GPIO_PIN_SET);
        break;
      case 3:
        change_relay_state(RELAY_2, GPIO_PIN_RESET);
        change_relay_state(RELAY_1, GPIO_PIN_RESET);
        break;
      }
    }
  }
}

/**********************************************************************************************************************************************
  * @brief  http server thread.
            This function will handle incoming connecion request from client and all handling function.
  * @param arg: pointer on argument(not used here)
  * @retval None
 ************************************************************************************************************************************************/
void http_server_netconn_thread(void *arg)
{
  struct netconn *conn;
  struct netconn *newconn;

  err_t err;
  err_t accept_err;

  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);

  if (conn != NULL)
  {
    /* Bind to port 80 (HTTP) with default IP address */
    err = netconn_bind(conn, NULL, 80);

    if (err == ERR_OK)
    {
      /* Put the connection into LISTEN state */
      netconn_listen(conn);

      while (1)
      {
        /* accept any icoming connection */
        accept_err = netconn_accept(conn, &newconn);
        if (accept_err == ERR_OK)
        {
          /* serve connection */
          http_server_serve(newconn);

          /* delete connection */
          netconn_delete(newconn);
        }
      }
    }
  }
}

/**
 * @brief  DHCP Process
 * @param  argument: network interface
 * @retval None
 */
void DHCP_thread(void *argument)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  struct dhcp *dhcp;
  uint8_t dhcp_queue = {0};
  BaseType_t xHigherPriorityTaskWoken = pdFAIL;

  for (;;)
  {
    if (pdTRUE == xQueueReceive(queue_ip_conf, (uint8_t *)&dhcp_queue, portMAX_DELAY))
    {
      switch (dhcp_queue)
      {
      case CHECK_NETIF_IS_UP:
        if (netif_is_up(&gnetif))
        {
          dhcp_queue = DHCP_START;
        }
        else
        {
          dhcp_queue = CHECK_NETIF_IS_UP;
          // add retry count limit
        }
        xQueueSendFromISR(queue_ip_conf, (uint8_t *)&dhcp_queue, &xHigherPriorityTaskWoken);
        break;

      case DHCP_START:
      {
        ip_addr_set_zero_ip4(&gnetif.ip_addr);
        ip_addr_set_zero_ip4(&gnetif.netmask);
        ip_addr_set_zero_ip4(&gnetif.gw);
        dhcp_start(&gnetif);
        dhcp_queue = DHCP_WAIT_ADDRESS;
        xQueueSendFromISR(queue_ip_conf, (queue_data_def *)&dhcp_queue, &xHigherPriorityTaskWoken);
      }
      break;

      case DHCP_WAIT_ADDRESS:
      {
        if (dhcp_supplied_address(&gnetif))
        {
          dhcp_queue = DHCP_ADDRESS_ASSIGNED;
          xQueueSendFromISR(queue_ip_conf, (queue_data_def *)&dhcp_queue, &xHigherPriorityTaskWoken);
          fetch_network_time();
					
					queue_data_def eth_data = {0};
					xEventGroupSetBits(event_ip_stat, EVENT_IP_DYNAMIC);
memcpy(eth_data.tx_data, &fire, sizeof(EventLog_t));
    eth_data.tx_data_len = sizeof(EventLog_t);
  BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
  
					 eth_data.service = ETH_CLOUD_POST_QUEUE;
	
    xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					 xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
					
          // debug_msg("\r\nIP address assigned starting mqtt task:");
					// if(mqtt_task_handle == 0U)
					// {
          //     mqtt_task_handle = osThreadNew(mqtt_task, NULL, &mqtt_task_attributes);
					// }
        }
        else
        {
          dhcp = (struct dhcp *)netif_get_client_data(&gnetif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

          /* DHCP timeout */
          uint8_t buff[4] = {0};
          sprintf((char *)buff, "%d", dhcp->tries);
          debug_msg("retry count : ");
          debug_msg(buff);
          if (dhcp->tries > MAX_DHCP_TRIES)
          {
						xEventGroupClearBits(event_ip_stat, EVENT_IP_DYNAMIC);
            dhcp_queue = DHCP_TIMEOUT;
            xQueueSendFromISR(queue_ip_conf, (queue_data_def *)&dhcp_queue, &xHigherPriorityTaskWoken);
            /* Stop DHCP */
            dhcp_stop(&gnetif);

            /* Static address used */
            ipaddr_aton((char *)pf_config->IP.ip_addr, &ipaddr);
            ipaddr_aton((char *)pf_config->IP.gate_way, &gw);
            ipaddr_aton((char *)pf_config->IP.net_mask, &netmask);
            debug_msg("\r\n IP : ");
            debug_msg((uint8_t *)ipaddr_ntoa(&ipaddr));
            debug_msg("\r\n NET_MSK : ");
            debug_msg((uint8_t *)ipaddr_ntoa(&netmask));
            debug_msg("\r\n GW : ");
            debug_msg((uint8_t *)ipaddr_ntoa(&gw));

            // over-writing ip address from waiting to dhcp
            netif_set_addr(&gnetif, &ipaddr, &netmask, &gw);
            /*  Registers the default network interface. */
            netif_set_default(&gnetif);
          }
          else
          {
            // send DHCP_WAIT_ADDRESS event
            dhcp_queue = DHCP_WAIT_ADDRESS;
            xQueueSendFromISR(queue_ip_conf, (queue_data_def *)&dhcp_queue, &xHigherPriorityTaskWoken);
          }
        }
      }
      break;
      case DHCP_LINK_DOWN: // disconnected
      {
        /* Stop DHCP */
        dhcp_stop(&gnetif);
      }
      break;

      default:
        break;
      }
      /* wait 250 ms */
      osDelay(250);
    }
  }
}

/**
 * @brief
 * @author Shameem
 * @param argument
 */
void ip_task(void *argument)
{
  BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
  debug_msg((uint8_t *)"IP_task process");
  (void)argument;
  queue_data_def ip_data_recv = {0};
  /* init code for LWIP */
  if (pf_config->GSM.u8_gsm_enable)
  {
//    if (SUCCESS == lte_module_reset())
//    {
//      lte_init();
//    }
//    else
//    {
//      debug_msg((uint8_t *)"lte not initialized properly");
//    }
  }
  else
  {
    debug_msg("gsm disabled");
  }
  MX_LWIP_Init();

  /* Configure and start the SNTP client */

  http_server_netconn_init();

  User_notification(&gnetif);

#if (TEST_LTE_FW_DWNLD_DEBUG)
  lte_send_queue_isr(NULL, LTE_CLOUD_FW_DOWNLOAD_QUEUE, NULL, SEND_FROM_THREAD);
#endif
#if (TEST_ETH_FW_DWNLD_DEBUG)
  osDelay(15000); // given to acquire DHCP ip
  ip_data_recv.service = ETH_CLOUD_FW_DOWNLOAD_QUEUE;
  xQueueSendFromISR(queue_eth, (queue_data_def *)&ip_data_recv, &pxHigherPriorityTaskWoken);
#endif
#if (TEST_LTE_SMS)
  lte_send_queue_isr("Hello from lte module", LTE_SMS_QUEUE, strlen("Hello from lte module"), SEND_FROM_THREAD);
#endif
#if (TEST_LTE_SIM_SWITCH)
  lte_send_queue_isr("Hello from SIM1", LTE_SMS_QUEUE, strlen((char *)"Hello from SIM1"), SEND_FROM_THREAD);
  lte_send_queue_isr(NULL, LTE_SWITCH_SIM2, NULL, SEND_FROM_THREAD);
#endif

  /* Infinite loop */
  for (;;)
  {
		if(pdTRUE == xEventGroupWaitBits(event_ip_stat,EVENT_IP_DYNAMIC,pdFALSE,pdTRUE,100))
		{
#if (THREADX)
    if (TX_SUCCESS == tx_queue_receive(&queue_lte, ip_data_recv, TX_WAIT_FOREVER))
#elif (FREE_RTOS)
    if (pdTRUE == xQueueReceive(queue_eth, (queue_data_def *)&ip_data_recv, 100))
#endif
    {
      ip_task_process(ip_data_recv.service, ip_data_recv.tx_data, ip_data_recv.tx_data_len);
    }else
		{
			mqtt_process();
		}
  }
}
}

/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
void flash_task(void *argument)
{
  (void)argument;
  queue_data_def flash_data_recv = {0};
  /* Infinite loop */
  for (;;)
  {
#if (THREADX)
    if (TX_SUCCESS == tx_queue_receive(&queue_flash, flash_data_recv, TX_WAIT_FOREVER))
#elif (FREE_RTOS)
    if (pdTRUE == xQueueReceive(queue_flash, (queue_data_def *)&flash_data_recv, portMAX_DELAY))
#endif
    {
      flash_task_process(flash_data_recv.service, flash_data_recv.tx_data, flash_data_recv.tx_data_len);
    }
  }
}

/**
 * @brief
 * @author Vetrivel
 */
static void xTasks_init(void)
{
  if (pf_config->GSM.u8_gsm_enable)
  {
//    lte_task_handle = osThreadNew(lte_task, NULL, &lte_task_attributes);
  }
  else
  {
    debug_msg("\r\ngsm disabled");
  }
  ip_task_handle = osThreadNew(ip_task, NULL, &ip_task_attributes);
  flash_task_handle = osThreadNew(flash_task, NULL, &flash_task_attributes);
//  relay_task_handle = osThreadNew(relay_thread, NULL, &relay_task_attributes);
  if (pf_config->IP.ip_enable)
  {
    dhcp_task_handle = osThreadNew(DHCP_thread, NULL, &dhcp_task_attributes);
  }
  else
  {
    debug_msg("ip disabled , disabling dhcp thread");
  }
}

/**
 * @brief
 * @author Vetrivel
 */
static void event_flags_init(void)
{
  event_lte_rx = xEventGroupCreateStatic(&xEventGroupLTE);
  event_mcu_comm_rx = xEventGroupCreateStatic(&xEventGroupMCUcomm);
  event_eth_rx = xEventGroupCreateStatic(&xEventGroupETH);
	event_ip_stat = xEventGroupCreateStatic(&xEventGroupIP_stat);
}

/**
 * @brief
 * @author Vetrivel
 */
static void queue_init(void)
{
  queue_lte = xQueueCreateStatic(QUEUE_LENGTH, sizeof(queue_data_def),
                                 (uint8_t *)ltequeueStorageArea, &ltequeueStruct);
  queue_eth = xQueueCreateStatic(QUEUE_LENGTH, sizeof(queue_data_def),
                                 (uint8_t *)ethqueueStorageArea, &ethqueueStruct);
  queue_flash = xQueueCreateStatic(QUEUE_LENGTH, sizeof(queue_data_def),
                                   (uint8_t *)flashqueueStorageArea, &flashqueueStruct);
  queue_ip_conf = xQueueCreateStatic(QUEUE_LENGTH, sizeof(uint8_t),
                                     (uint8_t *)ip_confqueueStorageArea, &ip_confqueueStruct);
  queue_relay = xQueueCreateStatic(QUEUE_LENGTH, sizeof(uint16_t),
                                   (uint8_t *)relayqueueArea, &relay_queue_struct);
}

/***************************************************************************************************************************************
 * @brief  Initialize the HTTP server (start its thread)
 * @param  none
 * @retval None
 ***************************************************************************************************************************************/
void http_server_netconn_init(void)
{
  http_server_task_handle = osThreadNew(http_server_netconn_thread, NULL,
                                              &http_server_task_attributes);
}

/**
 * @brief
 * @author Vetrivel
 */
void app_init(void)
{
  queue_init();
  event_flags_init();
  xTasks_init();
}
