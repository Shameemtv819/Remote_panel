/**
 * **********************************************************************************************************************************************
 * @file   https_client_app.c
 * @author Shameem
 * @brief  complete operation ota download and fire/fault senting to cloud. application contains tcp + wolfssl(https stack).
 **********************************************************************************************************************************************
 * @date   2025-05-09
 * @copyright Copyright (c) 2025
 * **********************************************************************************************************************************************
 */

/************************************************************* Header includes ***************************************************************/
#include "https_client_app.h"

/***************************************************************** Macros ********************************************************************/
#define GOOGLE                  (0U)
#define MAXLINE                 (256U)
#define MODE_CHUNK         	    (1U)
#define CHUNK_SIZE              (0x2000U)
#define SERVER_PORT        	    (443U)
#define WOLFSS_DEBUG 			(1U)
#define HTTPS_STREAM 			(1U)
#define FORMAT_BYTES 			(12U)
#define RETRY_DELAY_MS 			(100U)
#define MAX_SEND_RETRIES    	(3U)
#define MAX_RECV_RETRIES    	(3U)
#define RECV_RETRY_DELAY_MS  	(100U)
#define NO_INTERNET_FOR_DNS     (0)
#define ETH_FW_DOWNLOAD_FAILED  (2U)
#define MAX_JSON_HEADER_SIZE    (0x600U)
#define CLEAR                   (0U)

#define CONTENT_LENGTH_STR "file-size: "
#define POST_API_LOG       "/api/logs/Emcus"
#define FILE_RANGE_STR     "Start-Non-FF-Packet: "
#define HTTPS_HOST         "us-central1-ip-gsm-ts.cloudfunctions.net"
#define FIRMWARE_PATH      "/api/files/gsm/downloadRange/firmware_AURA.bin"
#define REQ_FILE_SIZE      "/api/files/size?fileName=Emcus_Technology_Solutions_Private_Limited/firmware_AURA.bin"
#define REQ_FILE_RANGE     "/api/files/gsm/downloadRange/Emcus_Technology_Solutions_Private_Limited/firmware_AURA.bin"

#define LAN8742_PHY_ISR 0x1D
/******************************************************************* Variables ******************************************************************/
int i32_sockfd   = CLEAR;
WOLFSSL *ssl     = NULL;
WOLFSSL_CTX *ctx = NULL;

static scp_cnf *pf_scp_config                                                 = (scp_cnf *)INT_FLASH_SSL_CERT_ADDR;
static uint8_t au8_recvline[CHUNK_SIZE + FORMAT_BYTES + MAX_JSON_HEADER_SIZE] = {CLEAR};
static uint8_t ac_print_buff[200]                                             = {CLEAR};

/************************************************************ static Function prototypes *******************************************************/
static int32_t download_ota_file(WOLFSSL *ssl);

static void generate_fire_fault_POST_request(uint8_t *au8_ac_sendline, uint8_t *u8_server_file_path);

static void generate_firmware_chunk_request(uint8_t *au8_ac_sendline, uint8_t *u8_server_file_path,
											uint32_t u32_start_index, uint32_t u32_end_index);

int32_t sent_fire_fault(WOLFSSL *ssl, uint8_t *au8_post_data, uint16_t u16_post_data_len);


// /*************************************************************************************************************************************************
//  * @brief This will get a call back when tehre is stack overflow with the task
//  *        that cause the overflow.
//  * @author Mohammed Shameem
//  * @param xTask      :- Task handle
//  *        pcTaskName :- name of task caused the stack overflow
//  *
//  *************************************************************************************************************************************************/
// //void vApplicationStackOverflowHook(TaskHandle_t xTask, uint8_t *pcTaskName)
// //{
// //	sprintf((char *)ac_print_buff, "\r\n********Stack overflow in task: %s***************\r\n", pcTaskName);
// //	debug_msg((uint8_t *)ac_print_buff);
// //	Error_Handler();
// //}

