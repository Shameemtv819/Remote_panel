/**
 * @file lte_app.c
 * @author Vetrivel
 * @brief This file contains functions related to lte application.
 * @date 2025-01-27
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "lte_app.h"
/***************************************************************** Macros ********************************************************************/
#if (GSM_ENABLE)
/* init tx messages */
#define CMD_TX_AT                         "AT\r\n"                                       // test command
#define CMD_TX_ATV                        "ATV1\r\n"                                    // 1 = verbose mode or 0 = numeric mode default 1
#define CMD_TX_ATE                        "ATE1\r\n"                                    // echo mode :0 = off, 1 = on default = 1
#define CMD_TX_AT_CMEE                    "AT+CMEE=2\r\n"                           // for error code/ response(values = 0,1,2) - factory default = 1
#define CMD_TX_AT_QURCCFG                 "AT+QURCCFG=\"URCPORT\",\"usbat\"\r\n" // Unsolicited Result Codes port for notifications from quectel
/* init rx messages */
#define CMD_RX_AT                          "AT\r\r\nOK\r\n"
#define CMD_RX_ATV                         "ATV1\r\r\nOK\r\n"
#define CMD_RX_ATE                         "ATE1\r\r\nOK\r\n"
#define CMD_RX_AT_CMEE                     "AT+CMEE=2\r\r\nOK\r\n" 
#define CMD_RX_AT_QURCCFG                  "AT+QURCCFG=\"URCPORT\",\"usbat\"\r\r\nOK\r\n" //notifications or status updates are routed to usb port
/* SMS commands */    
#define CMD_TX_AT_CMGF                     "AT+CMGF=1\r\n" // 0 for pdu mode, 1 for text mode  // factory default = 0
#define CMD_TX_AT_CSCS                     "AT+CSCS=\"GSM\"\r\n" // factory default = gsm
#define CMD_RX_AT_CMGF                     "AT+CMGF=1\r\r\nOK\r\n"    
#define CMD_RX_AT_CSCS                     "AT+CSCS=\"GSM\"\r\r\nOK\r\n"    
/* device info tx commands */    
#define CMD_TX_ATI                         "ATI\r\n"  // to get lte module info
#define CMD_TX_AT_GSN                      "AT+GSN\r\n"  // Request International Mobile Equipment Identity (IMEI)
#define CMD_TX_AT_CIMI                     "AT+CIMI\r\n" //  International Mobile Subscriber Identity (IMSI)
#define CMD_TX_AT_QCCID                    "AT+QCCID\r\n" //  (Integrated Circuit Card Identifier) to identify sim. can be seen on top of sim card
#define CMD_TX_AT_CSQ                      "AT+CSQ\r\n" // SIGNAL STRENGTH
#define CMD_TX_AT_CREG                     "AT+CREG?\r\n"  // network registration status 
#define CMD_TX_AT_CGREG                    "AT+CGREG?\r\n"  // network registration status
#define CMD_TX_AT_COPS                     "AT+COPS?\r\n"
#define CMD_TX_AT_CEREG                    "AT+CEREG?\r\n"
/* device info rx commands */    
#define CMD_RX_ATI                         "ATI\r\r\nQuectel\r\nEC200T\r\nRevision: EC200TCNHAR02A14M16\r\n\r\nOK\r\n" // quectel, model id, FW revision id
#define CMD_RX_AT_GSN                      "AT+GSN\r\r\n868735049787957\r\n\r\nOK\r\n" // IMEI
#define CMD_RX_AT_CIMI                     "AT+CIMI\r\r\n404450991920976\r\n\r\nOK\r\n" //  International Mobile Subscriber Identity (IMSI) 
#define CMD_RX_AT_QCCID                    "AT+QCCID\r\r\n+QCCID: 8991000903555272977F\r\n\r\nOK\r\n" // ICCID
#define CMD_RX_AT_CSQ                      "AT+CSQ\r\r\n+CSQ: 31,99\r\n\r\nOK\r\n" /* SIGNAL STRENGTH. CSQ: <rssi>,<ber>  */                                                                     
#define CMD_RX_AT_CREG                     "AT+CREG?\r\r\n+CREG: 0,5\r\n\r\nOK\r\n"
#define CMD_RX_AT_CGREG                    "AT+CGREG?\r\r\n+CGREG: 0,5\r\n\r\nOK\r\n"
#define CMD_RX_AT_COPS                     "AT+COPS?\r\r\n+COPS: 0,0,\"IND airtel\",7\r\n\r\nOK\r\n"
#define CMD_RX_AT_CEREG                    "AT+CEREG?\r\r\n+CEREG: 0,1\r\n\r\nOK\r\n"
/* sms related commands */     
#define CMD_TX_AT_CMGS                     "AT+CMGS="
#define CMD_RX_AT_CMGS                     "AT+CMGS="  
/* baudrate query */     
#define CMD_TX_AT_IPR                      "AT+IPR?\r\n" // to check baudrate. not necessary.
#define CMD_RX_AT_IPR                      "AT+IPR?\r\r\n+IPR: 115200\r\n\r\nOK\r\n"
/* sim pin query */    
#define CMD_TX_AT_CPIN                     "AT+CPIN?\r\n"
#define CMD_RX_AT_CPIN                     "AT+CPIN?\r\r\n+CPIN: READY\r\n\r\nOK\r\n"

#define CMD_TX_SIM_DETECT                  "AT+QSIMDET=0,0\r\n"
#define CMD_RX_SIM_DETECT                  "AT+QSIMDET=0,0\r\r\nOK\r\n"

#define CMD_TX_DEV_RESTART                  "AT+CFUN=1,1\r\n"
#define CMD_RX_DEV_RESTART                  "AT+CFUN=1,1\r\r\nOK\r\n"

#define CMD_TX_AIRPLANE_MODE_OFF            "AT+CFUN=1\r\n"
#define CMD_RX_AIRPLANE_MODE_OFF            "AT+CFUN=1\r\r\nOK\r\n"
#define CMD_TX_AIRPLANE_MODE_ON             "AT+CFUN=0\r\n"
#define CMD_RX_AIRPLANE_MODE_ON             "AT+CFUN=0\r\r\nOK\r\n"
/* GPRS cmds */    
#define CMD_TX_GPRS_APN                    "AT+QICSGP=1,1,\"AIRTELGPRS.COM\",\"\",\"\",0\r\n"
#define CMD_RX_GPRS_APN                    "AT+QICSGP=1,1,\"AIRTELGPRS.COM\",\"\",\"\",0\r\r\nOK\r\n"
#define CMD_TX_GPRS_ACT                    "AT+QIACT=1\r\n"
#define CMD_RX_GPRS_ACT                    "AT+QIACT=1\r\r\nOK\r\n"
#define CMD_TX_GPRS_DEACT                  "AT+QIDEACT=1\r\n"
#define CMD_RX_GPRS_DEACT                  "AT+QIDEACT=1\r\r\nOK\r\n"

