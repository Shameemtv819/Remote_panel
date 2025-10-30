/**
 **********************************************************************************************************************************************
 * @file           : json_handler.c
 * @brief          : This file contain segregation of jason formatted data from
 *                   static page and cloud.ex:- content length from cloud, Json
 *                   post request.
 **********************************************************************************************************************************************
 */
/* USER CODE END Header */
/************************************************************* Header includes ***************************************************************/

#include "json_handler.h"

/***************************************************************** Macros ********************************************************************/
#define IP_MAX_LEN (16U)
#define URL_MAX_LEN (512U)
#define MOBILE_MAX_LEN (16U)

/*************************************************************** Variables ******************************************************************/
static scp_cnf *sp_scp_config = NULL; // used to copy flash data from flash into heap

/********************************************************* static Function prototypes *******************************************************/
static void fetch_data_from_payload(const uint8_t *au8_json_payload, const char *ac_str_parameter, char *ac_param_buff);

/**********************************************************************************************************************************************
 * @brief  This function will write the config struct to flash task  through flash_queue in small chunks
 * @author Mohammed Shameem
 * @param  p_scp_config   :- pointer to the config structure
 *********************************************************************************************************************************************/
static void write_config_to_flash(void)
{
   uint16_t u16_index = 0;
  queue_data_def flash_data_recv = {0};
  flash_data_recv.service = FLASH_ERASE_QUEUE;

  if (xQueueSend(queue_flash, (queue_data_def *)&flash_data_recv, portMAX_DELAY) != pdPASS)
	{
		debug_msg("\r\nqueue sent failed*************");
	}
  flash_data_recv.service = FLASH_WRITE_QUEUE;

  for (u16_index = 0; u16_index < sizeof(scp_cnf); u16_index += sizeof(flash_data_recv.tx_data))
  {
    if ((u16_index + sizeof(flash_data_recv.tx_data)) > sizeof(scp_cnf))
    {
      flash_data_recv.service = FLASH_WRITE_END_OF_QUEUE;
      flash_data_recv.tx_data_len = (sizeof(scp_cnf) - u16_index);
    }
    else
    {
      
      flash_data_recv.tx_data_len = sizeof(flash_data_recv.tx_data);
    }

    buffer_cpy((uint8_t *)flash_data_recv.tx_data, sizeof(flash_data_recv.tx_data), ((uint8_t *)sp_scp_config) + u16_index, flash_data_recv.tx_data_len);
		if ( xQueueSend(queue_flash, &flash_data_recv, portMAX_DELAY) != pdPASS) 
		{
			debug_msg("\r\nqueue sent failed*************");
      // Handle queue full error
    }
  }
}

/**********************************************************************************************************************************************
 * @brief  This function will extract the the next the ac_str_parameter and write it in ac_param_buff
 * @author Mohammed Shameem
 * @param  request   :- JSON requess/resposne from panel
 *********************************************************************************************************************************************/
static void fetch_data_from_payload(const uint8_t *au8_json_payload, const char *ac_str_parameter, char *ac_param_buff)
{
  uint16_t u16_len;
  char *p_str_end, *p_str_start;
  // Extract Facility Manager mobile number
  p_str_start = strstr((char *)au8_json_payload, ac_str_parameter);
  if (p_str_start)
  {
    p_str_start += strlen(ac_str_parameter);
    p_str_end = strchr(p_str_start, '"');
    if (p_str_end)
    {
      u16_len = p_str_end - p_str_start;
      strncpy((char *)ac_param_buff, p_str_start, u16_len);
      ac_param_buff[u16_len] = '\0';
    }
  }
}
/**********************************************************************************************************************************************
 * @brief  This function will extract specific value in numbers from passed JSON
           -file according to fetch_str
 * @author Mohammed Shameem
 * @param  request   :- JSON requess/resposne from panel
           fetch_str :- This will reprecent the string prior to the number to be fetched
 *********************************************************************************************************************************************/
