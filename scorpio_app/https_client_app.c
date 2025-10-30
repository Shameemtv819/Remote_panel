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
#include "mqtt_client.h"
#include "mqtt_socket.h"


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
#define POST_API_LOG       "/api/logs/gemini"
#define FILE_RANGE_STR     "Start-Non-FF-Packet: "
#define HTTPS_HOST         "us-central1-ip-gsm-ts.cloudfunctions.net"
#define FIRMWARE_PATH      "/api/files/gsm/downloadRange/firmware_AURA.bin"
#define REQ_FILE_SIZE      "/api/files/size?fileName=Gemini/firmware_AURA.bin"
#define RESET_CURSOR       "/api/files/reset-cursor?fileName=firmware_AURA.bin"
#define REQ_FILE_RANGE     "/api/files/gsm/downloadRange/gemini/firmware_AURA.bin"

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

void wolfssl_print(const int i32_logLevel, const char *const ai8_logMessage);



typedef struct
{
    uint16_t u16_event_id;
    uint16_t log_num;
    char     u8_device_text[16];
    char     u8_zone_text[16];
    uint8_t  u8_zone_number;
    uint8_t  u8_node_address;
    uint8_t  u8_device_address;
    uint8_t  u8_device_type;
    uint8_t  u8_device_sub_type;
    uint8_t  u8_date;
    uint8_t  u8_month;
    uint16_t u8_year;
    uint8_t  u8_hours;
    uint8_t  u8_minutes;
    uint8_t  u8_seconds;
    uint8_t  u8_logbitoffset;
    uint8_t  u8_serialNumber;
    uint16_t u16_crc;
} event_log_t;

typedef struct
{
    char message[64];
    char key[32];
} websocket_msg_t;


//#define MQTT_HOST       "62aae2fae7954f638553789f07baa0da.s1.eu.hivemq.cloud"
//#define MQTT_PORT       8883
//#define MQTT_USERNAME   "shameem"
//#define MQTT_PASSWORD   "Shameem@1343"
//#define MQTT_TOPIC      "stm32/test"
//#define MQTT_MESSAGE    "Hello from STM32!"

//#define MQTT_HOST       "io.adafruit.com"
//#define MQTT_PORT       1883
//#define MQTT_USERNAME   "Vetri29"
//#define MQTT_PASSWORD   "aio_MLkf04Qp0HOHCeWT53ePViDelhO1"
//#define MQTT_TOPIC      "Vetri29/feeds/gateway"
//#define MQTT_MESSAGE    "Hello from STM32!"

#define MQTT_HOST       "broker.hivemq.com"
#define MQTT_PORT       1883
#define MQTT_TOPIC      "stm32/test"
#define MQTT_MESSAGE    "Hello from STM32!"

#define MQTT_CMD_TIMEOUT_MS    5000U

// Buffers
static unsigned char txtx_buf[0x4000];
static unsigned char rxrx_buf[0x4000];
/*************************************************************************************************************************************************
  * @brief  This function will all tcp related operations until its get connected to server.
			Ones the socket is connected with server wolfssl task will be started.
			Until the socket connectes to server this funtion will be trying it in super loop.
  * @param  void
  * @author Mohammed Shameeme
 *************************************************************************************************************************************************/
uint8_t print_buff[150];

static int mqtt_tls_cb(MqttClient* client)
{
//	WOLFSSL_CTX* ctx = (WOLFSSL_CTX*)client->tls.ctx;
//    WOLFSSL* ssl = (WOLFSSL*)client->tls.ssl;
//	
//		// creating new ctx for tls1.2
//	if ((ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method())) == NULL)
//	{
//		sprintf((char *)ac_print_buff, "wolfSSL_CTX_ new\n");
//		debug_msg((uint8_t *)ac_print_buff);
//	}

//    wolfSSL_set_verify(ssl, SSL_VERIFY_NONE, NULL);

//    /* Enable SNI */
//    wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME, MQTT_HOST, 
//	                                            strlen(MQTT_HOST));

    /* Load CA cert */
    //wolfSSL_CTX_load_verify_buffer(ctx, ca_cert, sizeof(ca_cert), WOLFSSL_FILETYPE_PEM);
     debug_msg("\r\nTLS callback called.\n");
	
    return WOLFSSL_SUCCESS;
}

static int Msocket = -1;
static MqttClient mClient;
static MqttNet net;

typedef struct {
    int socket_fd;
} MqttLwipContext;

