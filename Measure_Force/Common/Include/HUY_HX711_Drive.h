

#include <stdint.h>

void hx711_init(uint8_t Data_Pin, uint8_t CLK_Pin);
void hx711_deinit(void);

void hx711_powerdown(void);

void hx711_wait_for_ready(void);
uint32_t hx711_measure(void);

uint32_t hx711_get_zero_offset(void);
void hx711_set_zero_offset(uint32_t offset);

