/**
  *****************************************************************************
  * @file               : flash_storage.h
  * @brief              : header for flash_storage.c file.
  *****************************************************************************
 */
#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H
// includes
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include <stdint.h>

// - DEFINES -
// flash storage location and size
#define FLASH_STORAGE_ADDR                  0x08060000
#define FLASH_STORAGE_SECTOR                FLASH_SECTOR_7
#define FLASH_STORAGE_SIZE                  (128 * 1024)

// total UID and amount
#define FLASH_UID_MAX_LEN                   10
#define FLASH_UID_TYPE_PAD_LEN              4
#define FLASH_MAX_ENTRIES                   256

// entry states
#define FLASH_ENTRY_EMPTY                   0xFF
#define FLASH_ENTRY_VALID                   0xA5
#define FLASH_ENTRY_DELETED                 0x00

// - TYPEDEFS -
// typedef struct
typedef struct
{
    uint8_t uidLen;
    uint8_t uid[FLASH_UID_MAX_LEN];
    uint8_t state;                          // UID valid/deleted
    uint8_t pad[FLASH_UID_TYPE_PAD_LEN];    // padding to 16-bytes 
} FlashUID_t;

// - FUNCTION PROTOTYPES -


#endif