#define CMD_TX_CTXTID                      "AT+QHTTPCFG=\"contextid\",1\r\n"
#define CMD_RX_CTXTID                      "AT+QHTTPCFG=\"contextid\",1\r\r\nOK\r\n"
#define CMD_TX_QSSLCFG_SSLCTXID            "AT+QHTTPCFG=\"sslctxid\",1\r\n"
#define CMD_RX_QSSLCFG_SSLCTXID            "AT+QHTTPCFG=\"sslctxid\",1\r\r\nOK\r\n"
#define CMD_TX_QSSLCFG_SNI                 "AT+QSSLCFG=\"sni\",1,1\r\n"
#define CMD_RX_QSSLCFG_SNI                 "AT+QSSLCFG=\"sni\",1,1\r\r\nOK\r\n"
#define CMD_TX_QSSLCFG_CIPHERSUITE         "AT+QSSLCFG=\"ciphersuite\",0,0xFFFF\r\n"
#define CMD_RX_QSSLCFG_CIPHERSUITE         "AT+QSSLCFG=\"ciphersuite\",0,0xFFFF\r\r\nOK\r\n"
#define CMD_TX_QSSLCFG_SSLVERSION          "AT+QSSLCFG=\"sslversion\",1,4\r\n"
#define CMD_RX_QSSLCFG_SSLVERSION          "AT+QSSLCFG=\"sslversion\",1,4\r\r\nOK\r\n"
#define CMD_TX_QSSLCFG_SECLEVEL_1          "AT+QSSLCFG=\"seclevel\",1,1\r\n"
#define CMD_RX_QSSLCFG_SECLEVEL_1          "AT+QSSLCFG=\"seclevel\",1,1\r\r\nOK\r\n"
#define CMD_TX_QFLST                       "AT+QFLST\r\n"
// #define CMD_RX_QFLST                       "AT+QFLST\r\r\n+QFLST: \"cacert.pem\"," // +5
#define CMD_RX_QFLST                       "AT+QFLST\r\n\r\n+QFLST: \"UFS:cacert.pem\"," // +5
#define CMD_TX_QFUPL                       "AT+QFUPL=\"cacert.pem\",%d,100\r\n"
#define CMD_RX_QFUPL                       "AT+QFUPL=\"cacert.pem\",%d,100\r\r\nCONNECT\r\n"
#define CMD_TX_QSSLCFG_CACERT              "AT+QSSLCFG=\"cacert\",1,\"cacert.pem\"\r\n"
#define CMD_RX_QSSLCFG_CACERT              "AT+QSSLCFG=\"cacert\",1,\"cacert.pem\"\r\r\nOK\r\n"
#define CMD_TX_GPRS_STOP                   "AT+QHTTPSTOP\r\n"
#define CMD_RX_GPRS_STOP                   "AT+QHTTPSTOP\r\r\nOK\r\n"
#define CMD_TX_GPRS_SLEEP_DIS              "AT+QSCLK=0\r\n"
#define CMD_RX_GPRS_SLEEP_DIS              "AT+QSCLK=0\r\r\nOK\r\n"
#define CMD_TX_GPRS_SLEEP_EN               "AT+QSCLK=1\r\n"
#define CMD_RX_GPRS_SLEEP_EN               "AT+QSCLK=1\r\r\nOK\r\n"
#define CMD_TX_GPRS_SLEEP_QRY              "AT+QSCLK?\r\n"
#define CMD_RX_GPRS_SLEEP_QRY              "AT+QSCLK?\r\r\n+QSCLK: 1\r\n\r\nOK\r\n"
#define CMD_TX_GPRS_URL                    "AT+QHTTPURL"
#define CMD_TX_GPRS_GET                    "AT+QHTTPGET=30\r\n"
#define CMD_RX_GPRS_GET                    "AT+QHTTPGET=30\r\r\nOK\r\n\r\n+QHTTPGET: 0,200"
#define CMD_TX_GPRS_GETEX                  "AT+QHTTPGETEX=60,%ld,%ld\r\n"
#define CMD_RX_GPRS_GETEX                  "AT+QHTTPGETEX=60,%ld,%ld\r\r\nOK\r\n\r\n+QHTTPGET: 0,206,%ld\r\n"
#define CMD_TX_GPRS_POST                   "AT+QHTTPPOST"
#define CMD_RX_GPRS_POST                   "AT+QHTTPPOST"

#define CMD_TX_QFDEL                       "AT+QFDEL=\"*\"\r\n"
#define CMD_RX_QFDEL                       "AT+QFDEL=\"*\"\r\r\nOK\r\n"
#define CMD_TX_HTTP_STOP                   "AT+QHTTPSTOP\r\n"
#define CMD_RX_HTTP_STOP                   "AT+QHTTPSTOP\r\r\nOK\r\n"

#define CMD_TX_SET_CONTENT_TYPE            "AT+QHTTPCFG=\"contenttype\",2\r\n" 
#define CMD_RX_SET_CONTENT_TYPE            "AT+QHTTPCFG=\"contenttype\",2\r\r\nOK\r\n"  // to post data in (2 :application/octet-stream)

#if(FREE_RTOS)
#define LTE_CLOUD_MAX_RESPONSE_TIME        (120000) // 120 sec for 10ms tick
#define LTE_SMS_MAX_RESPONSE_TIME          (2000)
#define LTE_MAX_RESPONSE_TIME              (500u) // 500 milli second
#elif(THREADX)
#define LTE_CLOUD_MAX_RESPONSE_TIME        (12000) // 120 sec for 10ms tick
#define LTE_SMS_MAX_RESPONSE_TIME          (200)
#define LTE_MAX_RESPONSE_TIME              (50u) // 500 milli second
#endif

#define CMD_TX_READ_FW                     "AT+QHTTPREAD=%ld\r\n"
#define CMD_RX_READ_FW                     "AT+QHTTPREAD=%ld\r\r\nCONNECT\r\n" // length = (27 to 29. varies based on %ld) 
#define FW_FILE_DOWNLOAD_SIZE              (0x2000)
#define CMD_RX_READ_FW_LEN                 (29u) // len of CMD_TX_READ_FW if size of %ld = 4. 
#define CMD_RX_READ_FW_LAST_PKT_LEN        (27u) // len of CMD_TX_READ_FW if size of %ld = 2.
#define CMD_CLOUD_PACKET_ST_SIZE           (12u)
#define CMD_TX_GPRS_POST_READ              "AT+QHTTPREAD=%ld\r\n"
#define CMD_RX_GPRS_POST_READ              "AT+QHTTPREAD=%ld\r\r\nCONNECT\r\nSuccess\r\nOK\r\n\r\n+QHTTPREAD: 0\r\n"

// #define FW_DOWNLOAD_URL                     "https://gemini-active-firmware-versions.s3.eu-west-2.amazonaws.com/firmware_AURA_AURA_v00.00.0090.bin"
#if (AWS_SERVER)
#define FW_POST_URL                         "https://ipgsm.emcus.co.in/api/logs/gemini"
#define FW_DOWNLOAD_URL                     "https://ipgsm.emcus.co.in/api/files/gsm/downloadRange/gemini/firmware_AURA.bin"
#elif(AZURE_SERVER)
#define FW_DOWNLOAD_URL                     "http://kiddeapi.emcus.co.in/api/files/gsm/downloadRange/gemini/firmware_AURA_AURA_v00.00.0055_Test.bin"
#define FW_POST_URL                         "http://kiddeapi.emcus.co.in/api/logs/gemini"
#elif(GOOGLE_SERVER)
#define FW_DOWNLOAD_URL                     "https://us-central1-ip-gsm-ts.cloudfunctions.net/api/files/gsm/downloadRange/gemini/firmware_AURA_AURA_v00.00.0055_Test.bin"
#define FW_POST_URL                         "https://us-central1-ip-gsm-ts.cloudfunctions.net/api/logs/gemini"
#endif

#define BIN_FILE_SIZE                       (2785297u) // 2785297 % 0x50 = 17 (or) 2785297 % 0x1000 = 17 (last 17 bytes = release version)

/*************************************************************** Variables ******************************************************************/
static at_cmd_def at_commands;
static lte_credentials_def lte_db = {
    .cloud_api_post = FW_POST_URL,
    .cloud_api_firmware = FW_DOWNLOAD_URL,
};
static firmware_upgrade_def fw_upgrade = {
    .value_to_read = BIN_FILE_SIZE,
    .start_value = 0,
    .packet_number = 1u};
#if (DEV_TEST)
static uint16_t test_message_count = 1u;
static uint16_t test_cloud_count = 1u;
#endif

/*************************************************************** Functions ******************************************************************/
/**
 * @brief 
 * @author Vetrivel
 * @param data 
 * @param length 
 * @return uint16_t 
 */
uint16_t quectel_xor_checksum(const uint8_t *data, size_t length)
{
  uint16_t checksum = 0;
  for (size_t i = 0; i + 1 < length; i += 2)
  {
    // Combine two bytes into one 16-bit word (big-endian)
    uint16_t word = (data[i] << 8) | data[i + 1];
    checksum ^= word;
  }

  if (length % 2 != 0)
  {
    // Handle last byte: make it the high byte, low byte = 0
    uint16_t last_word = data[length - 1] << 8;
    checksum ^= last_word;
  }
  return checksum;
}

/**
 * @brief This function transmits and verifies received AT commands from quectel module (raw without reset )
 * @author Vetrivel
 * @param tx_cmd buffer to transmit AT commands
 * @param rx_cmd buffer to verify received AT commands
 * @param state rtos event flag to wait till receive
 * @param wait_period wait period until verification
 * @param max_retry no of retries before exit.
 * @return uint8_t error status
 */