/*************************************************************************************************************************************************
 * @brief This function will be called when there is request from panel.
 *        This funtion will handle request from the main panel through UART.
 * @author Mohammed Shameem
 * @param u8_service     :- request from panel
 *        au8_tx_data     :- buffer to be sent though eth for log creation
 *        u16_tx_data_len :- length of the buffer to be transmitted.
*************************************************************************************************************************************************/
void ip_task_process(uint8_t u8_service, uint8_t *au8_tx_data, uint16_t u16_tx_data_len)
{
	char c_ret = 0;
	switch (u8_service)
	{
	case ETH_CLOUD_FW_DOWNLOAD_QUEUE:
	{
		/*notifying server that there will be no status update until ota update finishes*/
		mqtt_notify_ota();

		if (vsocketmanagertask() >= 0)
		{
			if (download_ota_file(ssl) != 0)
			{
				debug_msg("\r\nOTA download failed\r\n");
				send_data_frame(0x4003, NULL, NULL, 0u);
			}
			else
			{
				debug_msg("\r\nOTA download success\r\n");
			}
		}
		else
		{
			debug_msg("\r\nerror : internet error");
			lwip_close(i32_sockfd);
		}

		mqtt_clear_buffers();
		/*since OTA update takes more time the mqtt socket will be closed by server mqtt must connect from scratch*/
		u8_mqtt_state = MQTT_INIT;
		break;
	}
	case ETH_CLOUD_POST_QUEUE:
	{	
			debug_msg("\r\n Starting MQTT fire publish\r\n");
			
		
			if(u8_mqtt_state == MQTT_INIT)
			{
				do
				{
				debug_msg("waiting for internet......\r\n");
				c_ret = mqtt_process();
					
				}while(c_ret < 0);
			}
			else
			{
				debug_msg("mqtt is connected publishing data\r\n");
			}
		
			if (mqtt_publish_fire_fault(au8_tx_data, u16_tx_data_len) < 0)
			{
				debug_msg("\r\n MQTT log publish failed\r\n");
			}
			else
			{
				debug_msg("\r\n MQTT log publish success\r\n");
				u8_mqtt_state = MQTT_ROUTINE_OPERATION;
			}
		break;
	}
	}
}

/*************************************************************************************************************************************************
 * @brief This will get a call back when there is HEAP overflow or failed to alloc memory.
 * @author Mohammed Shameem
 * @param xTask      :- Task handle
 *        pcTaskName :- name of task caused the stack overflow
 *
 *************************************************************************************************************************************************/
//void vApplicationMallocFailedHook(void)
//{
//	debug_msg((uint8_t *)"\n***************** Heap overflow detected *************\n\0");
//	// Put breakpoint or LED blink here
//	Error_Handler();
//}

/*************************************************************************************************************************************************
  * @brief  This function will all tcp related operations until its get connected to server.
			Ones the socket is connected with server wolfssl task will be started.
			Until the socket connectes to server this funtion will be trying it in super loop.
  * @param  void
  * @author Mohammed Shameeme
 *************************************************************************************************************************************************/
