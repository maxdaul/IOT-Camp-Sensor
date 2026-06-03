#include "buzzer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define BUZZER_GPIO 18

static const char *TAG = "BUZZER";

void buzzer_init(void) {
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_GPIO, 0);
    ESP_LOGI(TAG, "Пищалка инициализирована");
}

void buzzer_on(void) {
    gpio_set_level(BUZZER_GPIO, 1);
}

void buzzer_off(void) {
    gpio_set_level(BUZZER_GPIO, 0);
}

void buzzer_beep(uint32_t duration_ms) {
    buzzer_on();
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    buzzer_off();
}