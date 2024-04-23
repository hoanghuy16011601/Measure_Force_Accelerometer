#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>


#include "HUY_HX711_Drive.h"
#include "HUY_GPIO_Drive.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



static void hx711_usleep(long sleep);
static uint32_t zero_offset;
static uint8_t HX711_DATA_PIN;
static uint8_t HX711_CLK_PIN;

void hx711_init(uint8_t Data_Pin, uint8_t CLK_Pin)
{
  Configured_Input_Mode(Data_Pin, Pulldown_Disable,Pullup_Disable);
  Configure_Output_Mode(CLK_Pin);
  HX711_DATA_PIN = Data_Pin;
  HX711_CLK_PIN= CLK_Pin;
}

void hx711_deinit(void)
{
  GPIO_Reset(HX711_DATA_PIN);
  GPIO_Reset(HX711_CLK_PIN);
}

void hx711_powerdown(void)
{
  gpio_set_level(HX711_CLK_PIN, 1);
  usleep(60);
  gpio_set_level(HX711_CLK_PIN, 0);
}

void hx711_wait_for_ready(void)
{
  gpio_set_level(HX711_CLK_PIN, 0);
  while (gpio_get_level(HX711_DATA_PIN) != 0) {
    // vTaskDelay(1/ portTICK_PERIOD_MS);
    hx711_usleep(10);
  }
  return;
}

uint32_t hx711_measure(void)
{
  //portDISABLE_INTERRUPTS();
  uint32_t measure_offset = 0;
  hx711_wait_for_ready();
  for (int8_t i = 0; i < 24; i++) {
    gpio_set_level(HX711_CLK_PIN, 1);
    hx711_usleep(1);
    uint32_t v = gpio_get_level(HX711_DATA_PIN);
    measure_offset += (v << (23-i));
    gpio_set_level(HX711_CLK_PIN, 0);
    hx711_usleep(1);
  }
  // 25th, 26th pulse
  for (uint8_t i = 0; i < 2; i++) {
    gpio_set_level(HX711_CLK_PIN, 1);
    hx711_usleep(1);
    gpio_set_level(HX711_CLK_PIN, 0);
    hx711_usleep(1);
  }

  // 27th
  gpio_set_level(HX711_CLK_PIN, 1);
  while(gpio_get_level(HX711_DATA_PIN) != 1) {}
  gpio_set_level(HX711_CLK_PIN, 0);
  //portENABLE_INTERRUPTS();
  return measure_offset;
}

void hx711_set_zero_offset(uint32_t offset)
{
  printf("Zero Offet=%ld\n",zero_offset);
  zero_offset = offset;
}

uint32_t hx711_get_zero_offset(void)
{
  return zero_offset;
}

static void hx711_usleep(long sleep)
{
  usleep(sleep);
}