static uint8_t lte_at_cmds_raw(uint8_t tx_cmd[], uint8_t rx_cmd[], uint8_t state, uint16_t wait_period, uint8_t max_retry, uint16_t extra_rx_data_len, uint16_t tx_len)
{
#if (THREADX)
  ULONG flag_value;
#elif (FREE_RTOS)
#endif
  uint8_t status = ERROR;
  at_commands.cmd_state = state; // should be assigned before tx
  for (uint8_t retry_count = 0; (retry_count < max_retry);)
  {
    lte_send_at_cmds(tx_cmd, rx_cmd, extra_rx_data_len, tx_len);
    if (
#if (THREADX)
        (TX_SUCCESS == tx_event_flags_get(&event_lte_rx, state, TX_OR_CLEAR, &flag_value, wait_period)) &&
#elif (FREE_RTOS)
        (state == xEventGroupWaitBits(event_lte_rx, state, pdTRUE, pdFALSE, wait_period)) &&
#endif
        (SUCCESS == lte_verify_at_rx_cmds(rx_cmd)))
    {
      status = SUCCESS;
      retry_count = max_retry; // to exit loop
    }
    else
    {
      lte_recv(20u);
      print_hex((uint8_t *)get_lte_rx_buff(), strlen((char *)get_lte_rx_buff()));
      status = ERROR;
      retry_count++;
      // no data (or) wrong data received from module
    }
  }
  return status;
}

/**
 * @brief
 * @author Vetrivel
 */
static void lte_reset_cmd(void)
{
#if (THREADX)
  tx_thread_sleep(LTE_MAX_RESPONSE_TIME);
#elif (FREE_RTOS)
  osDelay(LTE_MAX_RESPONSE_TIME);
#endif
  debug_msg((uint8_t *)get_lte_rx_buff());
  if (SUCCESS == lte_at_cmds_raw((uint8_t *)CMD_TX_AT, (uint8_t *)CMD_RX_AT, LTE_INIT_FLAG, LTE_MAX_RESPONSE_TIME, 10u, 0u, 0u))
  {
    debug_msg((uint8_t *)"\nreset done\0");
  }
  else
  {
    debug_msg((uint8_t *)"\nreset failed\0");
  }
}

/**
 * @brief
 * @author Vetrivel
 * @param state
 * @param no_of_cmds
 * @param wait_period
 */
static uint8_t lte_process_at_cmds(void (*fn_ptr)(void), uint32_t state, uint8_t no_of_cmds, uint32_t wait_period, uint16_t extra_rx_data_len, uint16_t tx_len)
{
  uint8_t retry_count = 0;
  uint8_t status = ERROR;
  for (at_commands.cmd_count = 0; ((at_commands.cmd_count < no_of_cmds) && (retry_count < 3u));)
  {
    if (SUCCESS == lte_at_cmds_raw(at_commands.at_tx_msg[at_commands.cmd_count], at_commands.at_rx_msg[at_commands.cmd_count],
                                   state, wait_period, 1u, extra_rx_data_len, tx_len))
    {
      at_commands.cmd_count++;
      status = SUCCESS;
    }
    else
    {
      print_hex((uint8_t *)get_lte_rx_buff(), strlen((char *)get_lte_rx_buff()));
      fn_ptr();
      at_commands.cmd_count = 0u;
      retry_count++;
      status = ERROR;
    }
  }
  return status;
}

/**
 * @brief
 * @author Vetrivel
 */
static void lte_pdp_error_handle(void)
{
#if (THREADX)
  tx_thread_sleep(LTE_MAX_RESPONSE_TIME);
#elif (FREE_RTOS)
  osDelay(LTE_MAX_RESPONSE_TIME);
#endif
  debug_msg((uint8_t *)get_lte_rx_buff());
  if ((SUCCESS == lte_at_cmds_raw((uint8_t *)CMD_TX_GPRS_DEACT, (uint8_t *)CMD_RX_GPRS_DEACT, LTE_INIT_FLAG, LTE_MAX_RESPONSE_TIME, 3u, 0u, 0u)) &&
      (SUCCESS == lte_at_cmds_raw((uint8_t *)CMD_TX_GPRS_ACT, (uint8_t *)CMD_RX_GPRS_ACT, LTE_INIT_FLAG, LTE_MAX_RESPONSE_TIME, 3u, 0u, 0u)))
  {
    debug_msg((uint8_t *)"\npdp error handled done\0");
  }
  else
  {
    debug_msg((uint8_t *)"\npdp error handling fail\0");
  }
}

/**
 * @brief 
 * @author Vetrivel
 */
void lte_http_busy_err_handle(void)
{
  if (SUCCESS == lte_at_cmds_raw((uint8_t *)CMD_TX_HTTP_STOP, (uint8_t *)CMD_RX_HTTP_STOP, LTE_INIT_FLAG, 5000, 10u, 0u, 0u))
  {
    debug_msg((uint8_t *)"\nhttp stop done\n\0");
  }
  else
  {
    debug_msg((uint8_t *)"\nhttp stop failure\n\0");
  }
}

/**
 * @brief
 * @author Vetrivel
 * @param data
 * @param event_type
 * @param dest_size
 * @param source_size
 * @param wait_option
 */
void lte_send_queue_isr(uint8_t *source_data, uint8_t event_type, uint8_t source_size, uint8_t thread_isr_mode)
{
  queue_data_def lte_data = {0};
  if (source_size != NULL)
  {
    lte_data.tx_data_len = source_size;
    buff_cpy(lte_data.tx_data, source_data, sizeof(lte_data.tx_data), source_size);
  }
  lte_data.service = event_type;

  if(thread_isr_mode == SEND_FROM_ISR)
  {
#if (THREADX)
    tx_queue_send(&queue_lte, (queue_data_def *)&lte_data, TX_NO_WAIT);
#elif (FREE_RTOS)
    BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
    xQueueSendFromISR(queue_lte, (queue_data_def *)&lte_data, &pxHigherPriorityTaskWoken);
#endif
  }
  else
  {
#if (THREADX)
    tx_queue_send(&queue_lte, (queue_data_def *)&lte_data, TX_WAIT_FOREVER);
#elif (FREE_RTOS)
    BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
    xQueueSend(queue_lte, (queue_data_def *)&lte_data, ( TickType_t ) 100u);
#endif    
  }
}

/**
 * @brief Set the event bits from isr object
 * @author Vetrivel
 */
void set_event_bits_from_isr(void)
{
#if (THREADX)
  tx_event_flags_set(&event_mcu_comm_rx, LTE_CLOUD_FW_UPLOAD_FLAG, TX_OR);
#elif (FREE_RTOS)
  BaseType_t xHigherPriorityTaskWoken, xResult;
  /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
  xHigherPriorityTaskWoken = pdFALSE;
  /* Set bit 0 and bit 4 in xEventGroup. */
  xResult = xEventGroupSetBitsFromISR(event_mcu_comm_rx, LTE_CLOUD_FW_UPLOAD_FLAG, &xHigherPriorityTaskWoken);
  /* Was the message posted successfully? */
  if (xResult != pdFAIL)
  {
    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
       switch should be requested. The macro used is port specific and will
       be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
       the documentation page for the port being used. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#endif
}

/**
 * @brief Set the eth event bits from isr object
 * @author Vetrivel
 */
void set_eth_event_bits_from_isr(void)
{
#if (THREADX)
  tx_event_flags_set(&event_lte_rx, at_commands.cmd_state, TX_OR);
#elif (FREE_RTOS)
  BaseType_t xHigherPriorityTaskWoken, xResult;
  /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
  xHigherPriorityTaskWoken = pdFALSE;
  /* Set bit 0 and bit 4 in xEventGroup. */
  xResult = xEventGroupSetBitsFromISR(event_eth_rx, ETH_CLOUD_FW_UPLOAD_FLAG, &xHigherPriorityTaskWoken);
  /* Was the message posted successfully? */
  if (xResult != pdFAIL)
  {
    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
       switch should be requested. The macro used is port specific and will
       be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
       the documentation page for the port being used. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#endif
}

void lte_config_ssl_cert(void)
{
  scp_cnf *pf_scp_config = (scp_cnf *)INT_FLASH_SSL_CERT_ADDR;
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  sprintf((char *)at_commands.at_tx_msg[0], "%s", CMD_TX_QFDEL);
  sprintf((char *)at_commands.at_rx_msg[0], "%s", CMD_RX_QFDEL);
  if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FLAG, 1, LTE_MAX_RESPONSE_TIME, 0u, 0u))
  {
    debug_msg((uint8_t *)"\nca cert file deleted\n\0");
    buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
    sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_QFUPL, strlen((const char *)pf_scp_config->https.cert));
    sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_QFUPL, strlen((const char *)pf_scp_config->https.cert));
    if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FLAG, 1, LTE_MAX_RESPONSE_TIME, 0u, 0u))
    {
      debug_msg((uint8_t *)"\nca cert uploading\n\0");
      buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
      sprintf((char *)at_commands.at_tx_msg[0], "%s", pf_scp_config->https.cert);
      sprintf((char *)at_commands.at_rx_msg[0], "+QFUPL: %d,%x\r\n\r\nOK\r\n", strlen((const char *)pf_scp_config->https.cert),
              quectel_xor_checksum(pf_scp_config->https.cert, strlen((const char *)pf_scp_config->https.cert)));
      if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FLAG, 1, LTE_MAX_RESPONSE_TIME, 0u, 0u))
      {
        debug_msg((uint8_t *)"\nca cert upload success\n\0");
        buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
        sprintf((char *)at_commands.at_tx_msg[0], "%s", CMD_TX_QSSLCFG_CACERT);
        sprintf((char *)at_commands.at_rx_msg[0], "%s", CMD_RX_QSSLCFG_CACERT);
        if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FLAG, 1, LTE_MAX_RESPONSE_TIME, 0u, 0u))
        {
          debug_msg((uint8_t *)"\nca cert configured successfully\n\0");
          // successfully configured ca cert
        }
        else
        {
          debug_msg((uint8_t *)"\nca cert configure failed\n\0");
        }
      }
      else
      {
        debug_msg((uint8_t *)"\nca cert upload fail\n\0");
      }
    }
    else
    {
      debug_msg((uint8_t *)"\nca cert upl cmd failed\n\0");
    }
  }
  else
  {
    debug_msg((uint8_t *)"\nca cert file delete failed\n\0");
  }
}
/**
 * @brief
 * @author Vetrivel
 */
