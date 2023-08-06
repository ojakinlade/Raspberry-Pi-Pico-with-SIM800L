#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "sim800l.h"

#define GSM_UART    uart0
#define BAUD_RATE   9600
#define UART_TX_PIN 16
#define UART_RX_PIN 17

#define DATA_BITS   8
#define STOP_BITS   1



uint8_t mqttMessage[127];
// const char APN[] = "web.gprs.mtnnigeria.net\r\n";
const char APN[] = "gloflat\r\n";

static void mqtt_connect_message(uint8_t* mqtt_message, const char* client_id)
{
    uint8_t client_id_len = strlen(client_id);

    mqtt_message[0] = 16;                      // MQTT Message Type CONNECT
    mqtt_message[1] = 14 + client_id_len;      // Remaining length of the message
    mqtt_message[2] = 0;                       // Protocol Name Length MSB
    mqtt_message[3] = 6;                       // Protocol Name Length LSB
    mqtt_message[4] = 77;                      // ASCII Code for M
    mqtt_message[5] = 81;                      // ASCII Code for Q
    mqtt_message[6] = 73;                      // ASCII Code for I
    mqtt_message[7] = 115;                     // ASCII Code for s
    mqtt_message[8] = 100;                     // ASCII Code for d
    mqtt_message[9] = 112;                     // ASCII Code for p
    mqtt_message[10] = 3;                      // MQTT Protocol version = 3
    mqtt_message[11] = 2;                      // conn flags
    mqtt_message[12] = 0;                      // Keep-alive Time Length MSB
    mqtt_message[13] = 15;                     // Keep-alive Time Length LSB
    mqtt_message[14] = 0;                      // Client ID length MSB
    mqtt_message[15] = client_id_len;       // Client ID length LSB
    // Client ID
    for(uint8_t i = 0; i < client_id_len + 16; i++){
        mqtt_message[16 + i] = client_id[i];
    }
}

static void mqtt_publish_message(uint8_t* mqtt_message, const char* topic, const char* message)
{
    uint8_t topic_len = strlen(topic);
    uint8_t msg_len = strlen(message);

    mqtt_message[0] = 48;                                  // MQTT Message Type CONNECT
    mqtt_message[1] = 2 + topic_len + msg_len;             // Remaining length
    mqtt_message[2] = 0;                                   // MQTT Message Type CONNECT
    mqtt_message[3] = topic_len;                           // MQTT Message Type CONNECT

    // Topic
    for(uint8_t i = 0; i < topic_len; i++){
        mqtt_message[4 + i] = topic[i];
    }
    // Message
    for(uint8_t i = 0; i < msg_len; i++){
        mqtt_message[4 + topic_len + i] = message[i];
    }
}

static void mqtt_disconnect_message(uint8_t* mqtt_message)
{
    mqtt_message[0] = 0xE0; // msgtype = connect
    mqtt_message[1] = 0x00; // length of message (?)
}

static void SIM800L_println(const char* pData)
{
    uart_puts(GSM_UART, pData);
    printf("%s", pData);
    while(uart_is_readable(GSM_UART))
    {
        char inChar = uart_getc(GSM_UART);
        printf("%c",inChar);
    }
}

void SIM800L_Init(void)
{
    uart_init(GSM_UART, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(GSM_UART,false,false); //Disable hardware flow control
    uart_set_format(GSM_UART,DATA_BITS,STOP_BITS,UART_PARITY_NONE);
    uart_set_fifo_enabled(GSM_UART,true);
}

void sendMQTTMessage(const char* clientId, const char* topic, const char* message)
{
    SIM800L_println("AT\r\n");
    sleep_ms(1000);
    SIM800L_println("AT+CIPMUX=0\r\n");
    sleep_ms(2000);
    char apnStr[128] = "AT+CSTT=";
    strcat(apnStr, APN);
    SIM800L_println(apnStr);
    sleep_ms(5000);
    SIM800L_println("AT+CIICR\r\n");
    sleep_ms(3000);
    SIM800L_println("AT+CIFSR\r\n");
    sleep_ms(2000);
    SIM800L_println("AT+CIPSPRT=0\r\n");
    sleep_ms(3000);
    SIM800L_println("AT+CIPSTART=\"TCP\",\"broker.hivemq.com\",\"1883\"\r\n");
    sleep_ms(6000);
    SIM800L_println("AT+CIPSEND\r\n");
    sleep_ms(4000);
    int mqttMsgLen = 16 + strlen(clientId);
    mqtt_connect_message(mqttMessage,clientId);
    for(int j = 0; j < mqttMsgLen; j++)
    {
        uart_putc(GSM_UART, mqttMessage[j]);
    }
    uart_putc(GSM_UART, (uint8_t)26); //(signals end of message)
    sleep_ms(1000);
    SIM800L_println("AT+CIPSEND\r\n");
    sleep_ms(5000);
    mqttMsgLen = 6 + strlen(topic) + strlen(message);
    mqtt_publish_message(mqttMessage,topic,message);
    for(int k = 0; k < mqttMsgLen; k++)
    {
        uart_putc(GSM_UART, mqttMessage[k]);
    }
    uart_putc(GSM_UART, (uint8_t)26); //(signals end of message)
    sleep_ms(2000);
    SIM800L_println("AT+CIPCLOSE\r\n");
    sleep_ms(1000);
}