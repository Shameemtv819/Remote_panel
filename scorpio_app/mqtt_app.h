/**
 * @file mqtt_client.h
 * @author shameem
 * @brief 
 * @date 2025-11-20
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef MQTT_APP_H
#define MQTT_APP_H
/******************************************************** Header includes **********************************************************/
#include "scorpio_app.h"
/*********************************************************** Macros ****************************************************************/

/*********************************************************** typedef ***************************************************************/
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
/*********************************************************** enum ****************************************************************/
/*Modes in mqtt operation */
enum mqtt_mode
{
    MQTT_INIT,
    MQTT_ROUTINE_OPERATION,
};
/******************************************************** Extern variables *********************************************************/
extern uint8_t u8_mqtt_state;
extern EventLog_t fire;
/******************************************************* function prototypes ******************************************************/
char mqtt_task(void);
int mqtt_notify_ota(void);
void mqtt_clear_buffers(void);
int mqtt_publish_fire_fault(uint8_t *payload, uint16_t payload_len);

#endif /* MQTT_APP_H */