void lte_config_ssl_cmds(void)
{
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  sprintf((char *)at_commands.at_tx_msg[0], "%s", CMD_TX_CTXTID);
  sprintf((char *)at_commands.at_rx_msg[0], "%s", CMD_RX_CTXTID);
  sprintf((char *)at_commands.at_tx_msg[1], "%s", CMD_TX_QSSLCFG_SSLCTXID);
  sprintf((char *)at_commands.at_rx_msg[1], "%s", CMD_RX_QSSLCFG_SSLCTXID);
  sprintf((char *)at_commands.at_tx_msg[2], "%s", CMD_TX_QSSLCFG_SNI);
  sprintf((char *)at_commands.at_rx_msg[2], "%s", CMD_RX_QSSLCFG_SNI);
  sprintf((char *)at_commands.at_tx_msg[3], "%s", CMD_TX_QSSLCFG_CIPHERSUITE);
  sprintf((char *)at_commands.at_rx_msg[3], "%s", CMD_RX_QSSLCFG_CIPHERSUITE);
  sprintf((char *)at_commands.at_tx_msg[4], "%s", CMD_TX_QSSLCFG_SSLVERSION);
  sprintf((char *)at_commands.at_rx_msg[4], "%s", CMD_RX_QSSLCFG_SSLVERSION);
  sprintf((char *)at_commands.at_tx_msg[5], "%s", CMD_TX_QSSLCFG_SECLEVEL_1);
  sprintf((char *)at_commands.at_rx_msg[5], "%s", CMD_RX_QSSLCFG_SECLEVEL_1);
  if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FLAG, 6, LTE_MAX_RESPONSE_TIME, 0u, 0u))
  {
    debug_msg((uint8_t *)"\nssl basic confs done\n\0");
  }
  else
  {
    debug_msg((uint8_t *)"\nca cert configuration failed\n\0");
    // error in ssl setup
  }
}

/**
 * @brief
 * @author Vetrivel
 */
void lte_init(void)
{
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  sprintf((char *)at_commands.at_tx_msg[0], "%s", CMD_TX_AT);
  sprintf((char *)at_commands.at_rx_msg[0], "%s", CMD_RX_AT);
  sprintf((char *)at_commands.at_tx_msg[1], "%s", CMD_TX_AT_CMEE);
  sprintf((char *)at_commands.at_rx_msg[1], "%s", CMD_RX_AT_CMEE);
  sprintf((char *)at_commands.at_tx_msg[2], "%s", CMD_TX_AT_QURCCFG);
  sprintf((char *)at_commands.at_rx_msg[2], "%s", CMD_RX_AT_QURCCFG);
  sprintf((char *)at_commands.at_tx_msg[3], "%s", CMD_TX_SIM_DETECT);
  sprintf((char *)at_commands.at_rx_msg[3], "%s", CMD_RX_SIM_DETECT);
  if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_INIT_FLAG, 4u, LTE_MAX_RESPONSE_TIME, 0u, 0u))
  {
    debug_msg((uint8_t *)"\nLTE init success\n\0");
  }
  else
  {
    debug_msg((uint8_t *)"\nLTE init failed\n\0");
  }

  lte_pdp_activate();
  lte_config_ssl_cmds(); 
  lte_config_ssl_cert();
}

/**
 * @brief
 * @author Vetrivel
 * @param mobile_num
 * @param message
 */
void lte_send_sms(__packed uint8_t mobile_num[][15], uint8_t *message)
{
  for (uint8_t i = 0; (i < 5 && mobile_num[i] != 0); i++)
  {
    buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
    sprintf((char *)at_commands.at_tx_msg[0], "%s", CMD_TX_AT_CMGF);
    sprintf((char *)at_commands.at_rx_msg[0], "%s", CMD_RX_AT_CMGF);
    sprintf((char *)at_commands.at_tx_msg[1], "%s\"%s\"\r\n", CMD_TX_AT_CMGS, mobile_num[i]);
    sprintf((char *)at_commands.at_rx_msg[1], "%s\"%s\"\r\r\n> ", CMD_RX_AT_CMGS, mobile_num[i]);
    if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_SMS_FLAG, 2, LTE_SMS_MAX_RESPONSE_TIME, 0u, 0u)) // 200 waitperiod = 2 sec (120s) specified in datasheet
    {
      debug_msg((uint8_t *)"\nsms configured\n\0");
      buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
#if (DEV_TEST)
      sprintf((char *)at_commands.at_tx_msg[0], "%ld : %s\r\x1A", test_message_count, message);
      sprintf((char *)at_commands.at_rx_msg[0], "%ld : %s\r\r\n+CMGS:", test_message_count++, message);
#else
      sprintf((char *)at_commands.at_tx_msg[0], "%s\r\x1A", message);
      sprintf((char *)at_commands.at_rx_msg[0], "%s\r\r\n+CMGS: ", message);
#endif                                                                                                    // + 10 bytes for rx after +CMGS: 1\r\n\r\nOK\r\n
      if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_SMS_FLAG, 1, LTE_SMS_MAX_RESPONSE_TIME, 10u, 0u)) // 200 waitperiod = 2 sec (120s) specified in datasheet
      {
        debug_msg((uint8_t *)"\nsms msg sent\n\0");
      }
      else
      {
        debug_msg((uint8_t *)"\nmsg not sent\n\0");
      }
    }
    else
    {
      debug_msg((uint8_t *)"\nsms configure failed\n\0");
    }
  }
}

/**
 * @brief
 * @author Vetrivel
 */
void lte_check_network_status(void)
{
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  sprintf((char *)at_commands.at_tx_msg[0], "%s", CMD_TX_AT_CPIN);
  sprintf((char *)at_commands.at_rx_msg[0], "%s", CMD_RX_AT_CPIN);
  sprintf((char *)at_commands.at_tx_msg[1], "%s", CMD_TX_AT_CREG);
  sprintf((char *)at_commands.at_rx_msg[1], "%s", CMD_RX_AT_CREG);
  sprintf((char *)at_commands.at_tx_msg[2], "%s", CMD_TX_AT_CGREG);
  sprintf((char *)at_commands.at_rx_msg[2], "%s", CMD_RX_AT_CGREG);

  if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FLAG, 3, LTE_MAX_RESPONSE_TIME, 0u, 0u))
  {
    debug_msg((uint8_t *)"\nnetwork good\n\0");
  }
  else
  {
    debug_msg((uint8_t *)"\nnetwork error\n\0");
  }
}

