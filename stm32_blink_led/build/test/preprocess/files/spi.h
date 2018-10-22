void SPI_init(void);

void SPI_sendData(uint8_t* src, uint16_t cnt);

void SPI_sendCmd(uint8_t cmd);

void SPI_checkDmaStatus(void);
