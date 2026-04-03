#include "rfid_rc522.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include <stdint.h>

// SPI1 handle 
extern SPI_HandleTypeDef hspi1;

void RFID_RC522_WriteReg(uint8_t reg, uint8_t value)
{
    // addressing: MSB 0=write; LSB 0 (always); middle bits is address (& 0x7E)
    uint8_t addr = (reg << 1) & 0x7E; 

    // drive CS pin low to select
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
    // transmit write address over SPI1
    HAL_SPI_Transmit(&hspi1, &addr, 1, HAL_MAX_DELAY);
    // transmit value to write over SPI1 
    HAL_SPI_Transmit(&hspi1, &value, 1, HAL_MAX_DELAY);
    // drive CS pin high to de-select
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}

uint8_t RFID_RC522_ReadReg(uint8_t reg)
{
    // addressing: MSB 1=read; LSB 0 (always); middle bits is address (& 0x7E)
    uint8_t addr = ((reg<<1) & 0x7E) | 0x80;
    uint8_t tx[2] = {addr, 0x00};
    uint8_t rx[2] = {0};
    
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
    // transmit address & dummy-byte then receive register data on rx[1] 
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);

    return rx[1]; // second byte is the data 
}

void RFID_RC522_Reset(void)
{
    // make sure RST is high in start-up 
    HAL_GPIO_WritePin(RFID_RC522_RST_GPIO_Port, RFID_RC522_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1); // allow 1 ms delay for RFID module system stability 

    // pull low to trigger reset
    HAL_GPIO_WritePin(RFID_RC522_RST_GPIO_Port, RFID_RC522_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1); // 1 ms reset hold

    // release back to high
    HAL_GPIO_WritePin(RFID_RC522_RST_GPIO_Port, RFID_RC522_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(50); // wait for module start-up/stabilization
}

void RFID_RC522_Init(void)
{
    // reset module 
    RFID_RC522_Reset();

    // read RFID module software version register (0x90 or 0x91) 
    for (uint8_t i=0; i<5; i++)
    {
        uint8_t version = RFID_RC522_ReadReg(0x37);
        printf("RFID-RC522 Software Version: 0x%X\r\n", version);
        printf("Reg 0x01: 0x%X\r\n", RFID_RC522_ReadReg(0x01));
        printf("Reg 0x07: 0x%X\r\n", RFID_RC522_ReadReg(0x07));
        printf("Reg 0x14: 0x%X\r\n", RFID_RC522_ReadReg(0x14));
        HAL_Delay(1000);
    }

    // other register configurations (antenna and etc...)

    /* 
    There is a problem with the RFID-RC522 module (faulty/clone module). 
    Reading the registers above should give 0x90/91, 0x00, 0x00, 0x00.
    What I'm getting, 0xB2, 0x20, 0x21, 0x80. 

    Next step is to try another RFID module that operates properly. 
    */
}