/**
 * @brief
 * @author Vetrivel
 */
void lte_pdp_activate(void)
{
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  sprintf((char *)at_commands.at_tx_msg[0], "%s", CMD_TX_GPRS_DEACT);
  sprintf((char *)at_commands.at_rx_msg[0], "%s", CMD_RX_GPRS_DEACT);
  sprintf((char *)at_commands.at_tx_msg[1], "%s", CMD_TX_GPRS_APN);
  sprintf((char *)at_commands.at_rx_msg[1], "%s", CMD_RX_GPRS_APN);
  sprintf((char *)at_commands.at_tx_msg[2], "%s", CMD_TX_GPRS_ACT);
  sprintf((char *)at_commands.at_rx_msg[2], "%s", CMD_RX_GPRS_ACT);
  if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FLAG, 3, LTE_MAX_RESPONSE_TIME, 0u, 0u))
  {
    debug_msg((uint8_t *)"\npdp act done\n\0");
    // log pdp activate error and manually
    // activate pdp (Packet Data Protocol (PDP) Activation)
    buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
    sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_SET_CONTENT_TYPE);
    sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_SET_CONTENT_TYPE);
    if (SUCCESS == lte_process_at_cmds(lte_http_busy_err_handle, LTE_INIT_FLAG, 1u, 5000, 0u, 0u))
    {
      debug_msg((uint8_t *)"\ncontent type set to 2\n\0");
    }
    else
    {
      debug_msg((uint8_t *)"\ncontent type setting failed\n\0");
    }
  }
  else
  {
    debug_msg((uint8_t *)"\npdp act fail\n\0");
  }
}

/**
 * @brief
 * @author Vetrivel
 * @param cloud
 */
void lte_send_data_to_cloud_get(const uint8_t *cloud)
{
}

/**
 * @brief 
 * @author Vetrivel
 */
uint8_t lte_airplane_mode (void)
{
  uint8_t status = ERROR;
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_AIRPLANE_MODE_ON);
  sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_AIRPLANE_MODE_ON);
  if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_SWITCH_SIM_SLOT, 1, 10000, 0u, 0u))
  {
    buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
    sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_AIRPLANE_MODE_OFF);
    sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_AIRPLANE_MODE_OFF);
    if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_SWITCH_SIM_SLOT, 1, 10000, 0u, 0u))
    {
      lte_reset_cmd();
      debug_msg("\nlte module reset");
      status = SUCCESS;
    }
  }
	return status;
}


/**
 * @brief 
 * @author Vetrivel
 */
uint8_t lte_module_reset (void)
{
  uint8_t status = ERROR;
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_DEV_RESTART );
  sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_DEV_RESTART );
  if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_SWITCH_SIM_SLOT, 1, LTE_SMS_MAX_RESPONSE_TIME, 0u, 0u))
  {
    osDelay(8000);	
    lte_recv(20u);
    lte_reset_cmd();
    debug_msg("\nlte module reset");
    status = SUCCESS;
  }
	return status;
}

/**
 * @brief
 * @author Vetrivel
 * @param cloud
 */
void lte_switch_sim_slot1(void)
{
  debug_msg("\nswitching to sim1");
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); // set for sim slot 2
  if(SUCCESS == lte_airplane_mode())
  {
#if (TEST_LTE_SIM_SWITCH)
    lte_send_queue_isr("Hello from SIM1", LTE_SMS_QUEUE, strlen("Hello from SIM1"), SEND_FROM_THREAD);
#endif
    debug_msg("\nswitched to sim1");
  }
}

/**
 * @brief
 * @author Vetrivel
 * @param cloud
 */
void lte_switch_sim_slot2(void)
{
  debug_msg("\nswitching to sim2");
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); // set for sim slot 2
  if(SUCCESS == lte_airplane_mode())
  {
#if (TEST_LTE_SIM_SWITCH)
    lte_send_queue_isr("Hello from SIM1", LTE_SMS_QUEUE, strlen("Hello from SIM1"), SEND_FROM_THREAD);
#endif
    debug_msg("\nswitched to sim2");
  }
}

/**
 * @brief
 * @author Vetrivel
 * @param cloud
 */