/* Connect function */
static int mqtt_lwip_connect(void *context, const char *host, word16 port, int timeout_ms)
{
    MqttLwipContext *ctx = (MqttLwipContext *)context;
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int rc;

#if MQTT_NET_DEBUG
    printf("MQTT: Connecting to %s:%d\n", host, port);
#endif

    /* DNS lookup */
    hp = gethostbyname(host);
    if (hp == NULL) {
#if MQTT_NET_DEBUG
        printf("MQTT: DNS failed\n");
#endif
        return MQTT_CODE_ERROR_NETWORK;
    }

    /* Create socket */
    ctx->socket_fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->socket_fd < 0)
        return MQTT_CODE_ERROR_NETWORK;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);
    memcpy(&server_addr.sin_addr, hp->h_addr, hp->h_length);

    /* Set send/receive timeout (optional) */
    int timeout = timeout_ms;
    lwip_setsockopt(ctx->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    lwip_setsockopt(ctx->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    /* Connect */
    rc = lwip_connect(ctx->socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rc < 0) {
#if MQTT_NET_DEBUG
        printf("MQTT: connect failed (%d)\n", rc);
#endif
			debug_msg("\r\nMQTT: connect failed (%d)\n" );
        lwip_close(ctx->socket_fd);
        ctx->socket_fd = -1;
        return MQTT_CODE_ERROR_NETWORK;
    }

#if MQTT_NET_DEBUG
    printf("MQTT: Connected\n");
#endif
		debug_msg("\r\nMQTT: Connected\n" );
    return MQTT_CODE_SUCCESS;
}

/* Read function */
static int mqtt_lwip_read(void *context, byte *buf, int len, int timeout_ms)
{
    MqttLwipContext *ctx = (MqttLwipContext *)context;
    int recvd;

    lwip_setsockopt(ctx->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));

    recvd = lwip_recv(ctx->socket_fd, buf, len, 0);
    if (recvd < 0)
		{
        return MQTT_CODE_ERROR_TIMEOUT;
			debug_msg("\r\nrecv failed\n" );
		}
    else if (recvd == 0)
		{
			debug_msg("\r\nrecv failed\n" );
        return MQTT_CODE_ERROR_NETWORK; /* Connection closed */
		}

    return recvd;
}

/* Write function */
static int mqtt_lwip_write(void *context, const byte *buf, int len, int timeout_ms)
{
    MqttLwipContext *ctx = (MqttLwipContext *)context;
    int sent;

    lwip_setsockopt(ctx->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout_ms, sizeof(timeout_ms));

    sent = lwip_send(ctx->socket_fd, buf, len, 0);
    if (sent < 0)
		{
			debug_msg("\r\nsent failed\n" );
        return MQTT_CODE_ERROR_TIMEOUT;
		}

    return sent;
}

/* Disconnect function */
static int mqtt_lwip_disconnect(void *context)
{
    MqttLwipContext *ctx = (MqttLwipContext *)context;

    if (ctx->socket_fd >= 0) {
        lwip_close(ctx->socket_fd);
        ctx->socket_fd = -1;
#if MQTT_NET_DEBUG
        printf("MQTT: Disconnected\n");
#endif
    }
    return MQTT_CODE_SUCCESS;
}

/* Initialize network structure */
int MqttNet_Init(MqttNet *net)
{
   

    static MqttLwipContext lwipCtx;  /* Can be static or allocated dynamically */
    lwipCtx.socket_fd = -1;

    net->context     = &lwipCtx;
    net->connect     = mqtt_lwip_connect;
    net->read        = mqtt_lwip_read;
    net->write       = mqtt_lwip_write;
    net->disconnect  = mqtt_lwip_disconnect;

    return MQTT_CODE_SUCCESS;
}

int message_callback (struct _MqttClient *client, MqttMessage *message, byte msg_new, byte msg_done)
{
	debug_msg("\r\nrecv message from brocker\n" );
	
	char print_buff[256];

    switch (message->type)
    {
        case MQTT_PACKET_TYPE_PUBLISH:
        {
            MqttPublish *publish = (MqttPublish*)message;

            sprintf(print_buff, "Received PUBLISH topic: %s\n", publish->topic_name);
            debug_msg(print_buff);

            sprintf(print_buff, "Payload: %.*s\n", publish->total_len, publish->buffer);
            debug_msg(print_buff);

            break;
        }

        case MQTT_PACKET_TYPE_PING_RESP:
            debug_msg("PING response received.\n");
            break;

        case MQTT_PACKET_TYPE_DISCONNECT:
            debug_msg("Server sent DISCONNECT.\n");
            break;

        default:
            sprintf(print_buff, "Unhandled message type: %d\n", message->type);
            debug_msg(print_buff);
            break;
    }

    return MQTT_CODE_SUCCESS;   // always return success unless you want to abort
	
}

