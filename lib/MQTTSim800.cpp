#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <string.h>
#include <stdio.h>
#include "MQTTPacket.h"
#include "MQTTSim800.h"

extern SIM800_t SIM800;

uint8_t rx_data = 0;
uint8_t rx_buffer[1460] = {0};
uint16_t rx_index = 0;

uint8_t mqtt_receive = 0;
char mqtt_buffer[1460] = {0};
uint16_t mqtt_index = 0;

bool uartDoneReceiving = false;

/**
 * @brief UART1 Interrupt Handler
 * 
 */
static void SIM800_UART_RX(uint8_t* rxData)
{
    if(uart_is_readable(SIM800_UART))
    {
        *rxData = uart_getc(SIM800_UART);
        uartDoneReceiving = true;
    }
}

void UART1_IRQHandler(void)
{
     SIM800_UART_RX(&rx_data);
     if(uartDoneReceiving)
     {
        uartDoneReceiving = false;
        SIM800_RxCallback();
     }
}

static void SIM800_UART_TX(char* command)
{
    uart_puts(SIM800_UART, command);
    //printf(command);
}

/**
 * @brief Configures SIM800 UART Port
 * @param NONE
 * @return NONE
 * 
 */
void SIM800_UART_Init(void)
{
    uart_init(SIM800_UART,9600);
    gpio_set_function(SIM800_TX, GPIO_FUNC_UART);
    gpio_set_function(SIM800_RX, GPIO_FUNC_UART);
    uart_set_hw_flow(SIM800_UART, false, false);
    uart_set_format(SIM800_UART,UART_DATA_BITS,UART_STOP_BITS,UART_PARITY_NONE);
    uart_set_fifo_enabled(SIM800_UART, true);
    //RX Interrupt Init
    irq_set_exclusive_handler(UART1_IRQ, UART1_IRQHandler);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(SIM800_UART, true, false);
}

/**
 * @brief Calback function for release read SIM800 UART buffer.
 * @param NONE
 * @param NONE
 * 
 */
void SIM800_RxCallback(void)
{
    rx_buffer[rx_index++] = rx_data;

    if(SIM800.mqttServer.connect == 0)
    {
        if(strstr((char*)rx_buffer, "\r\n") != NULL && rx_index == 2)
        {
            rx_index = 0;
        }
        else if(strstr((char*)rx_buffer, "\r\n") != NULL)
        {
            memcpy(mqtt_buffer, rx_buffer, sizeof(rx_buffer));
            ClearRxBuffer();
            if(strstr(mqtt_buffer, "DY CONNECT\r\n"))
            {
                SIM800.mqttServer.connect = 0;
            }
            else if(strstr(mqtt_buffer, "CONNECT\r\n"))
            {
                SIM800.mqttServer.connect = 1;
            }
        }
    }
    if(strstr((char*)rx_buffer, "CLOSED\r\n") || strstr((char*)rx_buffer, "ERROR\r\n") || strstr((char*)rx_buffer, "DEACT\r\n"))
    {
        SIM800.mqttServer.connect = 0;
    } 
    if(SIM800.mqttServer.connect == 1 && rx_data == 48)
    {
        mqtt_receive = 1;
    }  
    if(mqtt_receive == 1)
    {
        mqtt_buffer[mqtt_index++] = rx_data;
        if(mqtt_index > 1 && mqtt_index - 1 > mqtt_buffer[1])
        {
            MQTT_Receive((unsigned char*)mqtt_buffer);
            ClearRxBuffer();
            ClearMqttBuffer();
        }
        if(mqtt_index >= sizeof(mqtt_buffer))
        {
            ClearMqttBuffer();
        }
    }
    if(rx_index >= sizeof(mqtt_buffer))
    {
        ClearRxBuffer();
        ClearMqttBuffer();
    }
}

/**
 * @brief Clears SIM800 UART RX buffer
 * @param NONE
 * @return NONE
 * 
 */
