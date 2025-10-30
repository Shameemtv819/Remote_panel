/**
 **********************************************************************************************************************************************
 * @file    http_server_app.c
 * @author  shameem
 * @brief   http server application designed for static pages request response handling
 **********************************************************************************************************************************************
 */

/************************************************************* Header includes ***************************************************************/
#include "http_server_app.h"

/***************************************************************** Macros ********************************************************************/
#define CONNECTION_STS_STR "Connection: "
#define CONTENT_LENGTH_STR "Content-Length: "
#define TYPE_IMAGE "image/png"
#define TYPE_HTML "text/html"
#define TYPE_JSON "text/plain"

/*************************************************************** Variables ******************************************************************/
scp_cnf *pf_config = (scp_cnf *)INT_FLASH_SSL_CERT_ADDR;

/********************************************************* static Function prototypes *******************************************************/

/********************************************************* Function defenitions *************************************************************/

/*********************************************************************************************************************************************
 * @brief  This function is used generate Json header for success response to html page
 *
 * @param u8_buff : buffer the json string to be stored
 * @param i32_size : size of the payload
 * @param ac_content_type : content type ex:- like json/text,etc
*********************************************************************************************************************************************/
static void http_cli_header_response(char *u8_buff, int i32_size, char *ac_content_type)
{
  sprintf(u8_buff,
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: %s\r\n"
          "Content-Length: %d\r\n"
          "Connection: close\r\n"
          "\r\n",
          ac_content_type, i32_size);
}


/**
 * @brief This function will handle all POST request from http client client. 
 * 
 * @param pu8_http_buf : buffer read from tcp
 * @param conn         : conn structure for netconn communications
 * @param au8_temp_buf : temp buffer to handle http buff
 * @param u16_buflen   : tcp received data len
 */
void handle_http_POST_request(char *pu8_http_buf,struct netconn *conn,
                             char *au8_temp_buf,uint16_t u16_buflen)
{
     uint8_t *au8_post_payload = (uint8_t *)pvPortMalloc(2560);

  uint16_t u16_total_received_payload = CLEAR;
  uint16_t u16_payload_len            = CLEAR;
  uint8_t buff[500]                   = {CLEAR};
  uint16_t u16_json_header_len        = CLEAR;
        
  err_t recv_err = CLEAR;

  uint32_t u32_file_size = CLEAR;

  struct netbuf *inbuf = NULL;
       
        //fetch length of data from json haeder
        memcpy(au8_temp_buf, pu8_http_buf, 50);
        debug_msg((uint8_t *)"\r\nPOST received");
        u32_file_size = get_content_length(pu8_http_buf, CONTENT_LENGTH_STR);

        sprintf((char *)buff, "\nContent-Length: %lu\n", u32_file_size);
        debug_msg((uint8_t *)buff);

        // fetch header length
        u16_json_header_len = (uint16_t)((strstr((char *)pu8_http_buf, "\r\n\r\n") - (char *)pu8_http_buf) + 4); // end of headers
        u16_payload_len = u16_buflen - u16_json_header_len;

        memset(au8_post_payload, 0, sizeof(au8_post_payload));
        if (u16_payload_len > 0)
        {
          memcpy(au8_post_payload, (uint8_t *)pu8_http_buf + u16_json_header_len, u16_payload_len);
        }

        u16_total_received_payload = u16_payload_len;

        // read complete payload
        while (u16_total_received_payload < u32_file_size)
        {
          //read_payload
          recv_err = netconn_recv(conn, &inbuf);
          if (recv_err != ERR_OK)
          {
            debug_msg("recv failed");
            break;
          }

          netbuf_data(inbuf, (void **)&pu8_http_buf, &u16_buflen);
          if (u16_total_received_payload + u16_buflen < 2000)
          {
            memcpy(au8_post_payload + u16_total_received_payload, pu8_http_buf, u16_buflen);
            u16_total_received_payload += u16_buflen;
          }
          else
          {
            debug_msg("Payload too large");
            break;
          }

          netbuf_delete(inbuf);
        }

        au8_post_payload[u16_total_received_payload] = '\0'; // null-terminate
        debug_msg((uint8_t *)"\n--- Received JSON ---\n");
        debug_msg((uint8_t *)au8_post_payload);

        //process request from panel 
        if (0 == strncmp((char *)au8_temp_buf, "POST /certificate_url_config", 28))
        {
          debug_msg((uint8_t *)"Received : POST /certificate_url_config");
          segregate_cert_url_json_payload(au8_post_payload);
        }
        else if (0 == strncmp((char *)au8_temp_buf, "POST /gsm_config", 16))
        {
          debug_msg((uint8_t *)"Received : POST /gsm_config");
          segregate_GSM_conf_json_payload(au8_post_payload);
        }
        else if (0 == strncmp((char *)au8_temp_buf, "POST /ip_config", 15))
        {
          debug_msg((uint8_t *)"Received : POST /ip_config");
          segregate_IP_config_json_payload(au8_post_payload);
          // delay to finish queue writing

          debug_msg((uint8_t *)"\r\nxxxxxxxxxxx IP address changed Restarting device xxxxxxxxxxxx");
          NVIC_SystemReset();
          // Reset the MCU
        }
        else
        {
          debug_msg("\r\nReceived : POST /Not matching\r\n");
        }
 vPortFree(au8_post_payload);
}