//byte tx_buf[1024], rx_buf[1024];
int32_t Msocketmanagertask(void)
{
	MqttClient client;
    
    int rc;

MqttNet_Init(&net);
	
	client.msg_cb = message_callback;
	
	
	wolfSSL_Debugging_ON();
	wolfSSL_SetLoggingCb(wolfssl_print);
	
    /* Initialize network and client */
    rc = MqttClient_Init(&client, &net, message_callback, txtx_buf, sizeof(txtx_buf),
                    rxrx_buf, sizeof(rxrx_buf), 60000);
		
		 if (rc != MQTT_CODE_SUCCESS) {
         sprintf(print_buff,"\r\nMqttClient_Init error: %d\n",rc);
			debug_msg(print_buff);
        
    }

    rc = MqttClient_NetConnect(&client, MQTT_HOST, MQTT_PORT, 60000, 0, mqtt_tls_cb);
    if (rc != MQTT_CODE_SUCCESS) 
		{
      sprintf(print_buff,"\r\nMqttClient_Init error: %d\n",rc);
			debug_msg(print_buff);
        
    }

    MqttConnect connect;
    memset(&connect, 0, sizeof(connect));
    connect.keep_alive_sec = 60;
    connect.clean_session = 1;
//    connect.username = MQTT_USERNAME;
//    connect.password = MQTT_PASSWORD;
    connect.client_id = "stm134330";
		//connect.client_id_len = strlen("stm32client123");
	

    rc = MqttClient_Connect(&client, &connect);
    if (rc == MQTT_CODE_SUCCESS) 
		{
       debug_msg("MQTT CONNECTED!\n");
    } 
		else 
		{
			sprintf(print_buff,"\r\nMQTT connect error: %d\n",rc);
			debug_msg(print_buff);
    }

		// Step 4: Subscribe to topics
    MqttSubscribe mqtt_sub;
    MqttTopic topics[1];
    memset(&mqtt_sub, 0, sizeof(mqtt_sub));
    
    mqtt_sub.packet_id = 1;
    mqtt_sub.topic_count = 1;
    mqtt_sub.topics = topics;
    topics[0].topic_filter = MQTT_TOPIC;
    topics[0].qos = MQTT_QOS_0;
    
    rc = MqttClient_Subscribe(&client, &mqtt_sub);
    if (rc != MQTT_CODE_SUCCESS) {
        sprintf(print_buff,"Subscribe failed: %d\n", rc);
			 debug_msg(print_buff);
    } else {
        debug_msg("Subscribed to stm32/test\n");
    }
		
		
//    /* Publish example */
//    MqttPublish publish;
//    memset(&publish, 0, sizeof(publish));
//    publish.qos = 0;
//		publish.packet_id = 2;
//    publish.topic_name = MQTT_TOPIC;
//    publish.buffer = (byte*)MQTT_MESSAGE;
//    publish.total_len = (word16)strlen(MQTT_MESSAGE);

//    rc = MqttClient_Publish(&client, &publish);
//    if (rc == MQTT_CODE_SUCCESS)
//			{
//				sprintf(print_buff,"Published: %s \n Topic : %s", MQTT_MESSAGE,MQTT_TOPIC);
//    debug_msg(print_buff);
//    }
//		else 
//		{
//			sprintf(print_buff,"Publish failed: %d\n", rc);
//    debug_msg(print_buff);
//    }
//		
//		 /* Publish example */
//    publish;
//    memset(&publish, 0, sizeof(publish));
//    publish.qos = 2;
//		publish.packet_id =3;
//    publish.topic_name = MQTT_TOPIC;
//    publish.buffer = (byte*)"hai hivemq";
//    publish.total_len = (word16)strlen("hai hivemq");

//    rc = MqttClient_Publish(&client, &publish);
//    if (rc == MQTT_CODE_SUCCESS)
//			{
//				sprintf(print_buff,"Published: %s \n Topic : %s", MQTT_MESSAGE,MQTT_TOPIC);
//    debug_msg(print_buff);
//				
//				  /* ? Add this line for QoS > 0 */
//    }
//		else 
//		{
//			sprintf(print_buff,"Publish failed: %d\n", rc);
//    debug_msg(print_buff);
//    }
//		
//		 /* Publish example */
//    //MqttPublish publish;
//    memset(&publish, 0, sizeof(publish));
//    publish.qos = 0;
//		publish.packet_id =4;
//    publish.topic_name = MQTT_TOPIC;
//    publish.buffer = (byte*)"hai hivemq";
//    publish.total_len = (word16)strlen("hai hivemq");

//    rc = MqttClient_Publish(&client, &publish);
//    if (rc == MQTT_CODE_SUCCESS)
//			{
//				sprintf(print_buff,"Published: %s \n Topic : %s", "hai hivemq",MQTT_TOPIC);
//    debug_msg(print_buff);
//    }
//		else 
//		{
//			sprintf(print_buff,"Publish failed: %d\n", rc);
//    debug_msg(print_buff);
//    }
//		 /* Publish example */
//   // MqttPublish publish;
//    memset(&publish, 0, sizeof(publish));
//    publish.qos = 0;
//		publish.packet_id =5;
//    publish.topic_name = MQTT_TOPIC;
//    publish.buffer = (byte*)"from emcus";
//    publish.total_len = (word16)strlen("from emcus");

//    rc = MqttClient_Publish(&client, &publish);
//    if (rc == MQTT_CODE_SUCCESS)
//			{
//				sprintf(print_buff,"Published: %s \n Topic : %s", "from emcus",MQTT_TOPIC);
//    debug_msg(print_buff);
//    }
//		else 
//		{
//			sprintf(print_buff,"Publish failed: %d\n", rc);
//    debug_msg(print_buff);
//    }


//		
//		
//		
    
	
		
    // Step 5: Message processing loop
    debug_msg("Waiting for messages...\n");
    while (1) {
			
		 rc = MqttClient_WaitMessage(&client, 5000); // Wait up to 5 seconds

    if (rc == MQTT_CODE_SUCCESS)
    {
        // Incoming PUBLISH message will trigger mqtt_message_cb automatically
    }
    else if (rc == MQTT_CODE_CONTINUE)
    {
        // No message yet, timeout expired
        debug_msg(".");
        continue;
    }
    else
    {
        sprintf(print_buff, "WaitMessage error: %d\n", rc);
        debug_msg(print_buff);
        break;  // optionally reconnect
    }
//        rc = MqttClient_WaitMessage(&client,5000);  // wait 5 sec
//    if (rc == MQTT_CODE_SUCCESS)
//    {
//        if (rc == MQTT_CODE_PUB_CONTINUE)
//        {
//           // sprintf(print_buff,"Received message on topic: %s\n", msg.topic_name);
//					 debug_msg("message received\n");
//          // sprintf(print_buff,"Payload: %.*s\n", msg.total_len, msg.buffer);
//					
//        }
//    }
//    else if (rc == MQTT_CODE_CONTINUE)
//    {
//			debug_msg("waiting for msg......\n");
//        /* No message yet, timeout elapsed */
//        continue;
//    }
//    else
//    {
//        sprintf(print_buff,"Error waiting for message: %d\n", rc);
//			debug_msg(print_buff);
//        //break;
//    }
			}
		
    MqttClient_Disconnect(&client);
    MqttClient_NetDisconnect(&client);
	
}