int32_t vsocketmanagertask(void)
{
	uint8_t u8_socket_Conn_flag = 3;

	int32_t i32_ret = CLEAR;

	ip_addr_t ip_address;

	struct sockaddr_in servaddr;

	debug_msg((uint8_t *)pf_scp_config->https.url);
	
	// collecting server ip through DNS
	i32_ret = netconn_gethostbyname((char *)pf_scp_config->https.url, &ip_address);

	if (NO_INTERNET_FOR_DNS < i32_ret)
	{
		u8_socket_Conn_flag = SET;
		// TODO sent nack to main panel
	}
	sprintf((char *)ac_print_buff, "\r\nIP addr of %s is %s i32_ret %d", pf_scp_config->https.url, ipaddr_ntoa(&ip_address), i32_ret);
	debug_msg((uint8_t *)ac_print_buff);

	// Create socket and connecti!ng server
	while (u8_socket_Conn_flag != 0)
	{
		if ((i32_sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			debug_msg((uint8_t *)"\r\nSocket creation failed\n");
			i32_ret = i32_sockfd;
		}

		sprintf((char *)ac_print_buff, "\r\nsocket created with fd  %d ", i32_sockfd);
		debug_msg((uint8_t *)ac_print_buff);

		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SERVER_PORT);
		servaddr.sin_addr.s_addr = ip_address.addr;

		// Connect to the server
		if ((i32_ret = lwip_connect(i32_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
		{
			u8_socket_Conn_flag--;
			sprintf((char *)ac_print_buff, "\r\nConnection failed %d\n", i32_ret);
			debug_msg((uint8_t *)ac_print_buff);
			lwip_close(i32_sockfd);
		}
		else
		{
			sprintf((char *)ac_print_buff, "\r\nok...  socket connected successsfully  %d ", i32_sockfd);
			debug_msg((uint8_t *)ac_print_buff);
			// connected to server
			u8_socket_Conn_flag = 0;

			// starting https client task
			wolf_ssl_task();
		}
	}
	return i32_ret;
}

/*************************************************************************************************************************************************
  * @brief  tcp_send callback for wolfssl_write function.
  * @param  WOLFSSL *ssl
						uint8_t *buf    - buffer to write during communication.
						int sz       - size to be wrote.
						void *ctx
  * @retval None
 *************************************************************************************************************************************************/
int MySocketSend(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
	int i32_sockfd = *(int *)ctx;
	int i32_ret;
	struct timeval tv;
	fd_set writefds;
	uint8_t u8_TCP_attempts = 0;

	while (u8_TCP_attempts < MAX_SEND_RETRIES)
	{
		// Set timeout (e.g., 5 seconds)
		tv.tv_sec = 15;
		tv.tv_usec = 0;

		FD_ZERO(&writefds);
		FD_SET(i32_sockfd, &writefds);

		// Wait until socket is ready or timeout

		if (0 >= select(i32_sockfd + 1, NULL, &writefds, NULL, &tv))
		{
			debug_msg((uint8_t *)"\r\nSocket send timeout/error");
			i32_ret = WOLFSSL_CBIO_ERR_TIMEOUT;
			u8_TCP_attempts++;
		}
		else
		{
			// Attempt send
			i32_ret = send(i32_sockfd, buf, sz, 0);

			if (0 >= i32_ret)
			{
				// Handle send() errors
				switch (errno)
				{
				case ECONNRESET:
				case ENOTCONN:
				case EPIPE:
					debug_msg((uint8_t *)"\r\nConnection lost");
					u8_TCP_attempts = MAX_SEND_RETRIES;
					i32_ret = WOLFSSL_CBIO_ERR_CONN_CLOSE;
				case EWOULDBLOCK:
					u8_TCP_attempts++;
					debug_msg((uint8_t *)"\r\nSend would block, retrying...");
					osDelay(RETRY_DELAY_MS);
					continue;
				default:
					sprintf((char *)ac_print_buff, "\r\nSend error: %d", errno);
					debug_msg((uint8_t *)ac_print_buff);
					u8_TCP_attempts = MAX_SEND_RETRIES;
					i32_ret = WOLFSSL_CBIO_ERR_GENERAL;
				}
			}
			else
			{
				u8_TCP_attempts = MAX_SEND_RETRIES;
			}
		}
	}

	if (i32_ret < 0)
	{
		// TODO sent nack to main panel
		sprintf((char *)ac_print_buff, "\r\nsend returns with %d", i32_ret);
		debug_msg((uint8_t *)ac_print_buff);
	}
	return i32_ret;
}
/*************************************************************************************************************************************************
  * @brief  tcp_recv callback for wolfssl_read function.
  * @param  WOLFSSL *ssl
						uint8_t *buf    - buffer to read during communication.
						int sz       - size to be read.
						void *ctx
  * @retval None
 *************************************************************************************************************************************************/

int MySocketRecv(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
	int i32_sockfd = *(int *)ctx;
	int i32_ret;
	struct timeval tv;
	fd_set readfds;
	int u8_TCP_attempts = 0;

	while (u8_TCP_attempts < MAX_RECV_RETRIES)
	{
		// Setup timeout for select
		tv.tv_sec = 60;
		tv.tv_usec = 0;

		FD_ZERO(&readfds);
		FD_SET(i32_sockfd, &readfds);

		if (0 >= select(i32_sockfd + 1, &readfds, NULL, NULL, &tv))
		{
			sprintf((char *)ac_print_buff, "\r\nselect TCP recv failed: %d (errno=%d)", i32_ret, errno);
			debug_msg((uint8_t *)ac_print_buff);
			i32_ret = ((i32_ret == 0) ? WOLFSSL_CBIO_ERR_TIMEOUT : WOLFSSL_CBIO_ERR_GENERAL);
			u8_TCP_attempts++;
		}
		else
		{

			i32_ret = recv(i32_sockfd, buf, sz, 0);
			if (0 == i32_ret)
			{
				debug_msg((uint8_t *)"\r\nTCP connection closed by peer");
				i32_ret = WOLFSSL_CBIO_ERR_CONN_CLOSE;
				u8_TCP_attempts = MAX_RECV_RETRIES;
			}
			else if (0 > i32_ret)
			{
				switch (errno)
				{
				case EWOULDBLOCK:
					u8_TCP_attempts++;
					debug_msg((uint8_t *)"\r\nrecv would block, retrying...");
					osDelay(RETRY_DELAY_MS);
					break;
				case ECONNRESET:
					debug_msg((uint8_t *)"\r\nrecv failed: ECONNRESET");
					i32_ret = WOLFSSL_CBIO_ERR_CONN_RST;
					u8_TCP_attempts = MAX_RECV_RETRIES;
				case ENOTCONN:
					debug_msg((uint8_t *)"\r\nrecv failed: ENOTCONN");
					i32_ret = WOLFSSL_CBIO_ERR_CONN_CLOSE;
					u8_TCP_attempts = MAX_RECV_RETRIES;
				case ETIMEDOUT:
					debug_msg((uint8_t *)"\r\nrecv failed: ETIMEDOUT");
					i32_ret = WOLFSSL_CBIO_ERR_TIMEOUT;
					u8_TCP_attempts = MAX_RECV_RETRIES;
				case EINTR:
					debug_msg((uint8_t *)"\r\nrecv interrupted by signal, retrying...");
					u8_TCP_attempts++;
					break;
				default:
					sprintf((char *)ac_print_buff, "\r\nrecv failed: errno=%d", errno);
					debug_msg((uint8_t *)ac_print_buff);
					i32_ret = WOLFSSL_CBIO_ERR_GENERAL;
					u8_TCP_attempts = MAX_RECV_RETRIES;
				}
			}
			else
			{
				u8_TCP_attempts = MAX_RECV_RETRIES;
			}
		}
	}

	if (i32_ret < 0)
	{
		// TODO sent nack to main panel
		sprintf((char *)ac_print_buff, "\r\nrecv returns with %d", i32_ret);
		debug_msg((uint8_t *)ac_print_buff);
	}
	return i32_ret;
}

/*************************************************************************************************************************************************
  * @brief  print debug message which will be linked to wolf ssl library for debugging.
  * @param  WOLFSSL *ssl
						const int i32_logLevel     - Loglevel to be printed
						const uint8_t* ai8_logMessage - Log message to be printed
						void *ctx
  * @retval None
 *************************************************************************************************************************************************/
#if WOLFSS_DEBUG
/*custom funtion link with wolfssl to print debug*/
void wolfssl_print(const int i32_logLevel, const char *const ai8_logMessage)
{
	sprintf((char *)ac_print_buff, "\r\nWolfSSL Log [%d]: %s\n", i32_logLevel, ai8_logMessage);
	debug_msg((uint8_t *)ac_print_buff);
	// debug_println(ac_print_buff);
	osDelay(5);
}
#endif

/*************************************************************************************************************************************************
  * @brief  This function will generate JSON formatted get request as per parameter passed.
  * @param  uint8_t *au8_ac_sendline      - Generated file will be copied to this buffer.
						uint8_t *u8_server_file_path - This will be having the Api to be requested.
  * @retval None
 *************************************************************************************************************************************************/
void generate_GET_request(uint8_t *au8_ac_sendline, uint8_t *u8_server_file_path)
{
	// loading link of FW update binary into GET request
	sprintf((char *)au8_ac_sendline,
			"GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"Connection: Keep-Alive\r\n\r\n",
			u8_server_file_path, pf_scp_config->https.url);
}

/*************************************************************************************************************************************************
  * @brief  This function will generate JSON formatted range get request as per
			-parameter passed.
  * @param  uint8_t *au8_ac_sendline      - Generated file will be copied to this buffer.
						uint8_t *u8_server_file_path - This will be having the Api to be requested.
			uint32_t u32_start_index   - This parameter will be having start index
									 -of the code.
			uint32_t u32_end_index     - This will be representing the end index
									 -of the requesting chucnk.
  * @retval None
 *************************************************************************************************************************************************/
static void generate_firmware_chunk_request(uint8_t *au8_ac_sendline, uint8_t *u8_server_file_path, uint32_t u32_start_index, uint32_t u32_end_index)
{
	// loading link of FW update binary into GET request
	sprintf((char *)au8_ac_sendline,
			"GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"range: bytes=%d-%d\r\n"
			"Content-Type: application/octet-stream\r\n"
			"Connection: Keep-Alive\r\n\r\n",
			u8_server_file_path, pf_scp_config->https.url, u32_start_index, u32_end_index);
}

/*************************************************************************************************************************************************
  * @brief  This function will generate JSON formatted POST request as per
			-parameter passed.
			Its usually used for senting fire or fault for logs generation.

  * @param  uint8_t *au8_ac_sendline      - Generated request will be copied to this buffer.
						uint8_t *u8_server_file_path - This will be having the Api to be requested.
			uint8_t *post_body        - This parameter will .
			uint32_t u32_end_index     - This will be representing the end index of
									 -the requesting chucnk.
  * @retval None
 *************************************************************************************************************************************************/
static void generate_fire_fault_POST_request(uint8_t *au8_ac_sendline, uint8_t *u8_server_file_path)
{

	sprintf((char *)au8_ac_sendline,
			"POST %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: application/octet-stream\r\n"
			"Connection: Keep-Alive\r\n\r\n",
			u8_server_file_path, pf_scp_config->https.url, (37 + 14));
}

/*************************************************************************************************************************************************
 * @brief  This function is used to sent the fire-fault notification to server .
 * @param  WOLFSSL* ssl      - for communication in same code.
 *         au8_post_data     - buffer with the fire/fault log structure from panel
 *         u16_post_data_len - length of received array
 * @retval None
 *************************************************************************************************************************************************/
int32_t sent_fire_fault(WOLFSSL *ssl, uint8_t *au8_post_data, uint16_t u16_post_data_len)
{
	debug_msg((uint8_t *)"\r\nSENT POST event log started");

	int16_t i32_ret = CLEAR;

	int i32_bytes_received = CLEAR;

	uint8_t au8_ac_sendline[MAXLINE];

	// generate JSON header
	generate_fire_fault_POST_request(au8_ac_sendline, POST_API_LOG);

	// writing header with Payload(log structure)
	if ((i32_ret = wolfSSL_write(ssl, (char *)au8_ac_sendline, strlen((char *)au8_ac_sendline))) !=
		strlen((char *)au8_ac_sendline))
	{
		sprintf((char *)ac_print_buff, "\r\nwolfSSL_write failed %d", i32_ret);
		debug_msg((uint8_t *)ac_print_buff);
	}
	else
	{
		sprintf((char *)ac_print_buff, "\r\n%s", au8_ac_sendline);
		debug_println(ac_print_buff);
		debug_msg((uint8_t *)"\r\nwolfSSL_write success");
	}

	// writing Payload(log structure from panel)
	if ((i32_ret = wolfSSL_write(ssl, (const void *)au8_post_data, u16_post_data_len)) !=
		u16_post_data_len)
	{
		sprintf((char *)ac_print_buff, "\r\nwolfSSL_write failed %d", i32_ret);
		debug_msg((uint8_t *)ac_print_buff);
	}
	else
	{
		sprintf((char *)ac_print_buff, "\r\n%s", au8_ac_sendline);
		debug_println(ac_print_buff);
		debug_msg((uint8_t *)"\r\nwolfSSL_write success");
	}

	// Read response
	if ((i32_bytes_received = wolfSSL_read(ssl, au8_recvline, 4096)) <= 0)
	{
		debug_msg("\r\nwolfssl rad failed\r\n");
		i32_ret = i32_bytes_received;
	}
	else
	{
		au8_recvline[i32_bytes_received] = '\0';
		debug_println((uint8_t *)au8_recvline);
	}

	// clearig wolf ssl buffers
	wolfSSL_free(ssl);	   /* Free WOLFSSL object */
	wolfSSL_CTX_free(ctx); /* Free WOLFSSL_CTX object */
	wolfSSL_Cleanup();	   /* Free wolfSSL */
	lwip_close(i32_sockfd);

	return i32_ret;
}

/*************************************************************************************************************************************************
  * @brief  This function will handle read and write operation with https server
			Including Download complete OTA file and writing to master panel
			-as per the event set with respect to ack received.
  * @param  WOLFSSL *ssl - structure containing all the information
			-regarding ssl operation
  * @retval None
 *************************************************************************************************************************************************/
int32_t download_ota_file(WOLFSSL *ssl)
{
	uint8_t crc_retry_count          = CLEAR;
	uint8_t au8_ac_sendline[MAXLINE] = {CLEAR};

	uint16_t u16_print_size     = CLEAR;
	uint16_t u16_received_crc   = CLEAR;
	uint16_t u16_calculated_crc = CLEAR;

	int32_t i32_ret            = CLEAR;
	int32_t i32_bytes_received = CLEAR;

	uint32_t u32_file_size            = CLEAR;
	uint32_t u32_end_index            = CLEAR;
	uint32_t u32_start_index          = CLEAR;
	uint32_t u32_packet_count         = CLEAR;
	uint32_t u32_num_of_bytes_pending = CLEAR;

	char *pc_payload_ptr;

#if MODE_CHUNK
	/************************  fetching fie_size*********************************/

	// generate JSON header to get OTA file size
	generate_GET_request((uint8_t *)au8_ac_sendline, REQ_FILE_SIZE);

	// writing GET request to server
	if ((i32_ret = wolfSSL_write(ssl, (uint8_t *)au8_ac_sendline, strlen((char *)au8_ac_sendline))) !=
		                                                                          strlen((char *)au8_ac_sendline))
	{
		sprintf((char *)ac_print_buff, "\r\nwolfSSL_write failed %d", i32_ret);
		debug_msg((uint8_t *)ac_print_buff);
		i32_ret = ETH_FW_DOWNLOAD_FAILED;
	}
	else
	{
		debug_msg((uint8_t *)au8_ac_sendline);
		debug_msg((uint8_t *)"\r\nwolfSSL_write success");
	}

	//reading response json with ota file size
	if ((i32_bytes_received = wolfSSL_read(ssl, au8_recvline, 4096)) <= 0)
	{
		sprintf((char *)ac_print_buff, "\r\nwolfSSL_read failed %d", i32_bytes_received);
		i32_ret = ETH_FW_DOWNLOAD_FAILED;
		u32_num_of_bytes_pending = 0;
	}
	else
	{
		au8_recvline[i32_bytes_received] = '\0';
		debug_msg((uint8_t *)au8_recvline);

		//extracting content length from json header
		u32_file_size = get_content_length((uint8_t *)au8_recvline, CONTENT_LENGTH_STR);
		u32_num_of_bytes_pending = u32_file_size;
	}

	/********************************** request and download chunk ********************************/
	while (u32_num_of_bytes_pending != 0)
	{
#if TEST_ETH_PANEL_UPLOAD
		// waiting for ACK from panel
		if ((ETH_CLOUD_FW_UPLOAD_FLAG == xEventGroupWaitBits(event_eth_rx, ETH_CLOUD_FW_UPLOAD_FLAG, pdTRUE, pdFALSE, 5000u)))
		{
#endif // TEST_ETH_PANEL_UPLOAD
	   // updating variables to request for next chunk
			crc_retry_count = CLEAR;
			u32_start_index = u32_end_index;
			u32_end_index += CHUNK_SIZE;
			// handling for last packet
			if (u32_file_size < u32_end_index)
			{
				u32_end_index -= CHUNK_SIZE;
				u32_end_index += (u32_file_size - u32_end_index);
			}

			// Generating chunk request
			generate_firmware_chunk_request((uint8_t *)au8_ac_sendline, REQ_FILE_RANGE, u32_start_index, (u32_end_index - 1));

			do
			{
				crc_retry_count++;

				// writing request for CHUNK size
				if ((i32_ret = wolfSSL_write(ssl, (uint8_t *)au8_ac_sendline, strlen((char *)au8_ac_sendline))) !=
					                                                                   strlen((char *)au8_ac_sendline))
				{
					sprintf((char *)ac_print_buff, "\r\nwolfSSL_write failed %d", i32_ret);
					debug_msg((uint8_t *)ac_print_buff);
					i32_ret = ETH_FW_DOWNLOAD_FAILED;
					u32_num_of_bytes_pending = 0;
				}
				else
				{
					debug_msg((uint8_t *)"\r\nwolfSSL_write success");
					debug_msg(au8_ac_sendline);
				}

				// reading Response JSON HEADER  with complete data in it
				if ((i32_bytes_received = wolfSSL_read(ssl, au8_recvline, sizeof(au8_recvline))) <= 0)
				{
					sprintf((char *)ac_print_buff, "\r\nwolfSSL_raed failed %d", i32_bytes_received);
					i32_ret = ETH_FW_DOWNLOAD_FAILED;
					u32_num_of_bytes_pending = 0;
				}
				//handle for skipp while 0xFF packet skiping
				else
				{
                    //pointing to json payload  
					pc_payload_ptr = strstr((char *)au8_recvline, "\r\n\r\n");
					pc_payload_ptr += strlen("\r\n\r\n");
					
					// fetching content length from JSON HEADER
					// handling for skipping chunk with all 0XFF
					au8_recvline[i32_bytes_received] = '\0';
					if ((i32_ret = get_content_length(au8_recvline, FILE_RANGE_STR)) != 0)
					{
						// printing header with content length
						debug_msg((uint8_t *)au8_recvline);

						// updating variables after first read
						u32_start_index = i32_ret;
						u32_end_index = i32_ret + CHUNK_SIZE;
						if (u32_file_size < u32_end_index)
						{
							u32_end_index -= CHUNK_SIZE;
							u32_end_index += (u32_file_size - u32_end_index);
						}
					}
				}

				if (i32_ret != ETH_FW_DOWNLOAD_FAILED)
				{
					// handling for last packet
					if (((u32_end_index - u32_start_index) + FORMAT_BYTES) != CHUNK_SIZE + FORMAT_BYTES) 
					{
						u32_num_of_bytes_pending = 0;
						u16_print_size = (u32_end_index - u32_start_index) + FORMAT_BYTES;
					}
					else
					{
						u16_print_size = CHUNK_SIZE + FORMAT_BYTES; // EXPECTED_DATA_SIZE;
					}

					// calculate crc from received chunk
					u16_calculated_crc = calculate_crc((uint32_t *)(pc_payload_ptr),
													   ((uint32_t)u16_print_size - 4));

					// segregate crc from received chunk last 3rd and 4th bits
					u16_received_crc = ((uint16_t)(pc_payload_ptr)[u16_print_size - 3] << 8) | (pc_payload_ptr)[u16_print_size - 4];

					sprintf((char *)ac_print_buff, "CRC received %04X  ======  CRC calculated :%04X \r\n bytes_downloaded : %d \r\n", u16_received_crc, u16_calculated_crc, u32_end_index);
					debug_msg(ac_print_buff);
				}
				else
				{
					debug_msg("\r\n internet error \r\n");
					crc_retry_count = 4;
				}
			} while ((u16_calculated_crc != u16_received_crc) && (crc_retry_count <= 3));

			if (i32_ret != ETH_FW_DOWNLOAD_FAILED)
			{
				// incase of crc_mismatch
				if (crc_retry_count >= 3)
				{
					debug_msg((uint8_t *)"\r\n***************CRC mismatch data corrupted***************\r\n");
					u32_num_of_bytes_pending = 0;
					i32_ret = ETH_FW_DOWNLOAD_FAILED;
					
				}

				u32_packet_count = ((pc_payload_ptr)[7] << 8) | (pc_payload_ptr)[6];
				sprintf((char *)ac_print_buff, "Packet count = %d\r\n", u32_packet_count);
				debug_msg(ac_print_buff);
			}

#if TEST_ETH_PANEL_UPLOAD
				// below fn sends bin data along with start and end packets.
				if ((u32_end_index - u32_start_index) == 17)
				{
					debug_msg((uint8_t *)"transmitted Last packet\r\n");
					mcu_comm_transmit((uint8_t *)(pc_payload_ptr), ((uint32_t)(u32_end_index - u32_start_index) + 8u + 4u));
				}
				else
				{
					mcu_comm_transmit((uint8_t *)(pc_payload_ptr), ((uint32_t)(u32_end_index - u32_start_index) + 8u + 4u));
						u32_num_of_bytes_pending -= ((uint32_t)(u32_end_index - u32_start_index) + 8u + 4u);
				}
				
			

				sprintf((char *)ac_print_buff, "\r\ntransmitted packet no: %d \n with size : %d ", u32_packet_count, (u32_end_index - u32_start_index));
				debug_msg((uint8_t *)ac_print_buff);
			sprintf((char *)ac_print_buff, "\r\nnum_of_bytes_pending : %d ", u32_num_of_bytes_pending);
			debug_msg(ac_print_buff);
		}
		else
		{
			debug_msg("\r\nmaster ack timeout!!!!!");
			u32_num_of_bytes_pending = 0;
			i32_ret = ETH_FW_DOWNLOAD_FAILED;
		}
		

#endif // TEST_ETH_PANEL_UPLOAD
	}

	if (0 == i32_ret)
	{
		debug_msg("\r\nfinished reading bin");
	}
	else
	{
		debug_msg("\r\nupdate failed");
	}
#else

#endif
	// clearig wolf ssl buffers
	wolfSSL_free(ssl);	   /* Free WOLFSSL object */
	wolfSSL_CTX_free(ctx); /* Free WOLFSSL_CTX object */
	wolfSSL_Cleanup();	   /* Free wolfSSL */
	lwip_close(i32_sockfd);

	return i32_ret;
}


/*************************************************************************************************************************************************
  * @brief  This function will handle all initialiation to connection
			-operations for wolfssl .
  * @param  void *argument - not used
  * @retval None
 *************************************************************************************************************************************************/
void wolf_ssl_task(void) //*argument)
{
	debug_msg((uint8_t *)"stated wolf_ssl task  waiting to connect with socket");

	int i32_ret = RESET;

// enabling init
#if WOLFSS_DEBUG
	wolfSSL_Debugging_ON();
	wolfSSL_SetLoggingCb(wolfssl_print);
#endif

	// wold ssl init
	wolfSSL_Init();

	// creating new ctx for tls1.2
	if ((ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method())) == NULL)
	{
		sprintf((char *)ac_print_buff, "wolfSSL_CTX_ new\n");
		debug_msg((uint8_t *)ac_print_buff);
	}

	// setting secrity level 
	wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_PEER, NULL);

		sprintf((char *)ac_print_buff, "\n\ncertificate length %d\n \n", strlen((char *)pf_scp_config->https.cert));
	debug_msg((uint8_t *)ac_print_buff);
	debug_msg((uint8_t *)pf_scp_config->https.cert);
	
	
	
	// Load CA certificate from memory  wolfSSL_CTX_load_verify_buffer
	if ((i32_ret = wolfSSL_CTX_load_verify_buffer(ctx, (const uint8_t *)pf_scp_config->https.cert, strlen((char *)pf_scp_config->https.cert), CTC_FILETYPE_PEM)) != SSL_SUCCESS)
	{
		sprintf((char *)ac_print_buff, "wolfSSL_CTX_new error : %d\n", i32_ret);
		debug_msg((uint8_t *)ac_print_buff);
		vTaskDelete(NULL);
	}
	else
	{
		sprintf((char *)ac_print_buff, "\r\nwolfSSL certificate load SSL_SUCCESS ret : %d\n", i32_ret);
		debug_msg((uint8_t *)ac_print_buff);
	}

//	// Set WolfSSL callbacks before connecting
//	wolfSSL_SetIORecv(ctx, MySocketRecv);
//	wolfSSL_SetIOSend(ctx, MySocketSend);

	// part of ssl init
	if ((ssl = wolfSSL_new(ctx)) == NULL)
	{
		debug_msg((uint8_t *)"wolfSSL_new error.\n");
	}

	// linking socket with ssl
	wolfSSL_set_fd(ssl, i32_sockfd);

	wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME, pf_scp_config->https.url, strlen((char *)pf_scp_config->https.url));

	debug_msg((uint8_t *)"wolfSSL Ready.\n");
}

/*************************************************************************************************************************************************
 * @brief  This function will get call back when dhcp received.
 * @param  nertif - variable containing all ip address details.
 * @retval None
 *************************************************************************************************************************************************/
void netif_status_callback(struct netif *netif)
{
	uint8_t buff[50];
	if (netif_is_up(netif))
	{
		sprintf((char *)buff, "Network interface is UP. IP: %s\n", ipaddr_ntoa(&netif->ip_addr));
		debug_msg(buff);
	}
	else
	{
		debug_msg("Network interface is DOWN.\n");
	}
}