uint32_t get_content_length(const uint8_t *request, const char *fetch_str)
{
  // Find the position of "Content-Length:"
  char *pos = strstr((char *)request, fetch_str);
  long int content_length = 0;
  uint8_t buff[30] = {0};
  if (pos)
  {
    // Move pointer to the start of the length value
    pos += strlen(fetch_str);

    // Use strtol to extract the full number
    content_length = strtol(pos, NULL, 10);
    // Print the extracted length
    sprintf((char *)buff, "Content-Length: %ld\n", content_length);
    debug_msg(buff);
  }
	else
	{
		sprintf((char *)buff, "Content-Length: %ld\n", content_length);
    debug_msg(buff);
	}
  return content_length;
}

/**********************************************************************************************************************************************
 * @brief  This function will extract the data entered in the hosted
           -static page JSON request
 * @author Mohammed Shameem
 * @param  request   :- JSON requess/resposne from panel
 *********************************************************************************************************************************************/
// Function to extract JSON from HTTP request
char *extract_json_payload(const uint8_t *request)
{
  char *json_start = strstr((char *)request, "\r\n\r\n"); // Find start of JSON
  char *c_ret = NULL;
  if (json_start)
  {
    c_ret = json_start + 4; // Skip "\r\n\r\n" to reach JSON data
  }
  return c_ret;
}

/**********************************************************************************************************************************************
 * @brief  This function will extract the data entered in IP config static page
 * @author Mohammed Shameem
 * @param  request   :- JSON requess/resposne from panel
 *********************************************************************************************************************************************/
void segregate_IP_config_json_payload(const uint8_t *au8_json_payload)
{
  char buff[10];
  scp_cnf *pf_scp_config = (scp_cnf *)INT_FLASH_SSL_CERT_ADDR;

  sp_scp_config = (scp_cnf *)pvPortMalloc(sizeof(scp_cnf));
  buffer_cpy((uint8_t *)sp_scp_config, sizeof(scp_cnf), (uint8_t *)pf_scp_config, sizeof(scp_cnf));

  ip_config *p_ip_config = &sp_scp_config->IP;

  memset((uint8_t *)p_ip_config, 0x00, sizeof(ip_config));

  fetch_data_from_payload(au8_json_payload, "\"staticIP\":\"",(char *)p_ip_config->ip_addr);

  fetch_data_from_payload(au8_json_payload, "\"netMask\":\"", (char *)p_ip_config->net_mask);

  fetch_data_from_payload(au8_json_payload, "\"gatewayAddress\":\"",(char *)p_ip_config->gate_way);

  fetch_data_from_payload(au8_json_payload, "\"ipSelection\":\"", buff);
  if(!(strcmp(buff,"ENABLED")))
  {
     p_ip_config->ip_enable = 1;
  }
  else
  {
    p_ip_config->ip_enable = 0;
  }

  write_config_to_flash();
  debug_msg("\r\nIP :");
  debug_msg(p_ip_config->ip_addr);

  debug_msg("net_mask :");
  debug_msg(p_ip_config->net_mask);

  debug_msg("gateway_addr :");
  debug_msg(p_ip_config->gate_way);

  debug_msg("enable/disable :");
  debug_msg(&p_ip_config->ip_enable);
  vPortFree(sp_scp_config);
}

/**********************************************************************************************************************************************
 * @brief  This function will extract the data entered in certificate and url static page
 * @author Mohammed Shameem
 * @param  request   :- JSON requess/resposne from panel
 *********************************************************************************************************************************************/
