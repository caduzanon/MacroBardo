#include "WaveshareLib/Config/DEV_Config.h"
#include "WaveshareLib/GUI/GUI_Paint.h"
#include "WaveshareLib/e-Paper/ImageData.h"
#include "WaveshareLib/e-Paper/EPD_2in9_V2.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdlib.h>

#define ENC_A 11
#define ENC_B 12
#define NUM_BUTTONS 10

const uint32_t BUTTON_PINS[NUM_BUTTONS] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 13};

volatile bool encoder_rotated = false;
volatile bool clockwise = false;

void encoder_callback(uint gpio, uint32_t events) {
    static uint8_t last_state = 0;
    uint8_t current_state = (gpio_get(ENC_A) << 1) | gpio_get(ENC_B);

    // Check for clockwise rotation
    if ((last_state == 0b00 && current_state == 0b01) ||
        (last_state == 0b01 && current_state == 0b11) ||
        (last_state == 0b11 && current_state == 0b10) ||
        (last_state == 0b10 && current_state == 0b00)) {
        clockwise = true;
        encoder_rotated = true;
    }
    // Check for counter-clockwise rotation
    else if ((last_state == 0b00 && current_state == 0b10) ||
             (last_state == 0b10 && current_state == 0b11) ||
             (last_state == 0b11 && current_state == 0b01) ||
             (last_state == 0b01 && current_state == 0b00)) {
        clockwise = false;
        encoder_rotated = true;
    }

    last_state = current_state;
}

void init_encoder_interrupts(void) {
    gpio_set_irq_enabled_with_callback(ENC_A, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &encoder_callback);
    gpio_set_irq_enabled_with_callback(ENC_B, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &encoder_callback);
}

void init_buttons(void) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_init(BUTTON_PINS[i]);
        gpio_set_dir(BUTTON_PINS[i], GPIO_IN);
        gpio_pull_up(BUTTON_PINS[i]);
    }
}

void init_display(void) {
    if (DEV_Module_Init() != 0) {
        printf("Failed to initialize module\r\n");
        return;
    }
    printf("e-Paper Init and Clear...\r\n");
    EPD_2IN9_V2_Init();
    EPD_2IN9_V2_Clear();
}

void print_direction_on_display(bool clockwise) {
    // Print "CW" or "CCW" depending on the direction
    if (clockwise) {
        printf("CW\n");
    } else {
        printf("CCW\n");
    }
}

int main() {
    stdio_init_all();

    // Initialize encoder interrupts
    init_encoder_interrupts();

    // Initialize buttons with pull-up resistors
    init_buttons();

    // Initialize display
    init_display();

    while (1) {
        if (encoder_rotated) {
            // Print the direction on the display
            print_direction_on_display(clockwise);
            encoder_rotated = false;
        }

        // Check button states
        for (int i = 0; i < NUM_BUTTONS; i++) {
            if (!gpio_get(BUTTON_PINS[i])) {
                printf("Button %u pressed\n", BUTTON_PINS[i]);
            }
        }

        sleep_ms(100);
    }
    return 0;
}
