/**
 * @file message_client.c
 * @author shameem
 * @brief 
 * @date 2025-10-30
 * @copyright Copyright (c) 2025
 */

/************************************************************* Header includes ***************************************************************/
#include "https_client_app.h"
#include "mqtt_client.h"
#include "mqtt_socket.h"
#include "message_client.h"


/***************************************************************** Macros ********************************************************************/
#define MQTT_HOST       "broker.hivemq.com"
#define MQTT_PORT       1883
#define MQTT_TOPIC      "stm32/test"
#define MQTT_MESSAGE    "Hello from STM32!"
#define MQTT_CMD_TIMEOUT_MS    5000U

/******************************************************************* Variables ******************************************************************/
uint8_t print_buff[150];
static unsigned char txtx_buf[0x4000];
static unsigned char rxrx_buf[0x4000];

/******************************************************************* function definitions ******************************************************************/

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