void segregate_cert_url_json_payload(const uint8_t *au8_json_payload)
{
  // char url[100], cert[2000];
  uint16_t u16_len;
  char *cert_end, *cert_start;
  uint8_t buff[10] = {0};
  scp_cnf *pf_scp_config = (scp_cnf *)INT_FLASH_SSL_CERT_ADDR;
  sp_scp_config = (scp_cnf *)pvPortMalloc(sizeof(scp_cnf));
  uint8_t *temp_cert_buff = (uint8_t *) pvPortMalloc(3000);
  buffer_cpy((uint8_t *)sp_scp_config, sizeof(scp_cnf), (uint8_t *)pf_scp_config, sizeof(scp_cnf));

  cert_url *p_https_conf = &sp_scp_config->https;

	memset(p_https_conf, 0x00, sizeof(cert_url));
	
  fetch_data_from_payload(au8_json_payload, "\"cloudURL\":\"", (char *)p_https_conf->url);

  cert_start = strstr((char *)au8_json_payload, "-----BEGIN CERTIFICATE-----");
  if (cert_start)
  {
    cert_end = strstr(cert_start, "-----END CERTIFICATE-----");
    cert_end += strlen("-----END CERTIFICATE-----");
    debug_msg("\r\nURL :");
    sprintf((char *)buff, "%d\r\n", cert_start);
    debug_msg(buff);

    debug_msg("cert :");
    sprintf((char *)buff, "%d\r\n", cert_end);
    debug_msg(buff);
    if (cert_end)
    {
      u16_len = cert_end - cert_start;
      strncpy((char *)temp_cert_buff, cert_start, u16_len);
      temp_cert_buff[u16_len] = '\0';
    }
  }
  debug_msg("\r\ncert before  : \n");
  debug_msg((uint8_t *)temp_cert_buff);


    int j = 0,i=0;
    for ( i = 0; temp_cert_buff[i] != '\0' && j < 3000 - 1; ++i) 
    {
        if (temp_cert_buff[i] == '\\' && temp_cert_buff[i + 1] == 'n')
        {
            p_https_conf->cert[j++] = '\n';
            i++; // skip the 'n'
        } 
        else 
        {
            p_https_conf->cert[j++] = temp_cert_buff[i];
        }
    }
    p_https_conf->cert[j] = '\0';

  debug_msg("\r\nurl : ");
  debug_msg((uint8_t *)p_https_conf->url);
  debug_msg("\r\ncert : ");
  debug_msg((uint8_t *)p_https_conf->cert);

  write_config_to_flash();

  vPortFree(sp_scp_config);
  vPortFree(temp_cert_buff);
}


int extract_gsm_selection(const char *json_str) {
    const char *key = "\"gsmSelection\":";
    char *found = strstr(json_str, key);
    int value = -1; // Default error value

    if (found) {
        sscanf(found + strlen(key), "%d", &value);
    }

    return value;
}

/**********************************************************************************************************************************************
 * @brief  This function will extract the data entered in GSM config static page
 * @author Mohammed Shameem
 * @param  request   :- JSON requess/resposne from panel
 *********************************************************************************************************************************************/
void segregate_GSM_conf_json_payload(const uint8_t *au8_json_payload)
{
  char buff[10];
  scp_cnf *pf_scp_config = (scp_cnf *)INT_FLASH_SSL_CERT_ADDR;

  sp_scp_config = (scp_cnf *)pvPortMalloc(sizeof(scp_cnf));
  buffer_cpy((uint8_t *)sp_scp_config, sizeof(scp_cnf), (uint8_t *)pf_scp_config, sizeof(scp_cnf));

  gsm_config *p_gsm_config = &sp_scp_config->GSM;
	
	memset(p_gsm_config, 0x00, sizeof(gsm_config));

  fetch_data_from_payload(au8_json_payload, "\"facilityManager\":\"", (char *)p_gsm_config->i8_phone_no1[0]);

  fetch_data_from_payload(au8_json_payload, "\"buildingOwner\":\"", (char *)p_gsm_config->i8_phone_no1[1]);

  fetch_data_from_payload(au8_json_payload, "\"phone1\":\"", (char *)p_gsm_config->i8_phone_no1[2]);

  fetch_data_from_payload(au8_json_payload, "\"phone2\":\"", (char *)p_gsm_config->i8_phone_no1[3]);

  fetch_data_from_payload(au8_json_payload, "\"phone3\":\"", (char *)p_gsm_config->i8_phone_no1[4]);

  fetch_data_from_payload(au8_json_payload, "\"gsmSelection\":\"", buff);


  if(!(strcmp(buff,"ENABLED")))
  {
     p_gsm_config->u8_gsm_enable = 1;
  }
  else
  {
    p_gsm_config->u8_gsm_enable = 0;
  }
  write_config_to_flash();

  debug_msg("ph no 1:");
  debug_msg(p_gsm_config->i8_phone_no1[0]);
  debug_msg("ph no 2:");
  debug_msg(p_gsm_config->i8_phone_no1[1]);
  debug_msg("ph no 3:");
  debug_msg(p_gsm_config->i8_phone_no1[2]);
  debug_msg("ph no 4:");
  debug_msg(p_gsm_config->i8_phone_no1[3]);
  debug_msg("ph no 5:");
  debug_msg(p_gsm_config->i8_phone_no1[4]);
  vPortFree(sp_scp_config);
}