///* Optional message callback */
//static int mqtt_message_cb(MqttClient *client, MqttMessage *msg, byte msg_new, byte msg_done)
//{
//    if (msg_new) 
//			{
//        sprintf(print_buff,"Received message on topic %s: %.*s\n",
//               msg->topic_name,
//               msg->buffer_len,
//               msg->buffer);
//				debug_msg(print_buff);
//    }
//    return MQTT_CODE_SUCCESS;
//}



//int32_t Msocketmanagertask(void)
//{
//	
//   volatile int rc;
//    MqttClient client;
//    MqttNet net;
//	
//	 
//	
//    MqttConnect connect;
//    MqttPublish publish;

//    /* Step 1: Initialize network (this typically just zeros structure) */
//    memset(&net, 0, sizeof(net));
//	
//	 net.connect = MqttSocket_Connect;
//    net.read    = MqttSocket_Read;
//    net.write   = MqttSocket_Write;
//    net.disconnect = MqttSocket_Disconnect;

//    /* Step 2: Initialize MQTT client */
//    rc = MqttClient_Init(&client, &net, mqtt_message_cb,
//                         tx_buf, sizeof(tx_buf),
//                         rx_buf, sizeof(rx_buf),
//                         MQTT_CMD_TIMEOUT_MS);
//    if (rc != MQTT_CODE_SUCCESS) {
//        sprintf(print_buff,"\r\nMqttClient_Init error: %d\n",rc);
//			debug_msg(print_buff);
//        //return;
//    }
//		else
//		{
//			debug_msg("\r\nMQTT INit success");
//		}

