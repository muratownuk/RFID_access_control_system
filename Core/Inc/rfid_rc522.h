#ifndef __RFID_RC522_H
#define __RFID_RC522_H

#include "stm32f4xx_hal.h"

// function prototypes 
void RFID_RC522_Reset(void);
void RFID_RC522_AntennaOn(void);
void RFID_RC522_Init(void);

void RFID_RC522_startTransceive(uint8_t *txData, uint8_t txLen);
uint8_t RFID_RC522_readResponse(uint8_t *rxData, uint8_t *rxLen);

#endif
