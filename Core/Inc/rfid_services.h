/**
  *****************************************************************************
  * @file               : rfid_services.h
  * @brief              : header for rfid_services.c file.
  *****************************************************************************
 */
#ifndef __RFID_SERVICES_H
#define __RFID_SERVICES_H
#include "rfid_rc522.h"
#include <string.h>

// - DEFINES -
// poll and timeouts
//#define DEBUG_MODE                    // debug mode (uncomment)
#ifdef DEBUG_MODE
  #define RFID_POLL_PERIOD              1000
  #define RFID_SEM_ACQUIRE_TIMEOUT      800
#else
  #define RFID_POLL_PERIOD              200
  #define RFID_SEM_ACQUIRE_TIMEOUT      50
#endif

#define RFID_ITEM_REMOVE_TIMEOUT        1500

// RFID_Item_t struct lengths
#define RFID_MAX_UID_LEN                10
#define RFID_ATQA_LEN                   2
#define PADDING_LEN                     2 

// tx/rx buf sizes
#define RX_BUF_SIZE                     16
#define TX_BUF_SIZE                     2

// - TYPEDEFS -
// typedef enum
typedef enum {
  RFID_ITEM_REMOVED = 0,
  RFID_ITEM_PRESENT
} RFID_ItemEvent_t;

// typedef struct 
typedef struct {
  uint8_t atqa[RFID_ATQA_LEN];      // ATQA (2 bytes)
  uint8_t event;                    // item present/removed
  uint8_t uidLen;                   // number of valid bytes
  uint8_t uid[RFID_MAX_UID_LEN];    // full UID
  uint8_t padding[PADDING_LEN];     // pad RFID_Item_t so its 16 bytes
} RFID_Item_t;

// - FUNCTION PROTOTYPES -
void RFID_RC522_resetTxRxBuf(uint8_t *txBuf, uint8_t *rxBuf);

RFID_Status_t RFID_RC522_poll(uint8_t *txBuf, uint8_t *rxBuf, uint8_t *rxLen);
RFID_Status_t RFID_RC522_antiColl(uint8_t *txBuf, uint8_t *rxBuf, \
  uint8_t *rxLen);

#endif


