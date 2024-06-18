cmake_minimum_required(VERSION 3.13)
# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

project(MacroBardo C CXX ASM)

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
    usb_descriptors.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${PICO_SDK_PATH}/lib/tinyusb/src
)
# Define the TinyUSB configuration
target_compile_definitions(${PROJECT_NAME} PUBLIC
    PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=5000
    CFG_TUSB_MCU=OPT_MCU_RP2040
    BOARD_TUD_RHPORT=0
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_STDIO_USB=1
    TINYUSB_CDC=1
    TINYUSB_HID=1
)

# Link the libraries
target_link_libraries(${PROJECT_NAME} 
    ePaper 
    GUI 
    Fonts 
    Config 
    pico_stdlib 
    pico_unique_id 
    hardware_gpio 
    hardware_uart
    hardware_spi 
    hardware_dma 
    hardware_pio 
    hardware_irq
    tinyusb_device
    tinyusb_board
    pico_multicore
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})