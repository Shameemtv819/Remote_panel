/**
 * @file mqtt_client.c
 * @author shameem
 * @brief 
 * @date 2025-11-20
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "mqtt_app.h"
/*********************************************************** Structure definitions  **********************************************************/
/*structure to sent fire fault for register log in cloud*/
typedef struct __attribute__((__packed__))
{
    uint16_t startof_file;
    uint16_t u16_source;
    uint16_t payload_length;
    uint16_t packetNumber;
    uint16_t u16_event_id;
    uint16_t log_num;
    uint8_t au8_device_text[21];
    uint8_t u8_zone_number;
    uint8_t u8_node_address;
    uint8_t u8_device_address;
    uint8_t u8_device_type;
    uint8_t u8_device_sub_type;
    uint8_t u8_date;
    uint8_t u8_month;
    uint8_t u8_year;
    uint8_t u8_hours;
    uint8_t u8_minutes;
    uint8_t u8_seconds;
    uint8_t u8_logbitoffset;
    uint16_t serial_no;
    uint16_t u16_crc;
} EventLog_t;

/* USER CODE END Private defines */
EventLog_t fire = {
    .u16_event_id = 2010,
    .log_num = 56,
    .au8_device_text = "Sensor Activated", // up to 20 chars (+1 for '\0')
    .u8_zone_number = 5,
    .u8_node_address = 12,
    .u8_device_address = 7,
    .u8_device_type = 2,
    .u8_device_sub_type = 1,
    .u8_date = 8,
    .u8_month = 11,
    .u8_year = 25,
    .u8_hours = 14,
    .u8_minutes = 37,
    .u8_seconds = 52,
    .serial_no = 5000,
    .u8_logbitoffset = 3,
    .u16_crc = 0xABCD, // placeholder CRC
    .u16_source = 0X400B,
    .payload_length = 42,
};

/*status update structure*/
typedef struct __attribute__((__packed__))
{
    uint16_t sl_no;
    uint8_t status;
} status_st;

/*socket file descriptor structure*/
typedef struct
{
    int socket_fd;
} MqttLwipContext;
/************************************************************** Macros & enums ****************************************************************/
/*Brocker url*/
#define MQTT_HOST "ec2-34-227-142-219.compute-1.amazonaws.com"
/*Brocker port*/
#define MQTT_PORT (1883U)
/*Topic to update status*/
#define MQTT_TOPIC "Emcus/status"

#define MQTT_FIRE_FAULT_TOPIC "Emcus/logs"

#define NO_INTERNET_FOR_DNS (0U)

#define MAX_RETRIES         (10U)

#define RETRY_DELAY_MS      (2000U)

#define MQTT_CONN_RETRY     (2U)



/************************************************************** static global variables ****************************************************************/
static MqttNet net;
static MqttClient client;
static uint8_t u8_print_buff[150];
static unsigned char txtx_buf[0x100];
static unsigned char rxrx_buf[0x100];

 uint8_t u8_mqtt_state = MQTT_INIT;

/************************************************************** static function prototypes ****************************************************************/
static int MqttNet_Init(MqttNet *net);
static int mqtt_tls_cb(MqttClient *client);
static int mqtt_lwip_disconnect(void *context);
static int mqtt_lwip_read(void *context, byte *buf, int len, int timeout_ms);
static int mqtt_lwip_write(void *context, const byte *buf, int len, int timeout_ms);
static int mqtt_lwip_connect(void *context, const char *host, word16 port, int timeout_ms);
static int message_callback(struct _MqttClient *client, MqttMessage *message, byte msg_new, byte msg_done);

/************************************************************** function definitions ****************************************************************/
/**
 * @brief  TLS callback for MQTT client.
 * @author shameem
 * @param client Pointer to the MQTT client.
 * @return       Status of the TLS callback.
 */
static int mqtt_tls_cb(MqttClient *client)
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
    // wolfSSL_CTX_load_verify_buffer(ctx, ca_cert, sizeof(ca_cert), WOLFSSL_FILETYPE_PEM);
    debug_msg("\r\nTLS callback called.\n");

    return WOLFSSL_SUCCESS;
}

