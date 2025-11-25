/**
 * @file mqtt_client.c
 * @author shameem
 * @brief
 * @date 2025-11-20
 * @copyright Copyright (c) 2025
 */

/************************************************************* Header includes ***************************************************************/
#include "mqtt.h"
/*********************************************************** Structure definitions  **********************************************************/
/*structure to sent fire fault for register log in cloud*/
typedef struct
{
    uint16_t u16_event_id;
    uint16_t log_num;
    char u8_device_text[16];
    char u8_zone_text[16];
    uint8_t u8_zone_number;
    uint8_t u8_node_address;
    uint8_t u8_device_address;
    uint8_t u8_device_type;
    uint8_t u8_device_sub_type;
    uint8_t u8_date;
    uint8_t u8_month;
    uint16_t u8_year;
    uint8_t u8_hours;
    uint8_t u8_minutes;
    uint8_t u8_seconds;
    uint8_t u8_logbitoffset;
    uint8_t u8_serialNumber;
    uint16_t u16_crc;
} event_log_t;

/*status update structure*/
typedef struct
{
    uint16_t sl_no;
    uint8_t status[10];
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

#define NO_INTERNET_FOR_DNS (0U)

#define MAX_RETRIES (10U)

#define RETRY_DELAY_MS (2000U)

/*Modes in mqtt operation */
enum mqtt_mode
{
    MQTT_INIT,
    MQTT_ROUTINE_OPERATION,
};

/************************************************************** static global variables ****************************************************************/
static MqttNet net;
static uint8_t u8_print_buff[150];
static unsigned char txtx_buf[0x100];
static unsigned char rxrx_buf[0x100];

/*************************************************************************************************************************************************
  * @brief  This function will all tcp related operations until its get connected to server.
            Ones the socket is connected with server wolfssl task will be started.
            Until the socket connectes to server this funtion will be trying it in super loop.
  * @param  void
  * @author Mohammed Shameeme
 *************************************************************************************************************************************************/


/**
 * @brief 
 * @param client 
 * @return 
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
 * @brief 
 * 
 * @param context 
 * @param host 
 * @param port 
 * @param timeout_ms 
 * @return 
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
 * @brief 
 * 
 * @param context 
 * @param buf 
 * @param len 
 * @param timeout_ms 
 * @return 
 */
static int mqtt_lwip_read(void *context, byte *buf, int len, int timeout_ms)
{
    MqttLwipContext *ctx = (MqttLwipContext *)context;
    int i32_recvd;
    fd_set readset;
    struct timeval tv;

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
 * @brief 
 * 
 * @param context 
 * @param buf 
 * @param len 
 * @param timeout_ms 
 * @return 
 */
static int mqtt_lwip_write(void *context, const byte *buf, int len, int timeout_ms)
{
    MqttLwipContext *ctx = (MqttLwipContext *)context;
    int i32_sent;
    fd_set writeset;
    struct timeval tv;

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
 * @brief 
 * 
 * @param context 
 * @return 
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
 * @brief 
 * 
 * @param net 
 * @return 
 */
int MqttNet_Init(MqttNet *net)
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
 * @brief 

 * @param client 
 * @param message 
 * @param msg_new 
 * @param msg_done 
 * @return 
 */
int message_callback(struct _MqttClient *client, MqttMessage *message, byte msg_new, byte msg_done)
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
 * @brief 
 * 
 * @param  
 */
void mqtt_task(void)
{
    uint8_t u8_mqtt_state = MQTT_INIT;
    MqttPublish publish;
    char char1[] = "{\n  \"serialNumber\": \"5000\",\n  \"status\": \"active\"\n}";
    MqttClient client;
    ip_addr_t ip_address;

    int i32_rc;

    while (1)
    {
        switch (u8_mqtt_state)
        {
        case MQTT_INIT:
        {
            MqttNet_Init(&net);

            client.msg_cb = message_callback;

            wolfSSL_Debugging_ON();
            wolfSSL_SetLoggingCb(wolfssl_print);

            /* Initialize network and client */
            i32_rc = MqttClient_Init(&client, &net, message_callback, txtx_buf, sizeof(txtx_buf),
                                 rxrx_buf, sizeof(rxrx_buf), 1000);

            if (i32_rc != MQTT_CODE_SUCCESS)
            {
                sprintf(u8_print_buff, "\r\nMqttClient_Init error: %d\n", i32_rc);
                debug_msg(u8_print_buff);
            }

            do
            {
                i32_rc = MqttClient_NetConnect(&client, MQTT_HOST, MQTT_PORT, 1000, 0, mqtt_tls_cb);
                if (i32_rc != MQTT_CODE_SUCCESS)
                {
                    sprintf(u8_print_buff, "\r\nMqttClient_Netconnect error: %d\n", i32_rc);
                    debug_msg(u8_print_buff);
                }
            } while (i32_rc != MQTT_CODE_SUCCESS);

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
            }
            else
            {
                debug_msg("Subscribed to stm32/test\n");
            }

            debug_msg("Wolf mQTT init is done starting routine opertions\n");
            u8_mqtt_state = MQTT_ROUTINE_OPERATION;

            break;
        }

        case MQTT_ROUTINE_OPERATION:
        {
            memset(&publish, 0, sizeof(publish));
            publish.qos = 0;
            publish.packet_id = 4;
            publish.topic_name = MQTT_TOPIC;
            publish.buffer = (byte *)char1;
            publish.total_len = (word16)strlen(char1);

            // Step 5: Message processing loop
            debug_msg("Waiting for messages...\n");
            while (u8_mqtt_state != MQTT_INIT)
            {

                i32_rc = MqttClient_WaitMessage(&client, 5000); // Wait up to 5 seconds
                i32_rc = MqttClient_Ping(&client);
                if (1 == u8_one_sec_flag)
                {
                    u8_one_sec_flag = 0U;
                    i32_rc = MqttClient_Publish(&client, &publish);
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
                    continue;
                }
                else
                {
                    sprintf(u8_print_buff, "WaitMessage error: %d\n", i32_rc);
                    debug_msg(u8_print_buff);
                    u8_mqtt_state = MQTT_INIT;
                    debug_msg("Internet Error waiting for internet\r\n");
                    MqttClient_Disconnect(&client);
                    MqttClient_NetDisconnect(&client);
                    // break;  // optionally reconnect
                }
            }
            break;
        }

        default:
        {
            break;
        }
        }
    }

    MqttNet_Init(&net);

    client.msg_cb = message_callback;

    wolfSSL_Debugging_ON();
    wolfSSL_SetLoggingCb(wolfssl_print);

    /* Initialize network and client */
    i32_rc = MqttClient_Init(&client, &net, message_callback, txtx_buf, sizeof(txtx_buf),
                         rxrx_buf, sizeof(rxrx_buf), 30000);

    if (i32_rc != MQTT_CODE_SUCCESS)
    {
        sprintf(u8_print_buff, "\r\nMqttClient_Init error: %d\n", i32_rc);
        debug_msg(u8_print_buff);
    }

    i32_rc = MqttClient_NetConnect(&client, MQTT_HOST, MQTT_PORT, 30000, 0, mqtt_tls_cb);
    if (i32_rc != MQTT_CODE_SUCCESS)
    {
        sprintf(u8_print_buff, "\r\nMqttClient_Init error: %d\n", i32_rc);
        debug_msg(u8_print_buff);
    }

    MqttConnect connect;
    memset(&connect, 0, sizeof(connect));
    connect.keep_alive_sec = 60;
    connect.clean_session = 1;
    connect.client_id = "stm134330";

    i32_rc = MqttClient_Connect(&client, &connect);
    if (i32_rc == MQTT_CODE_SUCCESS)
    {
        debug_msg("MQTT CONNECTED!\n");
    }
    else
    {
        sprintf(u8_print_buff, "\r\nMQTT connect error: %d\n", i32_rc);
        debug_msg(u8_print_buff);
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
    }
    else
    {
        debug_msg("Subscribed to stm32/test\n");
    }

    i32_rc = MqttClient_Ping(&client);
    memset(&publish, 0, sizeof(publish));
    publish.qos = 0;
    publish.packet_id = 4;
    publish.topic_name = MQTT_TOPIC;
    publish.buffer = (byte *)char1;
    publish.total_len = (word16)strlen(char1);

    // Step 5: Message processing loop
    debug_msg("Waiting for messages...\n");
    while (1)
    {

        i32_rc = MqttClient_WaitMessage(&client, 5000); // Wait up to 5 seconds
        i32_rc = MqttClient_Ping(&client);
        if (1 == u8_one_sec_flag)
        {
            u8_one_sec_flag = 0U;
            i32_rc = MqttClient_Publish(&client, &publish);
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
            continue;
        }
        else
        {
            sprintf(u8_print_buff, "WaitMessage error: %d\n", i32_rc);
            debug_msg(u8_print_buff);
        }
    }

    MqttClient_Disconnect(&client);
    MqttClient_NetDisconnect(&client);
}

////byte tx_buf[1024], rx_buf[1024];
// void mqtt_task(void)
//{
//	char  char1[] =  "{\n  \"serialNumber\": \"5000\",\n  \"status\": \"active\"\n}";
//	MqttClient client;
//
//     int i32_rc;

// MqttNet_Init(&net);
//
//	client.msg_cb = message_callback;
//
//
//	wolfSSL_Debugging_ON();
//	wolfSSL_SetLoggingCb(wolfssl_print);
//
//     /* Initialize network and client */
//     i32_rc = MqttClient_Init(&client, &net, message_callback, txtx_buf, sizeof(txtx_buf),
//                     rxrx_buf, sizeof(rxrx_buf),30000);
//
//		 if (i32_rc != MQTT_CODE_SUCCESS) {
//          sprintf(u8_print_buff,"\r\nMqttClient_Init error: %d\n",i32_rc);
//			debug_msg(u8_print_buff);
//
//     }

//    i32_rc = MqttClient_NetConnect(&client, MQTT_HOST, MQTT_PORT, 30000, 0, mqtt_tls_cb);
//    if (i32_rc != MQTT_CODE_SUCCESS)
//		{
//      sprintf(u8_print_buff,"\r\nMqttClient_Init error: %d\n",i32_rc);
//			debug_msg(u8_print_buff);
//
//    }

//    MqttConnect connect;
//    memset(&connect, 0, sizeof(connect));
//    connect.keep_alive_sec = 60;
//    connect.clean_session = 1;
////    connect.username = MQTT_USERNAME;
////    connect.password = MQTT_PASSWORD;
//    connect.client_id = "stm134330";
//		//connect.client_id_len = strlen("stm32client123");
//

//    i32_rc = MqttClient_Connect(&client, &connect);
//    if (i32_rc == MQTT_CODE_SUCCESS)
//		{
//       debug_msg("MQTT CONNECTED!\n");
//    }
//		else
//		{
//			sprintf(u8_print_buff,"\r\nMQTT connect error: %d\n",i32_rc);
//			debug_msg(u8_print_buff);
//    }

//		// Step 4: Subscribe to topics
//    MqttSubscribe mqtt_sub;
//    MqttTopic topics[1];
//    memset(&mqtt_sub, 0, sizeof(mqtt_sub));
//
//    mqtt_sub.packet_id = 1;
//    mqtt_sub.topic_count = 1;
//    mqtt_sub.topics = topics;
//    topics[0].topic_filter = "Emcus/ota/command";
//    topics[0].qos = MQTT_QOS_0;
//
//    i32_rc = MqttClient_Subscribe(&client, &mqtt_sub);
//    if (i32_rc != MQTT_CODE_SUCCESS)
//			{
//        sprintf(u8_print_buff,"Subscribe failed: %d\n", i32_rc);
//			 debug_msg(u8_print_buff);
//    }
//		else
//		{
//        debug_msg("Subscribed to stm32/test\n");
//    }
//
//
////    /* Publish example */
//    MqttPublish publish;
////    memset(&publish, 0, sizeof(publish));
////    publish.qos = 0;
////		publish.packet_id = 2;
////    publish.topic_name = MQTT_TOPIC;
////    publish.buffer = (byte*)MQTT_MESSAGE;
////    publish.total_len = (word16)strlen(MQTT_MESSAGE);

////    i32_rc = MqttClient_Publish(&client, &publish);
////    if (i32_rc == MQTT_CODE_SUCCESS)
////			{
////				sprintf(u8_print_buff,"Published: %s \n Topic : %s", MQTT_MESSAGE,MQTT_TOPIC);
////    debug_msg(u8_print_buff);
////    }
////		else
////		{
////			sprintf(u8_print_buff,"Publish failed: %d\n", i32_rc);
////    debug_msg(u8_print_buff);
////    }
////
//		 /* Publish example */
////    publish;
////    memset(&publish, 0, sizeof(publish));
////    publish.qos = 2;
////		publish.packet_id =3;
////    publish.topic_name = MQTT_TOPIC;
////    publish.buffer = (byte*)"hai hivemq";
////    publish.total_len = (word16)strlen("hai hivemq");

////    i32_rc = MqttClient_Publish(&client, &publish);
////    if (i32_rc == MQTT_CODE_SUCCESS)
////			{
////				sprintf(u8_print_buff,"Published: %s \n Topic : %s", MQTT_MESSAGE,MQTT_TOPIC);
////        debug_msg(u8_print_buff);
////
////				  /* ? Add this line for QoS > 0 */
////    }
////		else
////		{
////			sprintf(u8_print_buff,"Publish failed: %d\n", i32_rc);
////      debug_msg(u8_print_buff);
////    }
//
//		 /* Publish example */
//    //MqttPublish publish;
////    memset(&publish, 0, sizeof(publish));
////    publish.qos = 0;
////		publish.packet_id =4;
////    publish.topic_name = MQTT_TOPIC;
////  publish.buffer = (byte*)char1;
////					publish.total_len = (word16)strlen(char1);

////    i32_rc = MqttClient_Publish(&client, &publish);
////    if (i32_rc == MQTT_CODE_SUCCESS)
////			{
////				sprintf(u8_print_buff,"Published: %s \n Topic : %s", "hai hivemq",MQTT_TOPIC);
////    debug_msg(u8_print_buff);
////    }
////		else
////		{
////			sprintf(u8_print_buff,"Publish failed: %d\n", i32_rc);
////    debug_msg(u8_print_buff);
////    }
////		 /* Publish example */
////   // MqttPublish publish;
////    memset(&publish, 0, sizeof(publish));
////    publish.qos = 0;
////		publish.packet_id =5;
////    publish.topic_name = MQTT_TOPIC;
////    publish.buffer = (byte*)"from emcus";
////    publish.total_len = (word16)strlen("from emcus");

////    i32_rc = MqttClient_Publish(&client, &publish);
////    if (i32_rc == MQTT_CODE_SUCCESS)
////			{
////				sprintf(u8_print_buff,"Published: %s \n Topic : %s", "from emcus",MQTT_TOPIC);
////    debug_msg(u8_print_buff);
////    }
////		else
////		{
////			sprintf(u8_print_buff,"Publish failed: %d\n", i32_rc);
////    debug_msg(u8_print_buff);
////    }
//	 i32_rc = MqttClient_Ping(&client);
//     memset(&publish, 0, sizeof(publish));
//					publish.qos = 0;
//					publish.packet_id =4;
//					publish.topic_name = MQTT_TOPIC;
//					publish.buffer = (byte*)char1;
//					publish.total_len = (word16)strlen(char1);
//
//
//    // Step 5: Message processing loop
//    debug_msg("Waiting for messages...\n");
//    while (1) {
//
//		 i32_rc = MqttClient_WaitMessage(&client, 5000); // Wait up to 5 seconds
// i32_rc = MqttClient_Ping(&client);
//			if(1 == u8_one_sec_flag )
//			{
//				u8_one_sec_flag = 0U;
//				i32_rc = MqttClient_Publish(&client, &publish);
//			}
////				 memset(&publish, 0, sizeof(publish));
////					publish.qos = 0;
////					publish.packet_id =4;
////					publish.topic_name = MQTT_TOPIC;
////					publish.buffer = (byte*)char1;
////					publish.total_len = (word16)strlen(char1);

//
//    if (i32_rc == MQTT_CODE_SUCCESS)
//    {
//        // Incoming PUBLISH message will trigger mqtt_message_cb automatically
//    }
//    else if (i32_rc == MQTT_CODE_CONTINUE)
//    {
//        // No message yet, timeout expired
//        debug_msg("\n.");
//			 i32_rc = MqttClient_Ping(&client);
//        continue;
//    }
//    else
//    {
//        sprintf(u8_print_buff, "WaitMessage error: %d\n", i32_rc);
//        debug_msg(u8_print_buff);
//       // break;  // optionally reconnect
//    }
////        i32_rc = MqttClient_WaitMessage(&client,5000);  // wait 5 sec
////    if (i32_rc == MQTT_CODE_SUCCESS)
////    {
////        if (i32_rc == MQTT_CODE_PUB_CONTINUE)
////        {
////           // sprintf(u8_print_buff,"Received message on topic: %s\n", msg.topic_name);
////					 debug_msg("message received\n");
////          // sprintf(u8_print_buff,"Payload: %.*s\n", msg.total_len, msg.buffer);
////
////        }
////    }
////    else if (i32_rc == MQTT_CODE_CONTINUE)
////    {
////			debug_msg("waiting for msg......\n");
////        /* No message yet, timeout elapsed */
////        continue;
////    }
////    else
////    {
////        sprintf(u8_print_buff,"Error waiting for message: %d\n", i32_rc);
////			debug_msg(u8_print_buff);
////        //break;
////    }
//			}
//
//    MqttClient_Disconnect(&client);
//    MqttClient_NetDisconnect(&client);
//
//}

void mqtt_process(void)
{
    /*
    todo:
    */
}
