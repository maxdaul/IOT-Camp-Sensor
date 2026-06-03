cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Указываем, где лежат наши компоненты
set(EXTRA_COMPONENT_DIRS 
    components/bme280
    components/mq7
    components/led
    components/buzzer
)

project(project-name12312)