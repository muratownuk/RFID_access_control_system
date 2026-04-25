/**
  *****************************************************************************
  * @file               : flash_storage.c
  * @brief              : RFID UID data storage in STM32 flash
  *****************************************************************************
 */
// includes
#include "flash_storage.h"
#include <string.h>

// global variables
static uint32_t readWritePtr = FLASH_STORAGE_ADDR;
extern osMutexId_t flashMutexHandle;        // created in freertos.c

// - FUNCTIONS - 
// internal 
static void Flash_LockAll(void)
{
    HAL_FLASH_Lock();
}

static void Flash_UnlockAll(void)
{
    HAL_FLASH_Unlock();
}

static uint8_t isSameUID(FlashUID_t *entry, uint8_t *uid, uint8_t uidLen)
{
    // returns 1 if same UID, 0 otherwise
    return ((entry->uidLen == uidLen) && (memcmp(entry->uid, uid, uidLen) == 0));
}

// init
void FlashStorage_Init(void)
{
    // flash start address
    uint32_t addr = FLASH_STORAGE_ADDR;     
    // entry ptr to FlashUID data structure
    FlashUID_t *entryPtr;

    while (addr < (FLASH_STORAGE_ADDR + FLASH_STORAGE_SIZE))
    {
        entryPtr = (FlashUID_t *)addr;

        if (entryPtr->state == FLASH_ENTRY_EMPTY)
        {
            readWritePtr = addr;
            return;
        }

        // update addr to next FlashUID_t if there is data (FLASH_ENTRY_VALID)
        addr += sizeof(FlashUID_t);

    }
    // at the end of sector (full)
    readWritePtr = addr;

}

// exists
uint8_t FlashStorage_Exists(uint8_t *uid, uint8_t uidLen)
{
    uint32_t addr = FLASH_STORAGE_ADDR;
    FlashUID_t *entryPtr;

    while (addr < readWritePtr)
    {
        entryPtr = (FlashUID_t *)addr;

        if ((entryPtr->state == FLASH_ENTRY_VALID) && \
        (isSameUID(entryPtr, uid, uidLen)))
            return 1; // UID exists in flash
        
        addr += sizeof(FlashUID_t);
    }

    return 0; // UID doesn't exist in flash
}

// add
HAL_StatusTypeDef FlashStorage_Add(uint8_t *uid, uint8_t uidLen)
{
    uint32_t *data;
    FlashUID_t entry;

    if (FlashStorage_Exists(uid, uidLen))
        return HAL_OK; // already exists

    // acquire mutex for flash operation
    osMutexAcquire(flashMutexHandle, osWaitForever);
    
    // initialize entry data
    memset(&entry, 0, sizeof(entry));

    // populate
    entry.uidLen = uidLen;
    memcpy(entry.uid, uid, uidLen);
    entry.state = FLASH_ENTRY_VALID;

    data = (uint32_t *)&entry;

    // write to flash
    Flash_UnlockAll();

    for (uint8_t i = 0; i < (sizeof(FlashUID_t) / 4); i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, readWritePtr, data[i]) \
        != HAL_OK)
        {
            Flash_LockAll();
            osMutexRelease(flashMutexHandle);
            return HAL_ERROR;
        }

        readWritePtr += 4;
    }

    Flash_LockAll();
    osMutexRelease(flashMutexHandle);
    
    return HAL_OK;
}

// read all 
uint32_t FlashStorage_ReadAll(FlashUID_t *buffer, uint32_t max)
{
    uint32_t addr = FLASH_STORAGE_ADDR, count = 0;

    FlashUID_t *entryPtr;
    
    while (addr < readWritePtr)
    {
        entryPtr = (FlashUID_t *)addr;

        if (entryPtr->state == FLASH_ENTRY_VALID)
        {
            if (count < max)
                memcpy(&buffer[count], entryPtr, sizeof(FlashUID_t));

            count++;
        }

        addr += sizeof(FlashUID_t);
    }

    return count;
}

// delete
HAL_StatusTypeDef FlashStorage_Delete(uint8_t *uid, uint8_t uidLen)
{
    uint32_t addr = FLASH_STORAGE_ADDR;

    FlashUID_t *entryPtr;

    osMutexAcquire(flashMutexHandle, osWaitForever);

    // delete entry from flash (change .state)
    Flash_UnlockAll();

    while (addr < readWritePtr)
    {
        entryPtr = (FlashUID_t *)addr;

        if ((entryPtr->state == FLASH_ENTRY_VALID) && \
        isSameUID(entryPtr, uid, uidLen))
        {
            // mark as deleted
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, \
                addr + offsetof(FlashUID_t, state), FLASH_ENTRY_DELETED) \
                != HAL_OK)
            {
                // flash programming error...
                Flash_LockAll();
                osMutexRelease(flashMutexHandle);
                return HAL_ERROR;
            }
            
            // if we reach here, UID data is in flash and delete successful
            Flash_LockAll();
            osMutexRelease(flashMutexHandle);
            return HAL_OK;
        }

        addr += sizeof(FlashUID_t);
    }

    // if we reach here, UID data is not in flash
    Flash_LockAll();

    osMutexRelease(flashMutexHandle);

    return HAL_ERROR;
}

// erase
HAL_StatusTypeDef FlashStorage_EraseAll(void)
{
    uint32_t sectorErr;
    FLASH_EraseInitTypeDef eraseInit;
    HAL_StatusTypeDef status;

    osMutexAcquire(flashMutexHandle, osWaitForever);

    // erase flash
    Flash_UnlockAll();

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Sector = FLASH_STORAGE_SECTOR;
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    status = HAL_FLASHEx_Erase(&eraseInit, &sectorErr);

    Flash_LockAll();

    // reset read/write pointer
    readWritePtr = FLASH_STORAGE_ADDR;

    osMutexRelease(flashMutexHandle);

    return status;
}



