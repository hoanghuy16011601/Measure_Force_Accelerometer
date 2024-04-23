set (SRC_DIRS "../Common/Source")
set(component_srcs "Project_main.c" "${SRC_DIRS}/HUY_WIFI_Drive.c" "${SRC_DIRS}/HUY_HTTP_SERVER_Drive.c" "${SRC_DIRS}/HUY_I2C_Drive.c" "${SRC_DIRS}/HUY_MPU6050_Drive.c" "${SRC_DIRS}/HUY_MQTT_Drive.c" "${SRC_DIRS}/HUY_HX711_Drive.c" "${SRC_DIRS}/HUY_GPIO_Drive.c")



idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "../Common/Include"
                      )