void handle_http_GET_request(char *pu8_http_buf,struct netconn *conn,char *au8_temp_buf)
{
  uint8_t buff[500]         = {CLEAR};
  // check for GET request
        if (0 == strncmp((char *)pu8_http_buf, "GET / ", 6))
        {
          uint8_t *status_temp_buf = (uint8_t *)pvPortMalloc(28 * 1024);
          if (status_temp_buf == NULL)
          {
            debug_msg("\r\n mem allocate failed");
          }

          // adding IP to existing html page
          sprintf((char *)status_temp_buf, (char *)au8_login_page_html, pf_config->IP.ip_addr, pf_config->IP.ip_addr,
                  pf_config->IP.ip_addr, pf_config->IP.ip_addr);

          http_cli_header_response(buff, strlen((char *)status_temp_buf), TYPE_HTML);

          // json header
          netconn_write(conn, (const uint8_t *)(buff),
                        (uint16_t)strlen((char *)buff), NETCONN_NOCOPY);

          // data
          netconn_write(conn, (const uint8_t *)(status_temp_buf),

                        (uint16_t)strlen((char *)status_temp_buf), NETCONN_NOCOPY);

          // clearing heap
          vPortFree(status_temp_buf);

          debug_msg("\r\n transmitted login page");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /scorpioStatus", 18))
        {
          // using HEAP for optimized memmory use
          uint8_t *status_temp_buf = (uint8_t *)pvPortMalloc(28 * 1024);
          if (status_temp_buf == NULL)
          {
            debug_msg("\r\n mem allocate failed");
          }

          // adding IP to existing html page
          sprintf((char *)status_temp_buf, (char *)au8_status_page_html, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr);

          http_cli_header_response(buff, strlen((char *)status_temp_buf), TYPE_HTML);

          // json header
          netconn_write(conn, (const uint8_t *)(buff),
                        (uint16_t)strlen((char *)buff), NETCONN_NOCOPY);

          // data
          netconn_write(conn, (const uint8_t *)(status_temp_buf),
                        (uint16_t)strlen((char *)status_temp_buf), NETCONN_NOCOPY);

          // clearing heap
          vPortFree(status_temp_buf);

          debug_msg("\r\n transmitted status page\r\n");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /gsmdata", 12))
        {

          // senting saved lte mobile numbers for messages
          sprintf((char *)au8_temp_buf, "%s %s %s %s %s %d", (char *)pf_config->GSM.i8_phone_no1[0], (char *)pf_config->GSM.i8_phone_no1[1], (char *)pf_config->GSM.i8_phone_no1[2],
                  (char *)pf_config->GSM.i8_phone_no1[3], (char *)pf_config->GSM.i8_phone_no1[4],
                  (char *)pf_config->GSM.u8_gsm_enable);

          netconn_write(conn, (const uint8_t *)au8_temp_buf,
                        strlen((char *)au8_temp_buf), NETCONN_NOCOPY);

          debug_msg((uint8_t *)"\r\ntransmitted gsm_data structure\r\n");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /ipdata", 11))
        {

          // senting saved IP addreses for messages
          sprintf((char *)au8_temp_buf, "%s %s %s %d", (char *)pf_config->IP.ip_addr, (char *)pf_config->IP.net_mask,
                  (char *)pf_config->IP.gate_way, (char *)pf_config->IP.ip_enable);

          netconn_write(conn, (const uint8_t *)au8_temp_buf,
                        strlen((char *)au8_temp_buf), NETCONN_NOCOPY);

          debug_msg((uint8_t *)"\r\ntransmitted ip_data structure\r\n");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /certificatedata", 11))
        {
          // senting current server url
          sprintf((char *)au8_temp_buf, "%s", (char *)pf_config->https.url);

          netconn_write(conn, (const uint8_t *)au8_temp_buf,
                        strlen((char *)au8_temp_buf), NETCONN_NOCOPY);

          debug_msg((uint8_t *)"\r\ntransmitted ip_data structure\r\n");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /status_config", 18))
        {

          // responding request for current credentils
          sprintf((char *)au8_temp_buf, "%s %s %d %d %s %s", (char *)pf_config->version.scorpio_version,
                  (char *)pf_config->version.gemini_version, pf_config->IP.ip_enable,
                  pf_config->GSM.u8_gsm_enable, (char *)pf_config->version.gsm_number, (char *)pf_config->IP.ip_addr);

          debug_msg(au8_temp_buf);
          netconn_write(conn, (const uint8_t *)au8_temp_buf,
                        strlen(au8_temp_buf), NETCONN_NOCOPY);

          debug_msg((uint8_t *)"\r\ntransmitted status structure\r\n");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /certConfigPage", 19))
        {

          uint8_t *status_temp_buf = (uint8_t *)pvPortMalloc(28 * 1024);
          if (status_temp_buf == NULL)
          {
            debug_msg("\r\n mem allocate failed");
          }

          // adding IP to existing html page
          sprintf((char *)status_temp_buf, (char *)au8_cert_url_page_html, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr);

          http_cli_header_response(buff, strlen((char *)status_temp_buf), TYPE_HTML);

          // json header
          netconn_write(conn, (const uint8_t *)(buff),
                        (uint16_t)strlen((char *)buff), NETCONN_NOCOPY);

          // data
          debug_msg((uint8_t *)"loaded cert url page\r\n");
          netconn_write(conn, (const uint8_t *)(status_temp_buf),
                        (uint16_t)strlen((char *)status_temp_buf), NETCONN_NOCOPY);

          vPortFree(status_temp_buf);

          debug_msg("\r\n transmitted cert url page");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /ipConfigPage", 17))
        {
          uint8_t *status_temp_buf = (uint8_t *)pvPortMalloc(28 * 1024);
          if (status_temp_buf == NULL)
          {
            debug_msg("\r\n mem allocate failed");
          }

          // adding IP to existing html page
          sprintf((char *)status_temp_buf, (char *)au8_IP_conf_page_html, pf_config->IP.ip_addr,
                  pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr);

          http_cli_header_response(buff, strlen((char *)status_temp_buf), TYPE_HTML);

          // json header
          netconn_write(conn, (const uint8_t *)(buff),
                        (uint16_t)strlen((char *)buff), NETCONN_NOCOPY);

          // data
          netconn_write(conn, (const uint8_t *)(status_temp_buf),
                        (uint16_t)strlen((char *)status_temp_buf), NETCONN_NOCOPY);

          vPortFree(status_temp_buf);

          debug_msg("\r\n transmitted IP page");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /gsmConfigPage", 18))
        {
          uint8_t *status_temp_buf = (uint8_t *)pvPortMalloc(28 * 1024);
          if (status_temp_buf == NULL)
          {
            debug_msg("\r\n mem allocate failed");
          }

          // adding IP to existing html page
          sprintf((char *)status_temp_buf, (char *)au8_GSM_conf_page_html, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr, pf_config->IP.ip_addr);
          http_cli_header_response(buff, strlen((char *)status_temp_buf), TYPE_HTML);

          // json header
          netconn_write(conn, (const uint8_t *)(buff),
                        (uint16_t)strlen((char *)buff), NETCONN_NOCOPY);

          // data
          netconn_write(conn, (const uint8_t *)(status_temp_buf),
                        (uint16_t)strlen((char *)status_temp_buf), NETCONN_NOCOPY);

          vPortFree(status_temp_buf);
        }

        else if (0 == strncmp((char *)pu8_http_buf, "GET /bg-blue.png", 16))
        {

          http_cli_header_response(buff, sizeof(au8_bg_blue), TYPE_IMAGE);

          // json header
          netconn_write(conn, (const uint8_t *)(buff),
                        (uint16_t)strlen((char *)buff), NETCONN_NOCOPY);
          // data
          netconn_write(conn, (const uint8_t *)(au8_bg_blue),
                        (uint16_t)sizeof(au8_bg_blue), NETCONN_NOCOPY);
          debug_msg("\r\n transmitted  bg-blue.png");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /emcus-logo.png", 19))
        {
          http_cli_header_response(buff, sizeof(au8_emcus_logo), TYPE_IMAGE);

          // json header
          netconn_write(conn, (const uint8_t *)(buff),
                        (uint16_t)strlen((char *)buff), NETCONN_NOCOPY);
          // data
          netconn_write(conn, (const uint8_t *)(au8_emcus_logo),
                        (uint16_t)sizeof(au8_emcus_logo), NETCONN_NOCOPY);

          debug_msg("\r\ntransmitted  emcus_logo.png");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /emcus-icon.png", 19))
        {
          http_cli_header_response(buff, sizeof(au8_emcus_icon), TYPE_IMAGE);

          // json header
          netconn_write(conn, (const uint8_t *)(buff),
                        (uint16_t)strlen((char *)buff), NETCONN_NOCOPY);
          // data
          netconn_write(conn, (const uint8_t *)(au8_emcus_icon),
                        (uint16_t)sizeof(au8_emcus_icon), NETCONN_NOCOPY);

          debug_msg("\r\ntransmitted  emcus-icon.png");
        }
        else if (0 == strncmp((char *)pu8_http_buf, "GET /bg-green.png", 17))
        {
          http_cli_header_response(buff, sizeof(au8_bg_green), TYPE_IMAGE);

          // json header
          netconn_write(conn, (const uint8_t *)(buff),
                        (uint16_t)strlen((char *)buff), NETCONN_NOCOPY);
          // data
          netconn_write(conn, (const uint8_t *)(au8_bg_green),
                        (uint16_t)sizeof(au8_bg_green), NETCONN_NOCOPY);

          debug_msg("\r\ntransmitted  bg_green.png");
        }
        else
        {
          /* Load Error page */
        }

}


/**********************************************************************************************************************************************
  * @brief serve tcp connection. By handling request response from client.

  * @param conn: pointer on connection structure

  * @retval None
 **********************************************************************************************************************************************/

void http_server_serve(struct netconn *conn)
{

  uint8_t au8_temp_buf[100] = {CLEAR};
  uint8_t buff[500]         = {CLEAR};

  uint16_t u16_buflen                 = CLEAR;
  uint16_t u16_payload_len            = CLEAR;


  struct netbuf *inbuf   = NULL;

  err_t recv_err = CLEAR;

  uint8_t *pu8_http_buf     = NULL;
 

  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  recv_err = netconn_recv(conn, &inbuf);

  if (ERR_OK == recv_err)
  {
    if (ERR_OK == netconn_err(conn))
    {
      netbuf_data(inbuf, (void **)&pu8_http_buf, &u16_buflen);
      debug_msg((uint8_t *)pu8_http_buf);
      /* Is this an HTTP GET command? (only check the first 5 chars, since
      there are other formats for GET, and we're keeping it very simple )*/
      if ((u16_buflen >= 5) && (strncmp((char *)pu8_http_buf, "GET /", 5) == 0))
      {

         handle_http_GET_request(pu8_http_buf,conn,au8_temp_buf);
        
      }
      // check for POST request
      else if ((u16_buflen >= 5) && (strncmp((char *)pu8_http_buf, "POST /", 5) == 0))
      {
        handle_http_POST_request(pu8_http_buf,conn, au8_temp_buf,u16_buflen);
        
      }
    }
  }
  /* Close the connection (server closes in HTTP) */

 
  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}
