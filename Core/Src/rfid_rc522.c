#include "rfid_rc522.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include <stdint.h>

// SPI1 handle 
extern SPI_HandleTypeDef hspi1;

#define RFID_RC522_CS_PORT  GPIOA
#define RFID_RC522_CS_PIN   GPIO_PIN_4

void RFID_RC522_WriteReg(uint8_t reg, uint8_t value)
{
    // addressing: MSB 0=write; LSB 0 (always); middle bits is address (& 0x7E)
    uint8_t addr = (reg << 1) & 0x7E; 

    // drive CS pin low to select
    HAL_GPIO_WritePin(RFID_RC522_CS_PORT, RFID_RC522_CS_PIN, GPIO_PIN_RESET);
    // transmit write address over SPI1
    HAL_SPI_Transmit(&hspi1, &addr, 1, HAL_MAX_DELAY);
    // transmit value to write over SPI1 
    HAL_SPI_Transmit(&hspi1, &value, 1, HAL_MAX_DELAY);
    // drive CS pin high to de-select
    HAL_GPIO_WritePin(RFID_RC522_CS_PORT, RFID_RC522_CS_PIN, GPIO_PIN_SET);
}

uint8_t RFID_RC522_ReadReg(uint8_t reg)
{
    // addressing: MSB 1=read; LSB 0 (always); middle bits is address (& 0x7E)
    uint8_t addr = ((reg<<1) & 0x7E) | 0x80;
    uint8_t value = 0; 

    HAL_GPIO_WritePin(RFID_RC522_CS_PORT, RFID_RC522_CS_PIN, GPIO_PIN_RESET);
    // transmit read address over SPI1
    HAL_SPI_Transmit(&hspi1, &addr, 1, HAL_MAX_DELAY);
    // receive value of address over SPI1
    HAL_SPI_Receive(&hspi1, &value, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(RFID_RC522_CS_PORT, RFID_RC522_CS_PIN, GPIO_PIN_SET);

    return value;
}


