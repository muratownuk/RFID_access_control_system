#ifndef __RFID_RC522_H
#define __RFID_RC522_H

#include "stm32f4xx_hal.h"

// function prototypes 
void RFID_RC522_WriteReg(uint8_t reg, uint8_t value);
uint8_t RFID_RC522_ReadReg(uint8_t reg);
void RFID_RC522_Reset(void);
void RFID_RC522_Init(void);

#endif