//    /* Step 3: Connect to broker */
//    rc = MqttSocket_Connect(&client, MQTT_HOST, MQTT_PORT, 5000, 0, NULL);
//    if (rc != MQTT_CODE_SUCCESS)
//			{
//        sprintf(print_buff,"\r\nSocket connect failed: %d\n",rc);
//				debug_msg(print_buff);
//       // return;
//    }
//		else
//			{
//					debug_msg("\r\nMQTT INit success");
//			}
//    //debug_msg("Connected to broker: %s\n", MQTT_HOST);

//    /* Step 4: Prepare and send CONNECT packet */
//    memset(&connect, 0, sizeof(connect));
//    connect.keep_alive_sec = 60;
//    connect.clean_session = 1;
//    connect.client_id = CLIENT_ID;

//    rc = MqttClient_Connect(&client, &connect);
//    if (rc != MQTT_CODE_SUCCESS) 
//			{
//       
//			sprintf(print_buff,"MQTT connect packet failed: %d\n",rc);
//				debug_msg(print_buff);
//    }
//    debug_msg("MQTT CONNECT OK\n");

//    /* Step 5: Publish message */
//    memset(&publish, 0, sizeof(publish));
//    publish.qos = MQTT_QOS_0;
//    publish.topic_name = MQTT_TOPIC;
//    publish.buffer = (byte*)MQTT_MESSAGE;
//    publish.total_len = strlen(MQTT_MESSAGE);

//    rc = MqttClient_Publish(&client, &publish);
//    if (rc == MQTT_CODE_SUCCESS) 
//			{
//        sprintf(print_buff,"Published: %s ? %s\n", MQTT_TOPIC, MQTT_MESSAGE);
//				debug_msg(print_buff);
//    } else 
//			{
//				sprintf(print_buff,"Publish failed: %d\n", rc);
//				debug_msg(print_buff);
//    }

//    /* Step 6: Disconnect cleanly */
//    MqttClient_Disconnect(&client);
//    MqttSocket_Disconnect(&client);
//    debug_msg("Disconnected.\n");
//}

//int32_t Msocketmanagertask(void)
//{
//	
//websocket_msg_t ws_msg = {
//    .message = "Hello from Postman By JAI!",
//    .key = "test-123"
//};
//event_log_t dummy_data = 
//{
//    .u16_event_id        = 3001U,
//    .log_num             = 1U,
//    .u8_device_text      = "Smoke",
//    .u8_zone_text        = "Zone1",
//    .u8_zone_number      = 1U,
//    .u8_node_address     = 2U,
//    .u8_device_address   = 1U,
//    .u8_device_type      = 3U,
//    .u8_device_sub_type  = 4U,
//    .u8_date             = 12U,
//    .u8_month            = 10U,
//    .u8_year             = 2025U,
//    .u8_hours            = 14U,
//    .u8_minutes          = 22U,
//    .u8_seconds          = 23U,
//    .u8_logbitoffset     = 1U,
//    .u8_serialNumber     = 55U,
//    .u16_crc             = 0xABCDU
//};
//	
//	uint8_t u8_socket_Conn_flag = 10;

//	int32_t i32_ret = CLEAR;

//	volatile ip_addr_t ip_address;

//	struct sockaddr_in servaddr;
//	 
//	
//	int ret;

//	debug_msg((uint8_t *)pf_scp_config->https.url);
//	// collecting server ip through DNS
//	//i32_ret = netconn_gethostbyname((char *)pf_scp_config->https.url, &ip_address);

////	if (NO_INTERNET_FOR_DNS < i32_ret)
////	{
////		u8_socket_Conn_flag = SET;
////		// TODO sent nack to main panel
////	}
////	sprintf((char *)ac_print_buff, "\r\nIP addr of %s is %s i32_ret %d", pf_scp_config->https.url, ipaddr_ntoa(&ip_address), i32_ret);
////	debug_msg((uint8_t *)ac_print_buff);

