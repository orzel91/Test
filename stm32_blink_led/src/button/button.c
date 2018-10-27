#include "button.h"
#include "gpio.h"
#include "config.h"
#include "hdr_gpio.h"


void button_init(void)
{
    gpio_pin_cfg(BUTTON1_GPIO, BUTTON1_PIN, GPIO_CRx_MODE_CNF_IN_FLOATING_value);
}
