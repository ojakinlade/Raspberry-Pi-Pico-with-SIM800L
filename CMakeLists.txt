# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(project_name)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)



# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(pico-sim800l
    main.cpp
 )

 add_subdirectory(lib)


# Add pico_stdlib library, add more if used
target_link_libraries(pico-sim800l
    pico_stdlib
    lib 
    hardware_spi
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(pico-sim800l 1)
pico_enable_stdio_uart(pico-sim800l 0)

# Need to generate UF2 file for upload to RP2040
pico_add_extra_outputs(pico-sim800l)