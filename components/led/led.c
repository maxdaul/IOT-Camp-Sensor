#include "led.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define RGB_RED_GPIO     25
#define RGB_GREEN_GPIO   26
#define RGB_BLUE_GPIO    27

#define LEDC_TIMER       LEDC_TIMER_0
#define LEDC_MODE        LEDC_LOW_SPEED_MODE
#define LEDC_FREQUENCY   5000
#define LEDC_RESOLUTION  LEDC_TIMER_8_BIT

static const char *TAG = "RGB_LED";

void led_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_RESOLUTION,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t channels[] = {
        { .gpio_num = RGB_RED_GPIO,   .channel = LEDC_CHANNEL_0, .timer_sel = LEDC_TIMER, .speed_mode = LEDC_MODE, .duty = 0, .intr_type = LEDC_INTR_DISABLE, .hpoint = 0 },
        { .gpio_num = RGB_GREEN_GPIO, .channel = LEDC_CHANNEL_1, .timer_sel = LEDC_TIMER, .speed_mode = LEDC_MODE, .duty = 0, .intr_type = LEDC_INTR_DISABLE, .hpoint = 0 },
        { .gpio_num = RGB_BLUE_GPIO,  .channel = LEDC_CHANNEL_2, .timer_sel = LEDC_TIMER, .speed_mode = LEDC_MODE, .duty = 0, .intr_type = LEDC_INTR_DISABLE, .hpoint = 0 }
    };
    for (int i = 0; i < 3; ++i) {
        ESP_ERROR_CHECK(ledc_channel_config(&channels[i]));
    }
    
    ESP_LOGI(TAG, "RGB LED инициализирован");
}

void led_set_color(uint8_t red, uint8_t green, uint8_t blue) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, red);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, green);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_2, blue);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_2);
}

void led_set_red(void) { led_set_color(255, 0, 0); }
void led_set_green(void) { led_set_color(0, 255, 0); }
void led_set_blue(void) { led_set_color(0, 0, 255); }
void led_set_yellow(void) { led_set_color(255, 255, 0); }
void led_off(void) { led_set_color(0, 0, 0); }