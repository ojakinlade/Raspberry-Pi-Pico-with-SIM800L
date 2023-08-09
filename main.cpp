#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "MQTTSim800.h"
#include "sysTimer.h"

sysTimer_t ledTimer;
sysTimer uploadTimer;
SIM800_t SIM800;

const char* mqttPubTopic = "powerstatus/data";
const char* mqttSubTopic = "powerstatus/test";
const char* mqttData = "hello there!";

int main()
{
    stdio_init_all();
    SysTimer_Init(&ledTimer,1000);
    SysTimer_Init(&uploadTimer,20000);
    SIM800_UART_Init();

    //MQTT Settings
    SIM800.sim.apn = "fast.m2m";
    SIM800.sim.apn_user = "";
    SIM800.sim.apn_pass = "";
    SIM800.mqttServer.host = "broker.hivemq.com";
    SIM800.mqttServer.port = 1883;
    SIM800.mqttClient.username = "";
    SIM800.mqttClient.pass = "";
    SIM800.mqttClient.clientID = "powerLabs";
    SIM800.mqttClient.keepAliveInterval = 120;

    int ret = MQTT_Init();
    printf("%d",ret);

    uint8_t sub = 0;

    //Test data
    uint8_t pub_uint8 = 1;
    uint16_t pub_uint16 = 2;
    uint32_t pub_uint32 = 3;
    float pub_float = 1.1;
    double pub_double = 2.2;

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);

    while(true)
    {
        ret = MQTT_Init();
        printf("Error code: ");
        printf("%d\n",ret);
        // int ret = SIM800_SendCommand("AT\r\n", "OK");
        // printf("Return value: ");
        // printf("%d\n", ret);
        // uart_puts(uart1,"AT\r\n");
        // while(uart_is_readable(uart1))
        // {
        //     char c = uart_getc(uart1);
        //     printf("%c",c);
        // }
        sleep_ms(5000);

        // if(SIM800.mqttServer.connect == 0)
        // {
        //     MQTT_Init();
        //     sub = 0;
        // }
        // if(SIM800.mqttServer.connect == 1)
        // {
        //     if(sub = 0)
        //     {
        //         MQTT_Sub((char*)mqttSubTopic);
        //         sub = 1;
        //     }
        //     MQTT_Pub((char*)mqttPubTopic,"string");
        //     MQTT_PubUint8((char*)mqttPubTopic, pub_uint8);
        //     MQTT_PubUint16((char*)mqttPubTopic, pub_uint16);
        //     MQTT_PubUint32((char*)mqttPubTopic, pub_uint32);
        //     MQTT_PubFloat((char*)mqttPubTopic, pub_float);
        //     MQTT_PubDouble((char*)mqttPubTopic, pub_double);

        //     if(SIM800.mqttReceive.newEvent)
        //     {
        //         unsigned char* topic = SIM800.mqttReceive.topic;
        //         int payload = atoi((char*)SIM800.mqttReceive.payload);
        //         SIM800.mqttReceive.newEvent = 0;
        //     }
        // }
        // sleep_ms(1000);
    }
}