//	// Create socket and connecti!ng server
//	
//	/* Convert string to lwIP ip_addr_t */
//  ipaddr_aton(MQTT_IP, &ip_address);

//	while (u8_socket_Conn_flag != 0)
//	{
//		if ((i32_sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0)
//		{
//			debug_msg((uint8_t *)"\r\nSocket creation failed\n");
//			i32_ret = i32_sockfd;
//		}

//		sprintf((char *)ac_print_buff, "\r\nsocket created with fd  %d ", i32_sockfd);
//		debug_msg((uint8_t *)ac_print_buff);

//		servaddr.sin_family = AF_INET;
//		servaddr.sin_port = htons(MQTT_PORT);
//		servaddr.sin_addr.s_addr = ip_address.addr;

//		// Connect to the server
//		if ((i32_ret = lwip_connect(i32_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
//		{
//			u8_socket_Conn_flag--;
//			sprintf((char *)ac_print_buff, "\r\nConnection failed %d\n", i32_ret);
//			debug_msg((uint8_t *)ac_print_buff);
//			lwip_close(i32_sockfd);
//		}
//		else
//		{
//			sprintf((char *)ac_print_buff, "\r\nok...  socket connected successsfully  %d ", i32_sockfd);
//			debug_msg((uint8_t *)ac_print_buff);
//			
//			sprintf(tx_buf,
//			 "POST /send-message HTTP/1.1\r\n"
//		"Content-Type: application/json\r\n"
//		"User-Agent: PostmanRuntime/7.48.0\r\n"
//		"Accept: */*\r\n"
//		"Postman-Token: f1a70a82-d503-457a-be8f-e1617bbe414f\r\n"
//		"Host: 54.163.223.188:3000\r\n"
//		"Accept-Encoding: gzip, deflate, br\r\n"
//		"Connection: keep-alive\r\n"
//		"Content-Length: 62\r\n"
//		"\r\n"
//		"{ \"message\": \"Hello from Postman By JAI!\", \"key\": \"test-123\" }");
//			
//			
//			ret = lwip_send(i32_sockfd,tx_buf,strlen(tx_buf),NULL);
//			
//			if(ret < 0)
//			{
//				debug_msg((uint8_t *)"\r\nlwip write failed ");
//			}
//			else
//			{
//				debug_msg((uint8_t *)"\r\nlwip write success ");
//			}
//			debug_msg((uint8_t *)tx_buf);
////			ret = lwip_send(i32_sockfd,&ws_msg,sizeof(ws_msg),NULL);
////			
////			if(ret < 0)
////			{
////				debug_msg((uint8_t *)"\r\nlwip write failed ");
////			}
////			else
////			{
////				debug_msg((uint8_t *)"\r\nlwip write success ");
////			}
//			
//			lwip_recv(i32_sockfd,rx_buf,500,NULL);
//			
//			debug_msg((uint8_t *)rx_buf);
//			
//			// connected to server
//			u8_socket_Conn_flag = 0;

//			// starting https client task
//			//wolf_ssl_task();
//		}
//	}
//	return i32_ret;
//}

void mqtt_process(void)
{
	/*
	todo:
	*/
}














/*************************************************************************************************************************************************
 * @brief This will get a call back when tehre is stack overflow with the task
 *        that cause the overflow.
 * @author Mohammed Shameem
 * @param xTask      :- Task handle
 *        pcTaskName :- name of task caused the stack overflow
 *
 *************************************************************************************************************************************************/
//void vApplicationStackOverflowHook(TaskHandle_t xTask, uint8_t *pcTaskName)
//{
//	sprintf((char *)ac_print_buff, "\r\n********Stack overflow in task: %s***************\r\n", pcTaskName);
//	debug_msg((uint8_t *)ac_print_buff);
//	Error_Handler();
//}

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
	switch (u8_service)
	{
	case ETH_CLOUD_FW_DOWNLOAD_QUEUE:
	{
		if (Msocketmanagertask() >= 0)
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
		break;
	}
	case ETH_CLOUD_POST_QUEUE:
	{
		if (vsocketmanagertask() >= 0)
		{
			if (sent_fire_fault(ssl, au8_tx_data, u16_tx_data_len) <= 0)
			{
				debug_msg("\r\nLOG upload  failed\r\n");
			}
			else
			{
				debug_msg("\r\nLOG upload success\r\n");
			}
		}
		else
		{
			debug_msg("\r\nerror : internet error");
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
