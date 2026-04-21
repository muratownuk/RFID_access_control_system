/**
  *****************************************************************************
  * @file               : rfid_services.c
  * @brief              : provides RFID-RC522 driver services.
  *****************************************************************************
 */
// includes
#include "rfid_services.h"
#include "cmsis_os.h"

// - DEFINES -
// tx commands
#define REQA_CMD                      0x26
#define REQA_TXLASTBITS               0x07
#define ANTICOLL_CMD                  0x20
#define ANTICOLL_TXLASTBITS           0x00
#define CASCADE_LEVEL_1               0x93
#define CASCADE_LEVEL_2               0x95
#define CASCADE_LEVEL_3               0x97
#define CASCADE_TAG                   0x88

// - VARIABLES
extern osSemaphoreId_t RFIDSemHandle;           // created in freertos.c

// - FUNCTIONS - 
void RFID_RC522_resetTxRxBuf(uint8_t *txBuf, uint8_t *rxBuf)
{
  memset(txBuf, 0, TX_BUF_SIZE);
  memset(rxBuf, 0, RX_BUF_SIZE);
}

RFID_Status_t RFID_RC522_poll(uint8_t *txBuf, uint8_t *rxBuf, uint8_t *rxLen)
{
  txBuf[0] = REQA_CMD;

  // start REQA transmission 
  RFID_RC522_startTransceive(txBuf, 1, REQA_TXLASTBITS);
  
  // wait until ISR (EXTI0 callback) releases semaphore (with timeout)
  if (osSemaphoreAcquire(RFIDSemHandle, RFID_SEM_ACQUIRE_TIMEOUT) != osOK)
    return RFID_ERR_POLL_SEM_TIMEOUT; // Timeout! Retry
  
  // read & return ATQA reception
  return RFID_RC522_readResponse(rxBuf, rxLen);
}

RFID_Status_t RFID_RC522_antiColl(uint8_t *txBuf, uint8_t *rxBuf, \
  uint8_t *rxLen)
{
  uint8_t tmpBuf[8], tmpLen;
  uint8_t status; 

  uint8_t cascadeLevels[3] = {CASCADE_LEVEL_1,
                              CASCADE_LEVEL_2,
                              CASCADE_LEVEL_3};
  uint8_t uidIndex = 0;
  uint8_t bcc; 

  *rxLen = 0;

  for (uint8_t level = 0; level < 3; level++)
  {
    txBuf[0] = cascadeLevels[level];
    txBuf[1] = ANTICOLL_CMD;

    RFID_RC522_startTransceive(txBuf, 2, ANTICOLL_TXLASTBITS);

    // wait until ISR (EXTI0 callback) releases semaphore (with timeout)
    if (osSemaphoreAcquire(RFIDSemHandle, RFID_SEM_ACQUIRE_TIMEOUT) != osOK)
      return RFID_ERR_ANTICOLL_SEM_TIMEOUT; // Timeout! Retry
       
    status = RFID_RC522_readResponse(tmpBuf, &tmpLen);

    if (status != RFID_OK || tmpLen != 5) // expected: 4-bytes UID; 1-byte BCC
      return RFID_ERR_ANTICOLL_RX;
    
    // BCC check
    bcc = tmpBuf[0] ^ tmpBuf [1] ^ tmpBuf[2] ^ tmpBuf[3];
    if (bcc != tmpBuf[4])
      return RFID_ERR_ANTICOLL_BCC;

    // cascade handling
    if (tmpBuf[0] == CASCADE_TAG)
    {
      // skip CT, copy next 3 bytes 
      for (uint8_t i = 1; i < 4; i++)
        rxBuf[uidIndex++] = tmpBuf[i];
    }
    else
    {
      // last cascade; copy all 4 bytes
      for (uint8_t i = 0; i < 4; i++)
        rxBuf[uidIndex++] = tmpBuf[i];

      break;
    }
  }

  if (uidIndex == 0)
    return RFID_ERR_ANTICOLL_RX;

  *rxLen = uidIndex;

  return RFID_OK;
}




