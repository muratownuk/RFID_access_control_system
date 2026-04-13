#ifndef __RFID_RC522_H
#define __RFID_RC522_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

// defines
#define RFID_MAX_UID_LEN    10
#define RFID_ATQA_LEN       2
#define PADDING_LEN         3 

// typedef enum
typedef enum {
  RFID_OK = 0,
  RFID_ERR_RES_TIMEOUT,
  RFID_ERR_RES_ERR_REG,
  RFID_ERR_POLL_SEM_TIMEOUT,
  RFID_ERR_ANTICOLL_SEM_TIMEOUT,
  RFID_ERR_ANTICOLL_RX,
  RFID_ERR_ANTICOLL_BCC
} RFID_Status_t;

// typedef struct 
typedef struct{
  uint8_t uid[RFID_MAX_UID_LEN];    // full UID
  uint8_t uidLen;                   // number of valid bytes
  uint8_t atqa[RFID_ATQA_LEN];      // ATQA (2 bytes)
  uint8_t padding[PADDING_LEN];     // pad RFID_Item_t so its 16 bytes
} RFID_Item_t;

// function prototypes 
void RFID_RC522_Reset(void);
void RFID_RC522_AntennaOn(void);
void RFID_RC522_Init(void);

void RFID_RC522_startTransceive(uint8_t *txData, uint8_t txLen, \
    uint8_t txLastBits);
RFID_Status_t RFID_RC522_readResponse(uint8_t *rxData, uint8_t *rxLen);

#endif