void lte_firmware_download(const uint8_t *cloud)
{
#if (THREADX)
  ULONG flag_value;
#elif (FREE_RTOS)
#endif
  lte_uart_elements_def *buff = get_lte_rx_buff();
  fw_upgrade.mask_bytes = CMD_RX_READ_FW_LEN;
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  sprintf((char *)at_commands.at_tx_msg[0], "%s=%ld,30\r\n", CMD_TX_GPRS_URL, strlen((char *)cloud));
  sprintf((char *)at_commands.at_rx_msg[0], "%s=%ld,30\r\r\nCONNECT\r\n", CMD_TX_GPRS_URL, strlen((char *)cloud));
  sprintf((char *)at_commands.at_tx_msg[1], "%s", cloud);
  sprintf((char *)at_commands.at_rx_msg[1], "\r\nOK\r\n");
  debug_msg((uint8_t *)"queue received\n\0");

  if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FW_DOWNLOAD_FLAG, 2, LTE_CLOUD_MAX_RESPONSE_TIME, 0u, 0u))
  {
    debug_msg((uint8_t *)"\nurl ok\0");
    for (; FW_FILE_DOWNLOAD_SIZE < fw_upgrade.value_to_read;)
    {
#if (THREADX && !DEV_DEBUG_PROTOCOL)
      if ((TX_SUCCESS == tx_event_flags_get(&event_mcu_comm_rx, LTE_CLOUD_FW_UPLOAD_FLAG, TX_OR_CLEAR, &flag_value, 500u)) && (fw_upgrade.retry_count < 3)) // ack from panel
#elif (FREE_RTOS && !DEV_DEBUG_PROTOCOL)
      if ((LTE_CLOUD_FW_UPLOAD_FLAG == xEventGroupWaitBits(event_mcu_comm_rx, LTE_CLOUD_FW_UPLOAD_FLAG, pdTRUE, pdFALSE, 5000u)) && (fw_upgrade.retry_count < 3))
#else
      if (1)
#endif
      {
        buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
        sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_GPRS_GETEX, fw_upgrade.start_value, FW_FILE_DOWNLOAD_SIZE);
        sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_GPRS_GETEX, fw_upgrade.start_value, FW_FILE_DOWNLOAD_SIZE,
                FW_FILE_DOWNLOAD_SIZE + CMD_CLOUD_PACKET_ST_SIZE);
        if (SUCCESS == lte_process_at_cmds(lte_pdp_error_handle, LTE_CLOUD_FW_DOWNLOAD_FLAG, 1u, LTE_CLOUD_MAX_RESPONSE_TIME, 0u, 0u))
        {
          buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
          debug_msg((uint8_t *)"\nget ok\0");
          sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_READ_FW, FW_FILE_DOWNLOAD_SIZE + CMD_CLOUD_PACKET_ST_SIZE);
          sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_READ_FW, FW_FILE_DOWNLOAD_SIZE + CMD_CLOUD_PACKET_ST_SIZE);
          if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FW_DOWNLOAD_READ_FLAG, 1u, LTE_CLOUD_MAX_RESPONSE_TIME,
                                             FW_FILE_DOWNLOAD_SIZE + CMD_CLOUD_PACKET_ST_SIZE + 0x17u, 0u))
          { // 0x17 :length of \r\nOK\r\n\r\n+QHTTPREAD: 0\r\n)
            debug_msg((uint8_t *)"\nread ok\0");
            // if (SUCCESS != check_char(((uint8_t *)buff + CMD_RX_READ_FW_LEN), 0xff, FW_FILE_DOWNLOAD_SIZE)) // check for 0xff
            if (uart_lte_buff.rx_data.u16_crc == crc16((uint8_t *)uart_lte_buff.rx_buff + CMD_RX_READ_FW_LEN,
                                                       FW_FILE_DOWNLOAD_SIZE + (CMD_CLOUD_PACKET_ST_SIZE - 4u)))
            {
              // below fn sends bin data along with start and end packets.
              fw_upgrade.value_to_read = BIN_FILE_SIZE - (uart_lte_buff.rx_data.u16_packet_num * FW_FILE_DOWNLOAD_SIZE); // modify this with packet number
              fw_upgrade.start_value = (uart_lte_buff.rx_data.u16_packet_num * FW_FILE_DOWNLOAD_SIZE);
              fw_upgrade.packet_number = uart_lte_buff.rx_data.u16_packet_num;
#if (DEV_DEBUG_PROTOCOL)                                                                                                                   // comm protocol between PC and stm
              print_hex(((uint8_t *)uart_lte_buff.rx_buff + CMD_RX_READ_FW_LEN + (CMD_CLOUD_PACKET_ST_SIZE - 4u)), FW_FILE_DOWNLOAD_SIZE); // to send raw bin data
#else
              mcu_comm_transmit(((uint8_t *)uart_lte_buff.rx_buff + CMD_RX_READ_FW_LEN), ((uint32_t)FW_FILE_DOWNLOAD_SIZE + 8u + 4u));
#endif
              debug_msg((uint8_t *)"\nbin sent\n\0");
            }
            else
            { // if whole data packet is 0xff, no data will be sent to panel. so ack should be set here.
              debug_msg((uint8_t *)"\ncrc mismatch\0");
              fw_upgrade.retry_count++;
            }
          }
          else
          {
            send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // no response from lte. send nack
            fw_upgrade.value_to_read = 0;
            debug_msg((uint8_t *)"\nread error\0");
          }
        }
        else
        {
          send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // no response from lte. send nack
          fw_upgrade.value_to_read = 0;
          debug_msg((uint8_t *)"\nget error\0");
        }
      }
      else
      {
        send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // no response from panel. send nack
        fw_upgrade.value_to_read = 0;
        debug_msg((uint8_t *)"\nno response from panel\0");
      }
    }

    for (; fw_upgrade.value_to_read != 0;)
    { // last 17 bytes
#if (THREADX && !DEV_DEBUG_PROTOCOL)
      if ((TX_SUCCESS == tx_event_flags_get(&event_mcu_comm_rx, LTE_CLOUD_FW_UPLOAD_FLAG, TX_OR_CLEAR, &flag_value, 500u)) && (fw_upgrade.retry_count < 3)) // ack from panel
#elif (FREE_RTOS && !DEV_DEBUG_PROTOCOL)
      if ((LTE_CLOUD_FW_UPLOAD_FLAG == xEventGroupWaitBits(event_mcu_comm_rx, LTE_CLOUD_FW_UPLOAD_FLAG, pdTRUE, pdFALSE, 5000u)) && (fw_upgrade.retry_count < 3))
#else
      if (1)
#endif
      {
        fw_upgrade.mask_bytes = CMD_RX_READ_FW_LAST_PKT_LEN;
        buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
        sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_GPRS_GETEX, fw_upgrade.start_value, fw_upgrade.value_to_read);
        sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_GPRS_GETEX, fw_upgrade.start_value, fw_upgrade.value_to_read,
                fw_upgrade.value_to_read + CMD_CLOUD_PACKET_ST_SIZE);
        if (SUCCESS == lte_process_at_cmds(lte_pdp_error_handle, LTE_CLOUD_FW_DOWNLOAD_FLAG, 1u, LTE_CLOUD_MAX_RESPONSE_TIME, 0u, 0u))
        {
          buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
          sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_READ_FW, fw_upgrade.value_to_read + CMD_CLOUD_PACKET_ST_SIZE);
          sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_READ_FW, fw_upgrade.value_to_read + CMD_CLOUD_PACKET_ST_SIZE);
          if (SUCCESS == lte_process_at_cmds(lte_reset_cmd, LTE_CLOUD_FW_DOWNLOAD_READ_FLAG, 1u, LTE_CLOUD_MAX_RESPONSE_TIME,
                                             fw_upgrade.value_to_read + CMD_CLOUD_PACKET_ST_SIZE + 0x17, 0u))
          {
            // 0x17 :length of \r\nOK\r\n\r\n+QHTTPREAD: 0\r\n)
#if (DEV_DEBUG_PROTOCOL) // comm protocol between PC and stm
            print_hex(((uint8_t *)uart_lte_buff.rx_buff + CMD_RX_READ_FW_LAST_PKT_LEN + (CMD_CLOUD_PACKET_ST_SIZE - 4u)),
                      fw_upgrade.value_to_read);
#else
            mcu_comm_transmit(((uint8_t *)uart_lte_buff.rx_buff + CMD_RX_READ_FW_LAST_PKT_LEN), ((uint32_t)fw_upgrade.value_to_read + 8u + 4u));

#endif
          }
          else
          {
            send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // no response from lte. send nack
            fw_upgrade.value_to_read = 0;
            debug_msg((uint8_t *)"\nread error\0");
          }
        }
        else
        {
          send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // no response from lte. send nack
          fw_upgrade.value_to_read = 0;
          debug_msg((uint8_t *)"\nget error\0");
        }
#if (THREADX && !DEV_DEBUG_PROTOCOL)
        if ((TX_SUCCESS == tx_event_flags_get(&event_mcu_comm_rx, LTE_CLOUD_FW_UPLOAD_FLAG, TX_OR_CLEAR, &flag_value, 500u)) && (fw_upgrade.retry_count < 3)) // ack from panel
#elif (FREE_RTOS && !DEV_DEBUG_PROTOCOL)
        if ((LTE_CLOUD_FW_UPLOAD_FLAG == xEventGroupWaitBits(event_mcu_comm_rx, LTE_CLOUD_FW_UPLOAD_FLAG, pdTRUE, pdFALSE, 500u)) && (fw_upgrade.retry_count < 3))
#else
        if (1)
#endif
        {
          fw_upgrade.value_to_read = 0; // to exit loop
          debug_msg((uint8_t *)"\nfw upgrade complete\0");
        }
        else
        {
          send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // no response from panel. send nack
          fw_upgrade.value_to_read = 0;
          debug_msg((uint8_t *)"\nno response from panel\0");
        }
      }
      else
      {
        send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // no response from panel. send nack
        fw_upgrade.value_to_read = 0;
        debug_msg((uint8_t *)"\nno response from panel\0");
      }
    }
  }
  else
  {
    send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // no response from lte. send nack
    fw_upgrade.value_to_read = 0;
    debug_msg((uint8_t *)"\nurl error\0");
  }
  // restoring default values
  fw_upgrade.value_to_read = BIN_FILE_SIZE;
  fw_upgrade.start_value = 0;
  fw_upgrade.packet_number = 1u;
}

/**
 * @brief
 * @author Vetrivel
 * @param cloud
 * @param post_data
 */
uint8_t lte_send_data_to_cloud_post(const uint8_t *cloud, uint8_t *post_data, uint16_t post_data_len)
{
  uint8_t status = SUCCESS;
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  sprintf((char *)at_commands.at_tx_msg[0], "%s=%ld,30\r\n", CMD_TX_GPRS_URL, strlen((char *)cloud));
  sprintf((char *)at_commands.at_rx_msg[0], "%s=%ld,30\r\r\nCONNECT\r\n", CMD_TX_GPRS_URL, strlen((char *)cloud));
  sprintf((char *)at_commands.at_tx_msg[1], "%s", cloud);
  sprintf((char *)at_commands.at_rx_msg[1], "\r\nOK\r\n");
  sprintf((char *)at_commands.at_tx_msg[2], "%s=%ld,30\r\n", CMD_TX_GPRS_POST, post_data_len);
  sprintf((char *)at_commands.at_rx_msg[2], "%s=%ld,30\r\r\nCONNECT\r\n", CMD_RX_GPRS_POST, post_data_len);
  if (SUCCESS == lte_process_at_cmds(lte_pdp_error_handle, LTE_CLOUD_POST_FLAG, 3u, LTE_CLOUD_MAX_RESPONSE_TIME, 0u, 0u))
  {
    buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
    buff_cpy((uint8_t *)at_commands.at_tx_msg[0], post_data, post_data_len, post_data_len);
    sprintf((char *)at_commands.at_rx_msg[0], "%s", "\r\nOK\r\n\r\n+QHTTPPOST: 0,201,7\r\n");
    if (SUCCESS == lte_process_at_cmds(lte_pdp_error_handle, LTE_CLOUD_POST_FLAG, 1u, LTE_CLOUD_MAX_RESPONSE_TIME, 0u, post_data_len))
    {
      debug_msg((uint8_t *)"\ncloud msg sent\0");
      buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
      sprintf((char *)at_commands.at_tx_msg[0], CMD_TX_GPRS_POST_READ, 7);
      sprintf((char *)at_commands.at_rx_msg[0], CMD_RX_GPRS_POST_READ, 7);
      if (SUCCESS == lte_process_at_cmds(lte_pdp_error_handle, LTE_CLOUD_POST_FLAG, 1u, LTE_CLOUD_MAX_RESPONSE_TIME, 0u, 0u))
      {
        debug_msg((uint8_t *)"\nread success\0");
      }
      else
      {
        debug_msg((uint8_t *)"\nread error\0");
        status = ERROR;
      }
    }
    else
    {
      debug_msg((uint8_t *)"\ncloud msg not sent\0");
      status = ERROR;
    }
  }
  return status;
}

