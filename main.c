#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdlib.h>
#include "usb_descriptors.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "WaveshareLib/Config/DEV_Config.h"
#include "WaveshareLib/GUI/GUI_Paint.h"
#include "WaveshareLib/e-Paper/ImageData.h"
#include "WaveshareLib/e-Paper/EPD_2in9_V2.h"

#define LED_PIN 25
#define NUM_KEYS 6

#define ENC_A 11
#define ENC_B 12
#define NUM_BUTTONS 10
#define FLAG_VALUE 123

const uint32_t BUTTON_PINS[NUM_BUTTONS] = {1, 4, 7, 2, 5, 8, 3, 6, 9, 13};
uint8_t empty_keycode[NUM_KEYS] = {0};

uint8_t keycodes[12][NUM_KEYS] = {
    {HID_KEY_Q, 0, 0, 0, 0, 0}, // Button 1 sends 'Q'
    {HID_KEY_W, 0, 0, 0, 0, 0}, // Button 2 sends 'W'
    {HID_KEY_E, 0, 0, 0, 0, 0}, // Button 3 sends 'E'
    {HID_KEY_A, 0, 0, 0, 0, 0}, // Button 4 sends 'A'
    {HID_KEY_S, 0, 0, 0, 0, 0}, // Button 5 sends 'S'
    {HID_KEY_D, 0, 0, 0, 0, 0}, // Button 6 sends 'D'
    {HID_KEY_F, 0, 0, 0, 0, 0}, // Button 7 sends 'F'
    {HID_KEY_ALT_LEFT, HID_KEY_F4, 0, 0, 0, 0}, // Button 8 sends 'Ctrl' + 'C'
    {HID_KEY_ALT_LEFT, HID_KEY_TAB, 0, 0, 0, 0}, // Button 9 sends 'V'
    {HID_USAGE_CONSUMER_PLAY_PAUSE, 0, 0, 0, 0, 0}, // Button 10 sends 'Play/Pause'
    {HID_USAGE_CONSUMER_VOLUME_INCREMENT, 0, 0, 0, 0, 0},   //Button 11 sends 
    {HID_USAGE_CONSUMER_VOLUME_DECREMENT, 0, 0, 0, 0, 0}
};

const char* textos[3] = {
    " Q     W     E    Pause",
    " A     S     D    Vol +",
    " F   AltF4 AltTab Vol -"
};

volatile bool encoder_rotated = false;
volatile bool clockwise = false;

void draw_interface(void);

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

void encoder_action(bool clockwise) {
    // Print "CW" or "CCW" depending on the direction
    if (clockwise) {
        printf("CW\n");
        
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, keycodes[10], NUM_KEYS); //manda a bomba
        
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, empty_keycode, NUM_KEYS); //desmanda a bomba
    } else {
        printf("CCW\n");
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, keycodes[11], NUM_KEYS); //manda a bomba
        
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, empty_keycode, NUM_KEYS); //desmanda a bomba
    }
}

void core1_entry() {
    multicore_fifo_push_blocking(FLAG_VALUE);
    uint32_t g = multicore_fifo_pop_blocking();

    if (g != FLAG_VALUE) printf("Error!\n");
    else printf("We are good!");
    while (1) tud_task(); // tinyusb device task
}

void init_display(void) {
    if (DEV_Module_Init() != 0) {
        printf("Failed to initialize module\r\n");
        return;
    }
    printf("e-Paper Init and Clear...\r\n");
    EPD_2IN9_V2_Init();
    EPD_2IN9_V2_Clear();
    draw_interface();  // Chama a função que desenha a interface
    EPD_2IN9_V2_Sleep();
}

