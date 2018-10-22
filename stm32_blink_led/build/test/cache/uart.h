

void UART_init(uint32_t baudrate);

int UART_getChr(void);

void UART_putStr(char *s);

void UART_putInt(int value, int radix);

char *UART_getStr(char *buf);

void UART_RX_STR_EVENT(void);

void register_uart_str_rx_event_callback(void (*callback)(char *pBuf));
