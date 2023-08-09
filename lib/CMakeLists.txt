add_library(lib
    MQTTSim800.cpp
    MQTTSim800.h
    sysTimer.cpp
    sysTimer.h
    MQTTConnect.h
    MQTTConnectClient.c
    MQTTDeserializePublish.c
    MQTTFormat.c
    MQTTFormat.h
    MQTTPacket.c
    MQTTPacket.h
    MQTTPublish.h
    MQTTSerializePublish.c
    MQTTSubscribe.h
    MQTTSubscribeClient.c
    MQTTUnsubscribe.h
    MQTTUnsubscribeClient.c
    StackTrace.h
)

target_link_libraries(lib
    hardware_clocks
    hardware_resets
    hardware_spi
    hardware_dma
    hardware_rtc
    hardware_gpio
    pico_stdlib
)

target_include_directories(lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")