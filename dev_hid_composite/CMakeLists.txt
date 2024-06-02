cmake_minimum_required(VERSION 3.13)

# Create the library
add_library(dev_hid_composite STATIC
    usb_descriptors.c
)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(dev_hid_composite PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# Add dependencies
target_link_libraries(dev_hid_composite PUBLIC 
    pico_stdlib 
    pico_unique_id 
    tinyusb_device 
    tinyusb_board
)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
# target_compile_definitions(dev_hid_composite PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

# Remove this line since it's for executables
# pico_add_extra_outputs(dev_hid_composite)

# Commented out since it's not recognized
# example_auto_set_url(dev_hid_composite)
