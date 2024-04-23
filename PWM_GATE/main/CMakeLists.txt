set (SRC_DIRS "../Common/Source")
set(component_srcs "PWM_main.c" "${SRC_DIRS}/HUY_PWM_Drive.c" "${SRC_DIRS}/HUY_GPIO_Drive.c" "${SRC_DIRS}/HUY_WIFI_Drive.c" "${SRC_DIRS}/HUY_HTTP_SERVER_Drive.c")



idf_component_register(SRCS "${component_srcs}"
                        INCLUDE_DIRS "../Common/Include"
                      )