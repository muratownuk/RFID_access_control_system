#include "rfid_rc522.h"
#include "cmsis_os.h"
#include "main.h"
#include "retarget.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include "mfrc522_regs.h"

// SPI1 handle 
extern SPI_HandleTypeDef hspi1;

// private defines
#define SPI_TIMEOUT_MS              10

#define RFID_RC522_TIMER_PRESCALER  0x3E
#define RFID_RC522_TIMER_RELOAD_L   0x1E

#define MFRC522_ERRORMASK           0x1B

// external variables
extern osSemaphoreId_t RFIDSemHandle;           // created in freertos.c

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
    uint8_t addr = ((reg << 1) & 0x7E) | 0x80;
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

void RFID_RC522_AntennaOn(void)
{
    uint8_t tmp = RFID_RC522_ReadReg(TxControlReg);

    if (!(tmp & (TxControlReg_Tx1RFEn | TxControlReg_Tx2RFEn)))
    {
        RFID_RC522_SetBitMask(TxControlReg, \
            TxControlReg_Tx1RFEn | TxControlReg_Tx2RFEn);
    }        
}

static void RFID_RC522_Config(void)
{
    uint8_t tmp;

    // - CONFIGURE TIMER - 
    tmp = RFID_RC522_ReadReg(TModeReg);
    // clear prescaler high bits
    tmp &= ~TModeReg_TPrescaler_HiMask; 
    // set prescaler high to 0x0D        
    tmp |= TModeReg_TAuto | (0x0D & TModeReg_TPrescaler_HiMask); 
    RFID_RC522_WriteReg(TModeReg, tmp); 

    tmp = RFID_RC522_TIMER_PRESCALER;
    RFID_RC522_WriteReg(TPrescalerReg, tmp);

    tmp = RFID_RC522_TIMER_RELOAD_L;
    RFID_RC522_WriteReg(TReloadRegL, tmp);

    tmp = 0x00;
    RFID_RC522_WriteReg(TReloadRegH, tmp);

    // - CONFIGURE ASK MODULATION - 
    tmp = RFID_RC522_ReadReg(TxASKReg);
    tmp |= TxASKReg_Force100ASK;
    RFID_RC522_WriteReg(TxASKReg, tmp);

    // - CONFIGURE MODE - 
    tmp = RFID_RC522_ReadReg(ModeReg);
    // clear CRC preset bits
    tmp &= ~ModeReg_CRCPresetMask;              
    // set mode bits / set CRC Preset to 0x01
    tmp |= ModeReg_TxWaitRF | ModeReg_PolMFin | (0x01 & ModeReg_CRCPresetMask); 
    RFID_RC522_WriteReg(ModeReg, tmp);

    // - CONFIGURE ANTENNA - 
    // turn antenna on
    RFID_RC522_AntennaOn();

    // - CONFIGURE IRQ - 
    // clear all pending IRQs
    RFID_RC522_WriteReg(ComIrqReg, 0x7F);

    // enable IRQ pin output 
    tmp = RFID_RC522_ReadReg(DivIEnReg);
    tmp |= DivIEnReg_IRQPushPull;
    RFID_RC522_WriteReg(DivIEnReg, tmp);

    // disable IRqInv and enable RxIRq 
    tmp = RFID_RC522_ReadReg(ComIEnReg);
    tmp &= ~ComIEnReg_IRqInv;
    tmp |= ComIEnReg_RxIEn;
    RFID_RC522_WriteReg(ComIEnReg, tmp);

    // flush FIFO
    RFID_RC522_SetBitMask(FIFOLevelReg, FIFOLevelReg_FlushBuffer);

    // set module to idle
    RFID_RC522_WriteReg(CommandReg, PCD_IDLE);   
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

    // read RFID module software version register and key registers 
    DEBUG_LOG0("RFID-RC522 Software Version: 0x%X\r\n", \
        RFID_RC522_ReadReg(VersionReg));
    DEBUG_LOG0("--- Initial register values ---\r\n");
    DEBUG_LOG0("CommandReg (01h): 0x%X\r\n", \
        RFID_RC522_ReadReg(CommandReg));
    DEBUG_LOG0("Status1Reg(07h): 0x%X\r\n", \
        RFID_RC522_ReadReg(Status1Reg));
    DEBUG_LOG0("TxControlReg (0x14): 0x%X\r\n", \
        RFID_RC522_ReadReg(TxControlReg));
    DEBUG_LOG0("-------------------------------\r\n");
    HAL_Delay(1000);

    // configure RFID-RC522 
    RFID_RC522_Config();
}

void RFID_RC522_startTransceive(uint8_t *txData, uint8_t txLen, \
    uint8_t txLastBits)
{
    // RFID-RC522 in idle 
    RFID_RC522_WriteReg(CommandReg, PCD_IDLE);

    // clear IRq flags
    RFID_RC522_WriteReg(ComIrqReg, 0x7F);

    // flush FIFO buffer 
    RFID_RC522_SetBitMask(FIFOLevelReg, FIFOLevelReg_FlushBuffer);

    // set BitFramingReg TxLastBits
    RFID_RC522_WriteReg(BitFramingReg, \
        txLastBits & BitFramingReg_TxLastBitsMask);

    // write data to FIFO buffer
    for (uint8_t i = 0; i < txLen; i++)
        RFID_RC522_WriteReg(FIFODataReg, txData[i]);

    // start transceive command
    RFID_RC522_WriteReg(CommandReg, PCD_TRANSCEIVE);

    // start sending 
    RFID_RC522_SetBitMask(BitFramingReg, BitFramingReg_StartSend);

    DEBUG_LOG2("RFID_RC522_startTransceive: starting transceive...\r\n");
}

RFID_Status_t RFID_RC522_readResponse(uint8_t *rxData, uint8_t *rxLen)
{
    uint8_t irq;
    uint8_t fifoLevel;

    DEBUG_LOG2("RFID_RC522_readResponse: reading response...\r\n");

    irq = RFID_RC522_ReadReg(ComIrqReg);

    // clear StartSend
    RFID_RC522_ClearBitMask(BitFramingReg, BitFramingReg_StartSend);

    // timeout or error check
    if(!(irq & ComIrqReg_RxIRq))
        return RFID_ERR_RES_TIMEOUT; // timeout 

    if(RFID_RC522_ReadReg(ErrorReg) & MFRC522_ERRORMASK)
        return RFID_ERR_RES_ERR_REG; // error

    // read FIFO
    fifoLevel = RFID_RC522_ReadReg(FIFOLevelReg) & FIFOLevelReg_LevelMask;
    *rxLen = fifoLevel;

    for (uint8_t i = 0; i < *rxLen; i++)
        rxData[i] = RFID_RC522_ReadReg(FIFODataReg);

    // clear IRQ flags
    RFID_RC522_WriteReg(ComIrqReg, 0x7F);

    return RFID_OK; // success
}

