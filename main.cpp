#include <stdio.h>
#include "pico/stdlib.h"
#include "sim800l.h"
#include "sysTimer.h"

sysTimer_t ledTimer;
sysTimer uploadTimer;

const char* deviceName = "powerLabs";
const char* mqttTopic = "power-status";
const char* mqttData = "hello there!";

int main()
{
    stdio_init_all();
    SysTimer_Init(&ledTimer,1000);
    SysTimer_Init(&uploadTimer,20000);
    SIM800L_Init();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);

    while(true)
    {
        if(SysTimer_DoneCounting(&ledTimer))
        {
            gpio_put(PICO_DEFAULT_LED_PIN,!gpio_get(PICO_DEFAULT_LED_PIN));
        }
        if(SysTimer_DoneCounting(&uploadTimer))
        {
            sendMQTTMessage(deviceName,mqttTopic,mqttData);
        }
    }
}