/**
 * @brief Attempts to establish a TCP connection to the specified host and port using LwIP.
 *@author shameem
 * @param context    Pointer to the MQTT network context.
 * @param host       Pointer to the hostname to connect to.
 * @param port       Port number to connect to.
 * @param timeout_ms Connection timeout in milliseconds.
 * @return           Status of the connection attempt.
 */
static int mqtt_lwip_connect(void *context, const char *host, word16 port, int timeout_ms)
{
    int i32_rc = 0U;
    struct hostent *hp;
    struct sockaddr_in server_addr;
    MqttLwipContext *ctx = (MqttLwipContext *)context;

    uint8_t au8_dbg_buff[128]; /* Debug buffer */

    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++)
    {
        snprintf(au8_dbg_buff, sizeof(au8_dbg_buff),
                 "MQTT: Attempt %d/%d connecting to %s:%d\n",
                 attempt, MAX_RETRIES, host, port);
        debug_msg(au8_dbg_buff);

        /* DNS lookup */
        hp = gethostbyname(host);
        if (hp == NULL)
        {

            snprintf(au8_dbg_buff, sizeof(au8_dbg_buff),
                     "MQTT: DNS failed on attempt %d\n",
                     attempt);
            debug_msg(au8_dbg_buff);

            sys_msleep(RETRY_DELAY_MS);
            continue;
        }

        /* Create socket */
        ctx->socket_fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
        if (ctx->socket_fd < 0)
        {

            snprintf(au8_dbg_buff, sizeof(au8_dbg_buff),
                     "MQTT: socket create failed on attempt %d\n",
                     attempt);
            debug_msg(au8_dbg_buff);

            sys_msleep(RETRY_DELAY_MS);
            continue;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        memcpy(&server_addr.sin_addr, hp->h_addr, hp->h_length);

        /* Set timeouts */
        int timeout = timeout_ms;
        lwip_setsockopt(ctx->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        lwip_setsockopt(ctx->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        /* Attempt connect */
        i32_rc = lwip_connect(ctx->socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (i32_rc == 0)
        {

            snprintf(au8_dbg_buff, sizeof(au8_dbg_buff),
                     "MQTT: Connected successfully on attempt %d\n",
                     attempt);
            debug_msg(au8_dbg_buff);

            return MQTT_CODE_SUCCESS;
        }

        /* On failure */
        snprintf(au8_dbg_buff, sizeof(au8_dbg_buff),
                 "MQTT: connect failed (%d) on attempt %d\n",
                 i32_rc, attempt);
        debug_msg(au8_dbg_buff);

        lwip_close(ctx->socket_fd);
        ctx->socket_fd = -1;

        sys_msleep(RETRY_DELAY_MS);
    }

    /* All retries exhausted */
    snprintf(au8_dbg_buff, sizeof(au8_dbg_buff),
             "MQTT: All connection attempts failed\n");
    debug_msg(au8_dbg_buff);

    return MQTT_CODE_ERROR_NETWORK;
}

/**
 * @brief  Reads data from the MQTT network.
 * @author shameem
 * @param context    Pointer to the MQTT network context.
 * @param buf        Buffer to store the read data.
 * @param len        Length of the buffer.
 * @param timeout_ms Read timeout in milliseconds.
 * @return           Number of bytes read or error code.
 */
static int mqtt_lwip_read(void *context, byte *buf, int len, int timeout_ms)
{

    int i32_recvd;
    fd_set readset;
    struct timeval tv;
    MqttLwipContext *ctx = (MqttLwipContext *)context;

    FD_ZERO(&readset);
    FD_SET(ctx->socket_fd, &readset);

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int i32_rc = lwip_select(ctx->socket_fd + 1, &readset, NULL, NULL, &tv);

    if (i32_rc == 0)
    {
        // Timeout ? no data but connection still alive
        return MQTT_CODE_ERROR_TIMEOUT;
    }
    else if (i32_rc < 0)
    {
        // Select failed ? likely internet down
        return MQTT_CODE_ERROR_NETWORK;
    }

    i32_recvd = lwip_recv(ctx->socket_fd, buf, len, 0);
    if (i32_recvd < 0)
    {
        debug_msg("\r\nrecv failed timeout\n");
        return MQTT_CODE_ERROR_TIMEOUT;
    }
    else if (i32_recvd == 0)
    {
        debug_msg("\r\nrecv failed internet error\n");
        return MQTT_CODE_ERROR_NETWORK; /* Connection closed */
    }

    return i32_recvd;
}

/**
 * @brief Writes data to the MQTT network.
 *@author shameem
 * @param context    Pointer to the MQTT network context.
 * @param buf        Buffer containing the data to write.
 * @param len        Length of the data to write.
 * @param timeout_ms Write timeout in milliseconds.
 * @return Number of bytes written or error code.
 */
static int mqtt_lwip_write(void *context, const byte *buf, int len, int timeout_ms)
{

    int i32_sent;
    fd_set writeset;
    struct timeval tv;
    MqttLwipContext *ctx = (MqttLwipContext *)context;

    FD_ZERO(&writeset);
    FD_SET(ctx->socket_fd, &writeset);

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int i32_rc = lwip_select(ctx->socket_fd + 1, NULL, &writeset, NULL, &tv);

    if (i32_rc == 0)
    {
        return MQTT_CODE_ERROR_TIMEOUT;
    }
    else if (i32_rc < 0)
    {
        return MQTT_CODE_ERROR_NETWORK;
    }

    i32_sent = lwip_send(ctx->socket_fd, buf, len, 0);
    if (i32_sent < 0)
    {
        debug_msg("\r\nsent failed\n");
        return MQTT_CODE_ERROR_TIMEOUT;
    }

    return i32_sent;
}

/**
 * @brief  Disconnects the MQTT network.
 *@author shameem
 * @param context Pointer to the MQTT network context.
 * @return        Status of the disconnection.
 */
static int mqtt_lwip_disconnect(void *context)
{
    MqttLwipContext *ctx = (MqttLwipContext *)context;

    if (ctx->socket_fd >= 0)
    {
        lwip_close(ctx->socket_fd);
        ctx->socket_fd = -1;
#if MQTT_NET_DEBUG
        printf("MQTT: Disconnected\n");
#endif
    }
    return MQTT_CODE_SUCCESS;
}

/**
 * @brief Initializes the MQTT network structure with LwIP callbacks.
 *@author shameem
 * @param net Pointer to the MQTT network structure to initialize.
 * @return    Status of the initialization.
 */
static int MqttNet_Init(MqttNet *net)
{

    static MqttLwipContext lwipCtx; /* Can be static or allocated dynamically */
    lwipCtx.socket_fd = -1;

    net->context = &lwipCtx;
    net->connect = mqtt_lwip_connect;
    net->read = mqtt_lwip_read;
    net->write = mqtt_lwip_write;
    net->disconnect = mqtt_lwip_disconnect;

    return MQTT_CODE_SUCCESS;
}

/**
 * @brief Callback function to handle incoming MQTT messages.
 * @author shameem
 * @param client   Pointer to the MQTT client.
 * @param message  Pointer to the received MQTT message.
 * @param msg_new  Flag indicating if this is a new message.
 * @param msg_done Flag indicating if the message is complete.
 * @return         Status of the message handling.
 */
static int message_callback(struct _MqttClient *client, MqttMessage *message, byte msg_new, byte msg_done)
{
    debug_msg("\r\nrecv message from brocker\n");

    uint8_t u8_print_buff[256];

    switch (message->type)
    {
    case MQTT_PACKET_TYPE_PUBLISH:
    {
        MqttPublish *publish = (MqttPublish *)message;

        sprintf(u8_print_buff, "Received PUBLISH topic: %s\n", publish->topic_name);
        debug_msg(u8_print_buff);

        sprintf(u8_print_buff, "Payload: %.*s\n", publish->total_len, publish->buffer);
        debug_msg(u8_print_buff);

        break;
    }

    case MQTT_PACKET_TYPE_PING_RESP:
        debug_msg("PING response received.\n");
        break;

    case MQTT_PACKET_TYPE_DISCONNECT:
        debug_msg("Server sent DISCONNECT.\n");
        break;

    default:
        sprintf(u8_print_buff, "Unhandled message type: %d\n", message->type);
        debug_msg(u8_print_buff);
        break;
    }

    return MQTT_CODE_SUCCESS; // always return success unless you want to abort
}

/**
 * @brief MQTT task to handle connection and messaging
 * @author shameem
 * @param None
 */
char mqtt_process(void)
{
   char c_ret = 0;
    MqttPublish publish;
    uint8_t au8_status_buf[] = "{\n  \"serialNumber\": \"5000\",\n  \"status\": \"active\"\n}";
    ip_addr_t ip_address;
	uint8_t u8_conn_retry_cnt = CLEAR;

    static uint16_t temp_cnt = 0;

    int i32_rc;

    // while (1)
    // {
    switch (u8_mqtt_state)
    {
    case MQTT_INIT:
    {
			u8_conn_retry_cnt = CLEAR;
			
        MqttNet_Init(&net);

        client.msg_cb = message_callback;

        /* Initialize network and client */
        i32_rc = MqttClient_Init(&client, &net, message_callback, txtx_buf, sizeof(txtx_buf),
                                 rxrx_buf, sizeof(rxrx_buf), 10000);

        if (i32_rc != MQTT_CODE_SUCCESS)
        {
            sprintf(u8_print_buff, "\r\nMqttClient_Init error: %d\n", i32_rc);
            debug_msg(u8_print_buff);
        }

        /*Try connecting to brocker until internet is available*/
        do
        {
            i32_rc = MqttClient_NetConnect(&client, MQTT_HOST, MQTT_PORT, 10000, 0, mqtt_tls_cb);
            if (i32_rc != MQTT_CODE_SUCCESS)
            {
                sprintf(u8_print_buff, "\r\nMqttClient_Netconnect error: %d\n", i32_rc);
                debug_msg(u8_print_buff);
							u8_conn_retry_cnt++;
            }
        } while (u8_conn_retry_cnt >= 2);

        MqttConnect connect;
        memset(&connect, 0, sizeof(connect));
        connect.keep_alive_sec = 60;
        connect.clean_session = 1;
        connect.client_id = "stm5000";

        i32_rc = MqttClient_Connect(&client, &connect);
        if (i32_rc == MQTT_CODE_SUCCESS)
        {
            debug_msg("MQTT CONNECTED!\n");
        }
        else
        {
            sprintf(u8_print_buff, "\r\nMQTT connect error: %d\n", i32_rc);
            debug_msg(u8_print_buff);
					c_ret = -1;
        }

        // Step 4: Subscribe to topics
        MqttSubscribe mqtt_sub;
        MqttTopic topics[1];
        memset(&mqtt_sub, 0, sizeof(mqtt_sub));
        mqtt_sub.packet_id = 1;
        mqtt_sub.topic_count = 1;
        mqtt_sub.topics = topics;
        topics[0].topic_filter = "Emcus/ota/command";
        topics[0].qos = MQTT_QOS_0;

        i32_rc = MqttClient_Subscribe(&client, &mqtt_sub);
        if (i32_rc != MQTT_CODE_SUCCESS)
        {
            sprintf(u8_print_buff, "Subscribe failed: %d\n", i32_rc);
            debug_msg(u8_print_buff);
					c_ret -1;
        }
        else
        {
            debug_msg("Subscribed to stm32/test\n");
        }

        debug_msg("Wolf mQTT init is done starting routine opertions\n");
        u8_mqtt_state = MQTT_ROUTINE_OPERATION;

				MqttClient_WaitMessage(&client, 5000);
				 MqttClient_Ping(&client);
				
        break;
    }

    case MQTT_ROUTINE_OPERATION:
    {
			status_st st_status = {.sl_no = 5000,
			                       .status = 0};

			  memset(&publish, 0, sizeof(publish));
        publish.qos = 0;
        publish.packet_id = 4;
        publish.topic_name = MQTT_TOPIC;
        publish.buffer = (byte *)&st_status;
        publish.total_len = (word16)sizeof(status_st);
        // Step 5: Message processing loop
//        debug_msg("Waiting for messages...\n");
        // while (u8_mqtt_state != MQTT_INIT)
        // {

        i32_rc = MqttClient_WaitMessage(&client, 5000); // Wait up to 5 seconds
        i32_rc = MqttClient_Ping(&client);
        if (1 == u8_one_sec_flag)
        {
            u8_one_sec_flag = 0U;
            i32_rc = MqttClient_Publish(&client, &publish);
           // temp_cnt++;
//            if (temp_cnt > 60)
//            {
                //mqtt_publish_fire_fault(NULL, 0);
//							BaseType_t xHigherPriorityTaskWoken = pdFAIL;
//              BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
//              queue_data_def eth_data = {0};
//							 eth_data.service = ETH_CLOUD_FW_DOWNLOAD_QUEUE;
//                xQueueSendFromISR(queue_eth, (queue_data_def *)&eth_data, &pxHigherPriorityTaskWoken);
                temp_cnt = 0;
           // }
        }

        if (i32_rc == MQTT_CODE_SUCCESS)
        {
            // Incoming PUBLISH message will trigger mqtt_message_cb automatically
        }
        else if (i32_rc == MQTT_CODE_CONTINUE)
        {
            // No message yet, timeout expired
            debug_msg("\n.");
            i32_rc = MqttClient_Ping(&client);
            // continue;
        }
        else
        {
            sprintf(u8_print_buff, "WaitMessage error: %d\n", i32_rc);
            debug_msg(u8_print_buff);
            u8_mqtt_state = MQTT_INIT;
            debug_msg("Internet Error waiting for internet\r\n");
            MqttClient_Disconnect(&client);
            MqttClient_NetDisconnect(&client);
					c_ret = -1;
        }
        // }
        break;
    }

    default:
    {
        break;
    }
    }
    // }
		
		return c_ret;
}



/**
 * @brief  Publish a fire fault message to the MQTT broker.
 * @author shameem
 * @param payload     Pointer to the payload data.
 * @param payload_len Length of the payload data.
 * @return            Status of the publish operation.
 */
int mqtt_publish_fire_fault(uint8_t *payload, uint16_t payload_len)
{
    uint8_t u8_print_buff[100];
    static int i32_rc = 0;
    MqttPublish publish = {0};
    publish.qos = 2;
    publish.packet_id = 4;
    publish.topic_name = MQTT_FIRE_FAULT_TOPIC;
    publish.buffer = (byte *)payload;
    publish.total_len = (word16) payload_len;

    i32_rc = MqttClient_Publish(&client, &publish);
    debug_msg("published fire LOG\r\n");
    return i32_rc;
}

/**
 * @brief  Publish a starting ota notification to the MQTT broker.
 * @author shameem
 * @param  Void
 * @return Status of the publish operation.
 */
int mqtt_notify_ota(void)
{
	
		int i32_rc = 0;
		uint8_t u8_print_buff[100];
		MqttPublish publish = {0};
		status_st st_status = {.sl_no = 5000,
															 .status = 2};

		memset(&publish, 0, sizeof(publish));
		publish.qos = 0;
		publish.packet_id = 4;
		publish.topic_name = MQTT_TOPIC;
		publish.buffer = (byte *)&st_status;
		publish.total_len = (word16)sizeof(status_st);
														 
		i32_rc = MqttClient_Publish(&client, &publish);
    debug_msg("published notification starting ota update\r\n");
    return i32_rc;
}