/**
 * @brief
 * @author Vetrivel
 * @param lte_data_recv
 */
void lte_task_process(uint8_t service, uint8_t *tx_data, uint16_t tx_data_len)
{
  buff_clr((uint8_t *)&at_commands, sizeof(at_commands));
  uint8_t test_buff[20] = {0};
  switch (service)
  {
  case LTE_INIT_QUEUE:
    lte_init();
    break;
  case LTE_PDP_ACT_QUEUE:
    lte_pdp_activate();
    break;
  case LTE_SMS_QUEUE:
#if(TEST_LTE_SMS)
    lte_send_sms(pf_config->GSM.i8_phone_no1, tx_data);
#else
    sprintf((char*)test_buff, "\nrecvd data:%d", *((uint16_t *)tx_data) / 1000);
		debug_msg(test_buff);
    if(*(uint16_t *)tx_data / 1000 == 1)
    {
      lte_send_sms(pf_config->GSM.i8_phone_no1, "fire");
    }  
    else if(*(uint16_t *)tx_data / 1000 == 2)
    {
      lte_send_sms(pf_config->GSM.i8_phone_no1, "fault");
    } 
    else
    {
      lte_send_sms(pf_config->GSM.i8_phone_no1, "general event");
    }
#endif
    break;
  case LTE_CLOUD_GET_QUEUE:
    lte_send_data_to_cloud_get(lte_db.cloud_api_get);
    break;
  case LTE_CLOUD_POST_QUEUE:
    lte_send_data_to_cloud_post(lte_db.cloud_api_post, tx_data, tx_data_len);
    break;
  case LTE_CLOUD_FW_DOWNLOAD_QUEUE:
    lte_firmware_download(lte_db.cloud_api_firmware);
    break;
  case LTE_SWITCH_SIM2:
    lte_switch_sim_slot2();
  break;  
  case LTE_SWITCH_SIM1:
    lte_switch_sim_slot1();
  break;
  }
}

/**
 * @brief
 * @author Vetrivel
 */
