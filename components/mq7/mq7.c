#include "mq7.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define MQ7_ADC_CHANNEL    ADC_CHANNEL_6 

static const char *TAG = "MQ7";

static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t cali_handle = NULL;
static bool is_calibrated = false;

static bool init_adc_calibration(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle) {
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    #if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
    if (ret == ESP_OK) {
        calibrated = true;
    }
    #endif

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Калибровка АЦП не удалась");
    }

    *out_handle = handle;
    return calibrated;
}

void mq7_init(void) {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12, 
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, MQ7_ADC_CHANNEL, &chan_config));


    is_calibrated = init_adc_calibration(ADC_UNIT_1, ADC_ATTEN_DB_12, &cali_handle);

    ESP_LOGI(TAG, "MQ-7 успешно инициализирован (Калибровка: %s)", is_calibrated ? "ОК" : "ПРОПУЩЕНА");
}

int mq7_read_raw(void) {
    int raw = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, MQ7_ADC_CHANNEL, &raw));
    return raw;
}

float mq7_read_voltage(void) {
    int raw = mq7_read_raw();
    int voltage_mv = 0;

    if (is_calibrated) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &voltage_mv));
    } else {
        voltage_mv = (raw * 3300) / 4095;
    }

    return (float)voltage_mv / 1000.0f;
}