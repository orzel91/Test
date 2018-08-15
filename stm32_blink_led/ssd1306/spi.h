/*
 * spi.h
 *
 *  Created on: 7 sie 2018
 *      Author: jarek
 */

#ifndef SPI_SPI_H_
#define SPI_SPI_H_


#define SPI_COMMAND_SIZE 32
#define SPI_COMMAND_MASK (SPI_MESSAGE_QUEQUE_SIZE -1)

#define SPI_MESSAGE_QUEQUE_SIZE 32
#define SPI_MESSAGE_QUEQUE_MASK (SPI_MESSAGE_QUEQUE_SIZE -1)


void SPI_init(void);
void SPI_sendData(uint8_t* src, uint16_t cnt);
void SPI_sendCmd(uint8_t cmd);
void SPI_checkDmaStatus(void);


#endif /* SPI_SPI_H_ */