void ClearRxBuffer(void)
{
    rx_index = 0;
    memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
 * @brief Clears MQTT buffer
 * @param NONE
 * @return NONE
 */
void ClearMqttBuffer(void)
{
    mqtt_receive = 0;
    mqtt_index = 0;
    memset(mqtt_buffer, 0, sizeof(mqtt_buffer));
}

int SIM800_SendCommand(char* command, char* reply, uint16_t delay)
{
    SIM800_UART_TX(command);
    sleep_ms(delay);
    if(strstr(mqtt_buffer, reply) != NULL)
    {
        ClearMqttBuffer();
        return 0;
    }
    ClearMqttBuffer();
    return 1;
}

/**
 * @brief SIM800L Initialization
 * @param NONE
 * @return NONE
 */
int MQTT_Init(void)
{
    SIM800.mqttServer.connect = 0;
    int error = 0;
    char str[32] = {0};
    //Receive a byte ????????

    error += SIM800_SendCommand("AT\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("ATE0\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CIPSHUT\r\n", "SHUT OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CGATT=1\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CIPMODE=1\r\n", "OK\r\n", CMD_DELAY);

    snprintf(str, sizeof(str), "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", SIM800.sim.apn, SIM800.sim.apn_user,
             SIM800.sim.apn_pass);
    error += SIM800_SendCommand(str, "OK\r\n", CMD_DELAY);

    error += SIM800_SendCommand("AT+CIICR\r\n", "OK\r\n", CMD_DELAY);
    SIM800_SendCommand("AT+CIFSR\r\n", "", CMD_DELAY);
    if(error == 0)
    {
        MQTT_Connect();
        return error;
    }
    return error;
}

/**
 * @brief Connect to MQTT server over TCP.
 * @param NONE
 * @return NONE
 */
void MQTT_Connect(void)
{
    SIM800.mqttReceive.newEvent = 0;
    SIM800.mqttServer.connect = 0;
    char str[128] = {0};
    unsigned char buf[128] = {0};
    sprintf(str, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SIM800.mqttServer.host, SIM800.mqttServer.port);
    SIM800_SendCommand(str, "OK\r\n", CMD_DELAY);
    sleep_ms(5000);
    if(SIM800.mqttServer.connect == 1)
    {
        MQTTPacket_connectData datas = MQTTPacket_connectData_initializer;
        datas.username.cstring = SIM800.mqttClient.username;
        datas.password.cstring = SIM800.mqttClient.pass;
        datas.clientID.cstring = SIM800.mqttClient.clientID;
        datas.keepAliveInterval = SIM800.mqttClient.keepAliveInterval;
        datas.cleansession = 1;
        int mqtt_len = MQTTSerialize_connect(buf, sizeof(buf), &datas);
        SIM800_UART_TX((char*)buf);
        sleep_ms(5000);
    }
}

/**
 * @brief Publish a string in a topic on the MQTT broker
 * 
 * @param topic Topic to be published to
 * @param payload  Message to be sent to the topic
 */
void MQTT_Pub(char* topic, char* payload)
{
    unsigned char buf[256] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;
    int mqtt_len = MQTTSerialize_publish(buf, sizeof(buf), 0, 0, 0, 0,
                                         topicString, (unsigned char *)payload, (int)strlen(payload));
    SIM800_UART_TX((char*)buf);
    sleep_ms(100);
}

/**
 * @brief Publish a uin8_t data in a topic on the MQTT broker
 * 
 * @param topic Topic to published to
 * @param payload uint8_t data to be sent to the topic 
 */
void MQTT_PubUint8(char* topic, uint8_t payload)
{
    char str[32] = {0};
    sprintf(str,"%u",payload);
    MQTT_Pub(topic, str);
}

/**
 * @brief Publish a uint16_t data in a topic on the MQTT broker
 * 
 * @param topic Topic to be published to
 * @param payload uint16_t data to be sent to the topic
 */
void MQTT_PubUint16(char* topic, uint16_t payload)
{
    char str[32] = {0};
    sprintf(str,"%u",payload);
    MQTT_Pub(topic, str);
}

/**
 * @brief Publish a uint32_t data in a topic on the MQTT broker
 * 
 * @param topic Topic to be published to
 * @param payload uint16_t data to be sent to the topic
 */
void MQTT_PubUint32(char* topic, uint32_t payload)
{
    char str[32] = {0};
    sprintf(str,"%lu",payload);
    MQTT_Pub(topic, str);
}

/**
 * @brief Publish a float data type in a topic on the MQTT broker
 * 
 * @param topic Topic to be published to
 * @param payload Data to be sent to the topic
 */
void MQTT_PubFloat(char* topic, float payload)
{
    char str[32] = {0};
    sprintf(str,"%f",payload);
    MQTT_Pub(topic, str);
}

/**
 * @brief Publish a double data type in a topic on the MQTT broker
 * 
 * @param topic Topic to be published to
 * @param payload Data to be sent to the topic
 */
void MQTT_PubDouble(char* topic, double payload)
{
    char str[32] = {0};
    sprintf(str,"%f",payload);
    MQTT_Pub(topic, str);
}

/**
 * Send a PINGREQ to the MQTT broker (active session)
 * @param NONE
 * @return NONE
 */
void MQTT_PingReq(void)
{
    unsigned char buf[16] = {0};

    int mqtt_len = MQTTSerialize_pingreq(buf, sizeof(buf));
    SIM800_UART_TX((char*)buf);
}

/**
 * @brief Subscribe to a topic on the mqtt broker
 * 
 * @param topic Topic to subscibe to
 */
void MQTT_Sub(char* topic)
{
    unsigned char buf[256] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;

    int mqtt_len = MQTTSerialize_subscribe(buf, sizeof(buf), 0, 1, 1,
                                           &topicString, 0);
    SIM800_UART_TX((char*)buf); //This function should be modified to include the strlen as param                                    
    sleep_ms(100);
}

/**
 * @brief Receive a message from the MQTT broker
 * 
 * @param buf  MQTT message receive buffer
 */
void MQTT_Receive(unsigned char *buf)
{
    memset(SIM800.mqttReceive.topic, 0, sizeof(SIM800.mqttReceive.topic));
    memset(SIM800.mqttReceive.payload, 0, sizeof(SIM800.mqttReceive.payload));
    MQTTString receivedTopic;
    unsigned char *payload;
    MQTTDeserialize_publish(&SIM800.mqttReceive.dup, &SIM800.mqttReceive.qos, &SIM800.mqttReceive.retained,
                            &SIM800.mqttReceive.msgId,
                            &receivedTopic, &payload, &SIM800.mqttReceive.payloadLen, buf,
                            sizeof(buf));
    memcpy(SIM800.mqttReceive.topic, receivedTopic.lenstring.data, receivedTopic.lenstring.len);
    SIM800.mqttReceive.topicLen = receivedTopic.lenstring.len;
    memcpy(SIM800.mqttReceive.payload, payload, SIM800.mqttReceive.payloadLen);
    SIM800.mqttReceive.newEvent = 1;
}

