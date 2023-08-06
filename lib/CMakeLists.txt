add_library(lib
    sim800l.cpp
    sim800l.h
    sysTimer.cpp
    sysTimer.h
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