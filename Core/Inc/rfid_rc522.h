/**
  *****************************************************************************
  * @file               : rfid_rc522.h
  * @brief              : header for rfid_rc522.c file.
  *****************************************************************************
 */
#ifndef __RFID_RC522_H
#define __RFID_RC522_H
#include <stdint.h>

// - TYPEDEFS - 
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

// - FUNCTION PROTOTYPES -  
void RFID_RC522_Reset(void);
void RFID_RC522_AntennaOn(void);
void RFID_RC522_Init(void);

void RFID_RC522_startTransceive(uint8_t *txData, uint8_t txLen, \
    uint8_t txLastBits);
RFID_Status_t RFID_RC522_readResponse(uint8_t *rxData, uint8_t *rxLen);

#endif