void set_lte_bits_from_isr(void)
{
#if (THREADX)
  tx_event_flags_set(&event_lte_rx, at_commands.cmd_state, TX_OR);
#elif (FREE_RTOS)
  BaseType_t xHigherPriorityTaskWoken, xResult;
  /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
  xHigherPriorityTaskWoken = pdFALSE;
  /* Set bit 0 and bit 4 in xEventGroup. */
  xResult = xEventGroupSetBitsFromISR(event_lte_rx, at_commands.cmd_state, &xHigherPriorityTaskWoken);
  /* Was the message posted successfully? */
  // debug_msg("received response from lte\0");
  if (xResult != pdFAIL)
  {
    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
       switch should be requested. The macro used is port specific and will
       be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
       the documentation page for the port being used. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#endif
}

/**
 * @brief
 * @author Vetrivel
 */
void lte_download_file(void)
{
  lte_uart_elements_def *uart_lte_comm = get_lte_rx_buff();
  uint16_t calculated_crc = calculate_crc((uint32_t *)&uart_lte_comm->rx_buff,
                                          ((uint32_t)uart_lte_comm->rx_data.u16_payload_len + 8u));
  // if (uart_lte_comm.rx_data.u16_crc == calculated_crc)
  // {
    switch (uart_lte_comm->rx_data.u16_cmd)
    {
      case 0x1000:
      case 0x1004:
        set_lte_bits_from_isr();
      break;
    }
  // }
  // else
  // {
  //   debug_msg((uint8_t *)"crc mismatch\0");
  //   send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // crc mismatch. send nack
  // }
}

/**
 * @brief
 * @author Vetrivel
 */
void lte_at_cmd_rx_process(void)
{
  if (at_commands.cmd_state != LTE_CLOUD_FW_DOWNLOAD_READ_FLAG)
  {
    set_lte_bits_from_isr();
  }
  else
  {
    uart_lte_buff.rx_data.u16_sof = *(uint16_t *)&uart_lte_buff.rx_buff[0 + fw_upgrade.mask_bytes];
    uart_lte_buff.rx_data.u16_cmd = *(uint16_t *)&uart_lte_buff.rx_buff[2 + fw_upgrade.mask_bytes];
    uart_lte_buff.rx_data.u16_payload_len = *(uint16_t *)&uart_lte_buff.rx_buff[4 + fw_upgrade.mask_bytes];
    uart_lte_buff.rx_data.u16_packet_num = *(uint16_t *)&uart_lte_buff.rx_buff[6 + fw_upgrade.mask_bytes];
    uart_lte_buff.rx_data.pu8_payload = &uart_lte_buff.rx_buff[8 + fw_upgrade.mask_bytes];
    uart_lte_buff.rx_data.u16_crc = *(uint16_t *)&uart_lte_buff.rx_buff[uart_lte_buff.rx_data.u16_payload_len + 8u + fw_upgrade.mask_bytes];
    uart_lte_buff.rx_data.u16_eof = *(uint16_t *)&uart_lte_buff.rx_buff[uart_lte_buff.rx_data.u16_payload_len + 10u + fw_upgrade.mask_bytes];
    lte_download_file();
  }
}

void upg_from_lte(void)
{
  switch (uart_mcu_comm.rx_data.u16_cmd)
  {
  case 0x3000: // panel requested erase command file
    debug_msg((uint8_t *)"requested erase cmd\n\0");
    send_data_frame(MCU_COMM_CMD_FLASH_DELETE, NULL, NULL, 0u);
    break;
  case 0x3001: // panel requested bin file
    debug_msg((uint8_t *)"requested bin file\n\0");
    set_event_bits_from_isr();
    lte_send_queue_isr(NULL, LTE_CLOUD_FW_DOWNLOAD_QUEUE, NULL, SEND_FROM_ISR);
    break;

  case 0x3002: // ack response
    debug_msg((uint8_t *)"received ack\n\0");
    fw_upgrade.retry_count = 0;
    set_event_bits_from_isr();
    break;
  case 0x3003: // nack response
    debug_msg((uint8_t *)"received nack\n\0");
    fw_upgrade.value_to_read += FW_FILE_DOWNLOAD_SIZE;
    fw_upgrade.start_value -= FW_FILE_DOWNLOAD_SIZE;
    fw_upgrade.packet_number--;
    fw_upgrade.retry_count++;
    set_event_bits_from_isr();
    break;
  case 0x3005: // nack response
    debug_msg((uint8_t *)"received last packet nack\n\0");
    fw_upgrade.retry_count++;
    set_event_bits_from_isr();
    break;

  case 0x300A:
    debug_msg((uint8_t *)"received fire/fault log\n\0");
    print_hex((uint8_t *)uart_mcu_comm.rx_buff, 37 + 14);
    lte_send_queue_isr(uart_mcu_comm.rx_buff, LTE_CLOUD_POST_QUEUE, 37 + 14, SEND_FROM_ISR);
    lte_send_queue_isr(&uart_mcu_comm.rx_buff[8], LTE_SMS_QUEUE, 37, SEND_FROM_ISR); // send sms
    lte_send_queue_isr(NULL, LTE_SWITCH_SIM2, NULL, SEND_FROM_ISR); // switch to sim2
    lte_send_queue_isr(NULL, LTE_INIT_QUEUE, NULL, SEND_FROM_ISR); // init sim2
    lte_send_queue_isr(&uart_mcu_comm.rx_buff[8], LTE_SMS_QUEUE, 37, SEND_FROM_ISR); // send sms 
    lte_send_queue_isr(NULL, LTE_SWITCH_SIM1, NULL, SEND_FROM_ISR); // switch to sim 1
    lte_send_queue_isr(NULL, LTE_INIT_QUEUE, NULL, SEND_FROM_ISR); // init sim1
    break;
  }
}

void upg_from_eth(void)
{
#if (THREADX)
#elif (FREE_RTOS)
  BaseType_t xHigherPriorityTaskWoken = pdFAIL;
  BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
  queue_data_def eth_data = {0};
  uint16_t relay_queue_send = 0;
#endif
  switch (uart_mcu_comm.rx_data.u16_cmd)
  {
  case 0x4001: // panel requested bin file
    debug_msg((uint8_t *)"\nbin file\0");
#if (THREADX)
    tx_event_flags_set(&event_eth_rx, ETH_CLOUD_FW_UPLOAD_FLAG, TX_OR);
#elif (FREE_RTOS)
    xEventGroupSetBitsFromISR(event_eth_rx, ETH_CLOUD_FW_UPLOAD_FLAG, &xHigherPriorityTaskWoken);
    eth_data.service = ETH_CLOUD_FW_DOWNLOAD_QUEUE;
    xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
#endif
    break;
  case 0x4000: // panel requested erase command file
    debug_msg((uint8_t *)"\nerase cmd\0");
    send_data_frame(MCU_COMM_IP_CMD_FLASH_DELETE, NULL, NULL, 0u);
    break;

  case 0x4002: // ack response
    debug_msg((uint8_t *)"\nreceived ack\0");
#if (THREADX)
    tx_event_flags_set(&event_eth_rx, ETH_CLOUD_FW_UPLOAD_FLAG, TX_OR);
#elif (FREE_RTOS)
    set_eth_event_bits_from_isr();
#endif
    break;
  case 0x4003: // nack response
    debug_msg((uint8_t *)"\n received nack\0");
#if (THREADX)
    tx_event_flags_set(&event_eth_rx, ETH_CLOUD_FW_UPLOAD_FLAG, TX_OR);
#elif (FREE_RTOS)
    xEventGroupSetBitsFromISR(event_eth_rx, ETH_CLOUD_FW_UPLOAD_FLAG, &xHigherPriorityTaskWoken);
#endif
    break;
  case 0x4005: // nack response
    debug_msg((uint8_t *)"\nlast packet nack\0");
#if (THREADX)
    tx_event_flags_set(&event_eth_rx, ETH_CLOUD_FW_UPLOAD_FLAG, TX_OR);
#elif (FREE_RTOS)
    xEventGroupSetBitsFromISR(event_eth_rx, ETH_CLOUD_FW_UPLOAD_FLAG, &xHigherPriorityTaskWoken);
#endif
  break;
  case 0x400A:
    eth_data.service = ETH_CLOUD_POST_QUEUE;
    memcpy(eth_data.tx_data, uart_mcu_comm.rx_buff, 37 + 14);
    eth_data.tx_data_len = 37 + 14;
	
    xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
		relay_queue_send = ((uart_mcu_comm.rx_buff[9] << 8) | uart_mcu_comm.rx_buff[8]);
    xQueueSendFromISR(queue_relay, (uint16_t *)&relay_queue_send, &pxHigherPriorityTaskWoken);
	
  break;
  }
}

void mcu_comm_rx_process(void)
{
  uart_mcu_comm.rx_count = 0;
  uint8_t buff[30] = {0};
  debug_msg((uint8_t *)"\nmsg rx from panel\0");
  uint16_t calculated_crc = calculate_crc((uint32_t *)&uart_mcu_comm.rx_buff,
                                          ((uint32_t)uart_mcu_comm.rx_data.u16_payload_len + 10u));
  if (uart_mcu_comm.rx_data.u16_crc == calculated_crc)
  {
    debug_msg((uint8_t *)"\ncrc ok\0");
    sprintf((char *)buff, "received reqest as %05X", uart_mcu_comm.rx_data.u16_cmd);
    debug_println(buff);
    switch (uart_mcu_comm.rx_data.u16_cmd)
    {
    case 0x3000:
    case 0x3001:
    case 0x3002:
    case 0x3003:
    case 0x3004:
    case 0x3005:
    case 0x3006:
    case 0x3007:
    case 0x300A:
      // uart_mcu_comm.rx_data.u16_cmd = uart_mcu_comm.rx_data.u16_cmd / 3u;
      upg_from_lte();
    break;
    case 0x4000:
    case 0x4001:
    case 0x4002:
    case 0x4003:
    case 0x4004:
    case 0x4005:
    case 0x4006:
    case 0x4007:
    case 0x400A:
      upg_from_eth();
    break;
    }
  }
  else
  {
    debug_msg((uint8_t *)"crc mismatch\0");
    send_data_frame(MCU_COMM_CMD_FW_UPG_NACK, NULL, NULL, 0u); // crc mismatch. send nack
  }
}

/**
 * @brief
 * @author Vetrivel
 */
void mcu_comm_uart_process(void)
{
  if (1u == uart_mcu_comm.rx_count)
  {
    if(uart_mcu_comm.rx_buff[0] != 0u)
    {
      mcu_comm_receive_it(&uart_mcu_comm.rx_buff[1u], 5u);
    }
    else
    {
      mcu_comm_receive_it(&uart_mcu_comm.rx_buff[0u], 1u);
      uart_mcu_comm.rx_count = 0;
    }
  } 
  else if (6u == uart_mcu_comm.rx_count)
  {
    uart_mcu_comm.rx_data.u16_sof = *(uint16_t *)&uart_mcu_comm.rx_buff[0];
    uart_mcu_comm.rx_data.u16_cmd = *(uint16_t *)&uart_mcu_comm.rx_buff[2];
    uart_mcu_comm.rx_data.u16_payload_len = *(uint16_t *)&uart_mcu_comm.rx_buff[4];
    mcu_comm_receive_it(&uart_mcu_comm.rx_buff[6u], uart_mcu_comm.rx_data.u16_payload_len + 6u);
  }
  else
  {
    uart_mcu_comm.rx_data.u16_packet_num = *(uint16_t *)&uart_mcu_comm.rx_buff[6];
    uart_mcu_comm.rx_data.pu8_payload = &uart_mcu_comm.rx_buff[8];
    uart_mcu_comm.rx_data.scorpio_ser_num = 2000;
    uart_mcu_comm.rx_data.u16_eof = *(uint16_t *)&uart_mcu_comm.rx_buff[uart_mcu_comm.rx_data.u16_payload_len + 10u];

    uart_mcu_comm.rx_buff[uart_mcu_comm.rx_data.u16_payload_len + 8u] = uart_mcu_comm.rx_data.scorpio_ser_num & 0xff;
    uart_mcu_comm.rx_buff[uart_mcu_comm.rx_data.u16_payload_len + 9u] = uart_mcu_comm.rx_data.scorpio_ser_num >> 8u;
    uart_mcu_comm.rx_data.u16_crc = calculate_crc((uint32_t *)uart_mcu_comm.rx_buff, uart_mcu_comm.rx_data.u16_payload_len + 2u + 8u);
		
    // uart_mcu_comm.rx_data.u16_crc = *(uint16_t *)&uart_mcu_comm.rx_buff[uart_mcu_comm.rx_data.u16_payload_len + 8u];
    // overwrite crc and eof
    uart_mcu_comm.rx_buff[uart_mcu_comm.rx_data.u16_payload_len + 10u] = uart_mcu_comm.rx_data.u16_crc & 0xff;
    uart_mcu_comm.rx_buff[uart_mcu_comm.rx_data.u16_payload_len + 11u] = uart_mcu_comm.rx_data.u16_crc >> 8u;
    uart_mcu_comm.rx_buff[uart_mcu_comm.rx_data.u16_payload_len + 12u] = uart_mcu_comm.rx_data.u16_eof & 0xff;
    uart_mcu_comm.rx_buff[uart_mcu_comm.rx_data.u16_payload_len + 13u] = uart_mcu_comm.rx_data.u16_eof >> 8u; 

		char buff[10];
		for(int i =0;i<52;i++)
		{
			sprintf(buff," %02X",uart_mcu_comm.rx_buff[i]);
			debug_msg(buff);
		}
		
    mcu_comm_rx_process();
    mcu_comm_receive_it((uint8_t *)&uart_mcu_comm.rx_buff[0], 1u);
  }
}

/**
 * @brief
 * @author Vetrivel
 * @param huart
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == UART_LTE_INSTANCE)
  {
    lte_at_cmd_rx_process();
  }
  else if (huart->Instance == UART_MCU_COMM_INSTANCE)
  {
    mcu_comm_uart_process();
  }
}

#else
#error "lte_app.c file not included"
#endif
