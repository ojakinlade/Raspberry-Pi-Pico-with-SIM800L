#ifndef MQTTSIM800_H
#define MQTTSIM800_H

#define SIM800_UART     uart1
#define SIM800_TX       8
#define SIM800_RX       9
#define UART_DATA_BITS  8
#define UART_STOP_BITS  1
#define CMD_DELAY       2000

typedef struct
{
    char* apn;
    char* apn_user;
    char* apn_pass;
}sim_t;

typedef struct 
{
    char* host;
    uint16_t port;
    uint8_t connect;
}mqttServer_t;

typedef struct 
{
    char* username;
    char* pass;
    char* clientID;
    unsigned short keepAliveInterval;
}mqttClient_t;

typedef struct 
{
    uint8_t newEvent;
    unsigned char dup;
    int qos;
    unsigned char retained;
    unsigned short msgId;
    unsigned char payload[64];
    int payloadLen;
    unsigned char topic[64];
    int topicLen;
}mqttReceive_t;

typedef struct 
{
    sim_t sim;
    mqttServer_t mqttServer;
    mqttClient_t mqttClient;
    mqttReceive_t mqttReceive;
}SIM800_t;

extern void SIM800_UART_Init(void);
extern void SIM800_RxCallback(void);
extern void ClearRxBuffer(void);
extern void ClearMqttBuffer(void);
extern int SIM800_SendCommand(char* command, char* reply, uint16_t delay);
// extern int SIM800_SendCommand(char* command, char* reply);
extern int MQTT_Init(void);
extern void MQTT_Connect(void);
extern void MQTT_Pub(char* topic, char* payload);
extern void MQTT_PubUint8(char* topic, uint8_t data);
extern void MQTT_PubUint16(char *topic, uint16_t data);
extern void MQTT_PubUint32(char *topic, uint32_t data);
extern void MQTT_PubFloat(char *topic, float payload);
extern void MQTT_PubDouble(char *topic, double data);
extern void MQTT_PingReq(void);
extern void MQTT_Sub(char *topic);
extern void MQTT_Receive(unsigned char *buf);

#endif /* MQTTSIM800_H */