int main() {
    for (int i=0; i<NUM_BUTTONS; i++){
        gpio_init(BUTTON_PINS[i]);
        gpio_set_dir(BUTTON_PINS[i], GPIO_IN);
        gpio_pull_up(BUTTON_PINS[i]);
    }    
    stdio_init_all();
    board_init();
    tusb_init();
    // Initialize encoder interrupts
    init_encoder_interrupts();
    // Initialize display
    init_display();

    multicore_launch_core1(core1_entry);

    // Wait for it to start up

    uint32_t g = multicore_fifo_pop_blocking();

    if (g != FLAG_VALUE)
        printf("Hmm, that's not right on core 0!\n");
    else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("It's all gone well on core 0!");
    }

    bool button_state[NUM_BUTTONS] = {false};
    bool last_button_state[NUM_BUTTONS] = {false};
    bool key_sent[NUM_BUTTONS] = {false};

    while (1)
    {
        if (encoder_rotated) {
            // Print the direction on the display
            encoder_action(clockwise);
            encoder_rotated = false;
        }

        for (int i=0; i<NUM_BUTTONS; i++){
            button_state[i] = gpio_get(BUTTON_PINS[i]) == 0;
            if (button_state[i] != last_button_state[i]){
                if (button_state[i] && !key_sent[i]){
                    if (i==9){
                        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, keycodes[9], NUM_KEYS);
                    }
                    else{
                        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycodes[i]);
                    }
                    key_sent[i] = true;
                }
                else if(!button_state[i] && key_sent[i]){
                    if (i==9){
                        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, empty_keycode, NUM_KEYS);
                        tud_hid_keyboard_report(REPORT_ID_CONSUMER_CONTROL, 0, empty_keycode);
                    }
                    else{
                        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, empty_keycode);
                    }
                    key_sent[i] = false;
                }
            }
            last_button_state[i] = button_state[i];
        }
    }
    return 0;
}

void draw_interface() {
    UBYTE *BlackImage;
    UWORD Imagesize = ((EPD_2IN9_V2_WIDTH % 8 == 0)? (EPD_2IN9_V2_WIDTH / 8 ): (EPD_2IN9_V2_WIDTH / 8 + 1)) * EPD_2IN9_V2_HEIGHT;//* 2;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return;
    }
    //printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN9_V2_WIDTH, EPD_2IN9_V2_HEIGHT, 90, WHITE);
    //Paint_SelectImage(BlackImage);
    Paint_SetScale(2); // b&w
    Paint_Clear(WHITE);

    // Desenhar grade com 9 botões no lado esquerdo
    int i, j;
    for (i = 0; i < 3; i++) {
            //Paint_DrawLine(10 + j*45, 10 + i *35, 50 + j* 40, 50 + i*40, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
            //Paint_DrawRectangle(10 + j * 45, 10 + i * 35, 50 + j * 40, 50 + i * 40, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        Paint_DrawString_EN(5 + Font16.Width, 7 + i * 35 + Font16.Height, textos[i], &Font16, WHITE, BLACK); // Exemplo de texto no botão
    }
    int finalsize_x = 7 + i*35 + (Font16.Height*i);
    int finalsize_y = 7 + i*35 + (Font16.Height*i);
    //paint horizontals:
    Paint_DrawLine(0, 30 + Font16.Height, 296, 30 + Font16.Height, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    Paint_DrawLine(0, 65 + Font16.Height, 296, 65 + Font16.Height, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    //paint verticals:
    Paint_DrawLine(0, 0, 0,128, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    Paint_DrawLine(11 + 5* Font16.Width, 0, 11 + 5* Font16.Width, 128, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    Paint_DrawLine(12 + 11* Font16.Width, 0, 12 + 11* Font16.Width, 128, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    Paint_DrawLine(10 + 18* Font16.Width, 0, 10 + 18* Font16.Width, 128, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    //Paint_DrawLine(100 + 8*Font16.Width, 0, 100 + 8*Font16.Width, 128, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    /* Desenhar sinal de play/pause na parte inferior direita
    Paint_DrawRectangle(180, 200, 210, 230, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
    Paint_DrawString_EN(185, 210, ">", &Font20, WHITE, BLACK); // Exemplo de sinal de play
    

    // Desenhar "MB" acima do sinal de play/pause
    Paint_DrawString_EN(185, 190, "MB", &Font20, WHITE, BLACK);
    */

    EPD_2IN9_V2_Display(BlackImage);
    free(BlackImage);
}