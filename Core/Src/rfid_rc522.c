#include "rfid_rc522.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include "mfrc522_regs.h"
#include <stdint.h>

// SPI1 handle 
extern SPI_HandleTypeDef hspi1;

static void RFID_RC522_WriteReg(uint8_t reg, uint8_t value)
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

static uint8_t RFID_RC522_ReadReg(uint8_t reg)
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

static void RFID_RC522_SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = RFID_RC522_ReadReg(reg);
    RFID_RC522_WriteReg(reg, tmp | mask);
}

static void RFID_RC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = RFID_RC522_ReadReg(reg);
    RFID_RC522_WriteReg(reg, tmp & (~mask));
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

void RFID_RC522_AntennaOn(void)
{
    uint8_t tmp = RFID_RC522_ReadReg(TxControlReg);

    if (!(tmp & (TxControlReg_Tx1RFEn | TxControlReg_Tx2RFEn)))
    {
        RFID_RC522_SetBitMask(TxControlReg, \
            TxControlReg_Tx1RFEn | TxControlReg_Tx2RFEn);
    }        
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
        printf("CommandReg (01h): 0x%X\r\n", RFID_RC522_ReadReg(CommandReg));
        printf("Status1Reg(07h): 0x%X\r\n", RFID_RC522_ReadReg(Status1Reg));
        printf("TxControlReg (0x14): 0x%X\r\n", \
            RFID_RC522_ReadReg(TxControlReg));
        HAL_Delay(1000);
    }

    // turn antenna on 
    RFID_RC522_AntennaOn();
}

static uint8_t RFID_RC522_Transceive(uint8_t *txData, uint8_t txLen, \
    uint8_t *rxData, uint8_t *rxLen)
{
    uint8_t waitIRq = ComIrqReg_RxIRq | ComIrqReg_IdleIRq;
    uint8_t n;
    uint32_t i; 

    // RFID-RC522 in idle 
    RFID_RC522_WriteReg(CommandReg, PCD_IDLE);

    // clear IRq flags
    RFID_RC522_WriteReg(ComIrqReg, 0x7F);

    // flush FIFO buffer 
    RFID_RC522_SetBitMask(FIFOLevelReg, FIFOLevelReg_FlushBuffer);

    // write data to FIFO buffer
    for (uint8_t j = 0; j < txLen; j++)
        RFID_RC522_WriteReg(FIFODataReg, txData[j]);

    // command execution 
    RFID_RC522_WriteReg(CommandReg, PCD_TRANSCEIVE);

    // start transmission 
    RFID_RC522_SetBitMask(BitFramingReg, BitFramingReg_StartSend);

    // wait 
    i = 1000;
    do {
        n = RFID_RC522_ReadReg(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & waitIRq));

    RFID_RC522_ClearBitMask(BitFramingReg, BitFramingReg_StartSend);

    // timed out
    if (i == 0)
        return 1; 

    // error 
    if (RFID_RC522_ReadReg(ErrorReg) & 0x1B)
        return 2; 

    // read received data
    uint8_t fifoLevel = RFID_RC522_ReadReg(FIFOLevelReg);
    *rxLen = fifoLevel;

    for (uint8_t j = 0; j < fifoLevel; j++)
        rxData[j] = RFID_RC522_ReadReg(FIFODataReg);

    // successful read 
    return 0; 
}

uint8_t RFID_RC522_Request(uint8_t *tagType)
{
    uint8_t status;
    uint8_t backLen;
    uint8_t buffer[2];

    RFID_RC522_WriteReg(BitFramingReg, BitFramingReg_TxLastBitsMask & 0x07);

    buffer[0] = 0x26; // REQA

    status = RFID_RC522_Transceive(buffer, 1, buffer, &backLen);

    if ((status == 0) && (backLen==2))
    {
        tagType[0] = buffer[0];
        tagType[1] = buffer[1];
        return 0; // card detection 
    }

    return 1; // no card
}

