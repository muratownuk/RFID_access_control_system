#include "rfid_rc522.h"
#include "stm32f4xx_hal_spi.h"
#include <stdint.h>

// SPI1 handle 
extern SPI_HandleTypeDef hspi1;

#define RFID_RC522_CS_PORT  GPIOA
#define RFID_RC522_CS_PIN   GPIO_PIN_4

void RFID_RC522_WriteReg(uint8_t reg, uint8_t value)
{
    // addressing: MSB 0=write; LSB 0 (always); middle bits is address (& 0x7E)
    uint8_t addr = (reg << 1) & 0x7E 

    

}
