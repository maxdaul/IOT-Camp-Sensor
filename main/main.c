#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "bme280.h"
#include "mq7.h"
#include "led.h"
#include "buzzer.h"

static const char *TAG = "SENSOR";

// параметры рекламы BLE
static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x100,
    .adv_int_max        = 0x200,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// 8 байт температура, давление, CO, резерв
static uint8_t sensor_bytes[8] = {0};

// обновление рекламных данных
static void update_adv_data(void)
{
    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp = false,
        .include_name = true,
        .include_txpower = false,
        .manufacturer_len = 8,
        .p_manufacturer_data = sensor_bytes,
        .flag = 0x06,
    };
    
    esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config adv data: %d", ret);
    }
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(TAG, "Adv data set complete, starting advertising...");
            esp_ble_gap_start_advertising(&adv_params);
            break;
            
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "BLE advertising started successfully");
            } else {
                ESP_LOGE(TAG, "BLE advertising failed: %d", param->adv_start_cmpl.status);
            }
            break;
            
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            ESP_LOGI(TAG, "BLE advertising stopped");
            break;
            
        default:
            break;
    }
}

void app_main(void)
{
    // NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // датчики
    bme280_init();
    mq7_init();
    led_init();
    buzzer_init();
    
    // BLE
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gap_set_device_name("CampSensor"));  // Изменил имя
    
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "CampSensor готов!");
    ESP_LOGI(TAG, "Имя устройства: CampSensor");
    ESP_LOGI(TAG, "Формат: T(2) P(2) CO(2) RES(2)");
    ESP_LOGI(TAG, "========================================");
    
    float temperature, pressure, co_voltage;
    
    // первоначальная настройка рекламы
    update_adv_data();
    
    while (1) {
        temperature = bme280_read_temperature();
        pressure = bme280_read_pressure();
        co_voltage = mq7_read_voltage();
        
        uint16_t temp_code = (uint16_t)(temperature * 10);
        sensor_bytes[0] = (temp_code >> 8) & 0xFF;
        sensor_bytes[1] = temp_code & 0xFF;
        
        // давление в целое число
        uint16_t press_code = (uint16_t)pressure;
        sensor_bytes[2] = (press_code >> 8) & 0xFF;
        sensor_bytes[3] = press_code & 0xFF;
        
        // преобразование CO
        uint16_t co_code = (uint16_t)(co_voltage * 100);
        sensor_bytes[4] = (co_code >> 8) & 0xFF;
        sensor_bytes[5] = co_code & 0xFF;
        
        // резерв
        sensor_bytes[6] = 0x00;
        sensor_bytes[7] = 0x00;
        
        update_adv_data();
        
        ESP_LOGI(TAG, "=== ПОКАЗАНИЯ ===");
        ESP_LOGI(TAG, "Температура: %.1f°C", temperature);
        ESP_LOGI(TAG, "Давление: %.0f hPa", pressure);
        ESP_LOGI(TAG, "CO: %.2fV", co_voltage);
        ESP_LOGI(TAG, "HEX: %02X %02X %02X %02X %02X %02X %02X %02X",
                 sensor_bytes[0], sensor_bytes[1], sensor_bytes[2], sensor_bytes[3],
                 sensor_bytes[4], sensor_bytes[5], sensor_bytes[6], sensor_bytes[7]);
        ESP_LOGI(TAG, "================");
        
        led_set_green();
        
        // предупреждения
        if (temperature < 5) {
            ESP_LOGW(TAG, "ХОЛОДНО!");
            led_set_blue();
            buzzer_beep(300);
        } else if (temperature > 35) {
            ESP_LOGW(TAG, "ЖАРКО!");
            led_set_red();
            buzzer_beep(300);
        } else if (co_voltage > 1.5) {
            ESP_LOGW(TAG, "ВЫСОКИЙ CO!");
            led_set_yellow();
            buzzer_beep(500);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}