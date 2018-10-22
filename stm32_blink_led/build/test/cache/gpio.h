#include "../inc/stm32f10x.h"














enum {

    P0 = 0x00000001,

    P1 = 0x00000002,

    P2 = 0x00000004,

    P3 = 0x00000008,

    P4 = 0x00000010,

    P5 = 0x00000020,

    P6 = 0x00000040,

    P7 = 0x00000080,

    P8 = 0x00000100,

    P9 = 0x00000200,

    P10 = 0x00000400,

    P11 = 0x00000800,

    P12 = 0x00001000,

    P13 = 0x00002000,

    P14 = 0x00004000,

    P15 = 0x00008000

};

void gpio_init(void);

void gpio_pin_cfg(GPIO_TypeDef *port_ptr, uint32_t pin, uint32_t mode_cnf_value);
