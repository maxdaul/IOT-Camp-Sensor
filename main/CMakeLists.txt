idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES 
                           bme280
                           mq7
                           led
                           buzzer
                           bt
                           nvs_flash)