cmake_minimum_required(VERSION 3.13)
# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

project(MacroBardo)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Add compilation subdirectories
add_subdirectory(WaveshareLib/Config)
add_subdirectory(WaveshareLib/e-Paper)
add_subdirectory(WaveshareLib/Fonts)
add_subdirectory(WaveshareLib/GUI)


# Add header directories
include_directories(./WaveshareLib/Config)
include_directories(./WaveshareLib/GUI)

# Generate an executable file
add_executable(${PROJECT_NAME} 
    main.c
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})

# Link the libraries
target_link_libraries(${PROJECT_NAME} 
    ePaper 
    GUI 
    Fonts 
    Config 
    pico_stdlib 
    pico_unique_id 
    hardware_gpio 
    hardware_spi 
    hardware_dma 
    hardware_pio 
    hardware_irq
)