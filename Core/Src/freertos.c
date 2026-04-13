/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "retarget.h"
#include "rfid_rc522.h"
#include "mfrc522_regs.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define REQA_CMD                      0x26
#define REQA_TXLASTBITS               0x07
#define ANTICOLL_CMD                  0x20
#define ANTICOLL_TXLASTBITS           0x00
#define CASCADE_LEVEL_1               0x93
#define CASCADE_LEVEL_2               0x95
#define CASCADE_LEVEL_3               0x97
#define CASCADE_TAG                   0x88

#define RX_BUF_SIZE                   16
#define TX_BUF_SIZE                   2

#define RFID_SEM_ACQUIRE_TIMEOUT      1000 // essentially, how fast we polls 

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RFIDDriverTask */
osThreadId_t RFIDDriverTaskHandle;
const osThreadAttr_t RFIDDriverTask_attributes = {
  .name = "RFIDDriverTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for RFIDAppTask */
osThreadId_t RFIDAppTaskHandle;
const osThreadAttr_t RFIDAppTask_attributes = {
  .name = "RFIDAppTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for xRFIDQueue */
osMessageQueueId_t xRFIDQueueHandle;
uint8_t xRFIDQueueBuffer[ 10 * 16 ];
osStaticMessageQDef_t xRFIDQueueControlBlock;
const osMessageQueueAttr_t xRFIDQueue_attributes = {
  .name = "xRFIDQueue",
  .cb_mem = &xRFIDQueueControlBlock,
  .cb_size = sizeof(xRFIDQueueControlBlock),
  .mq_mem = &xRFIDQueueBuffer,
  .mq_size = sizeof(xRFIDQueueBuffer)
};
/* Definitions for RFIDSem */
osSemaphoreId_t RFIDSemHandle;
const osSemaphoreAttr_t RFIDSem_attributes = {
  .name = "RFIDSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void RFID_RC522_resetTxRxBuf(uint8_t *txBuf, uint8_t *rxBuf);

RFID_Status_t RFID_RC522_poll(uint8_t *txBuf, uint8_t *rxBuf, uint8_t *rxLen);
RFID_Status_t RFID_RC522_antiColl(uint8_t *txBuf, uint8_t *rxBuf, \
  uint8_t *rxLen);


/* USER CODE END FunctionPrototypes */

void DefaultTask(void *argument);
void RFID_DriverTask(void *argument);
void RFID_AppTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of RFIDSem */
  RFIDSemHandle = osSemaphoreNew(1, 0, &RFIDSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of xRFIDQueue */
  xRFIDQueueHandle = osMessageQueueNew (10, 16, &xRFIDQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(DefaultTask, NULL, &defaultTask_attributes);

  /* creation of RFIDDriverTask */
  RFIDDriverTaskHandle = osThreadNew(RFID_DriverTask, NULL, &RFIDDriverTask_attributes);

  /* creation of RFIDAppTask */
  RFIDAppTaskHandle = osThreadNew(RFID_AppTask, NULL, &RFIDAppTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_DefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_DefaultTask */
void DefaultTask(void *argument)
{
  /* USER CODE BEGIN DefaultTask */
  

  /* Infinite loop */
  for(;;)
  {
    // heartbeat 
    HAL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);
    
    osDelay(1000);
  }
  /* USER CODE END DefaultTask */
}

/* USER CODE BEGIN Header_RFID_DriverTask */
/**
* @brief Function implementing the RFIDDriverTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RFID_DriverTask */
void RFID_DriverTask(void *argument)
{
  /* USER CODE BEGIN RFID_DriverTask */
  uint8_t txBuffer[TX_BUF_SIZE], rxBuffer[RX_BUF_SIZE], rxLen;
  uint8_t status;
  RFID_Item_t rfidItem_QSend;

  /* Infinite loop */
  for(;;)
  {

    // reset Tx & Rx buffer before polling
    RFID_RC522_resetTxRxBuf(txBuffer, rxBuffer);
    
    // poll RFID-RC522
    status = RFID_RC522_poll(txBuffer, rxBuffer, &rxLen);

    switch (status)
    {
      case RFID_OK: // success
        DEBUG_LOG1("RFID_RC522_poll: success\r\n");

        // reset rfidItem_QSend fields
        memset(&rfidItem_QSend, 0, sizeof(rfidItem_QSend));

        // put ATQA in RFID Item
        memcpy(rfidItem_QSend.atqa, rxBuffer, rxLen);

        // reset Tx & Rx buffer before anticollision
        RFID_RC522_resetTxRxBuf(txBuffer, rxBuffer);
        
        status = RFID_RC522_antiColl(txBuffer, rxBuffer, &rxLen);

        switch (status)
        {
          case RFID_OK: // success
            DEBUG_LOG1("RFID_RC522_antiColl: success\r\n");

            // copy UID and its length
            memcpy(rfidItem_QSend.uid, rxBuffer, rxLen);
            rfidItem_QSend.uidLen = rxLen;

            // put rfidItem_QSend in RFIDQueue
            if (osMessageQueuePut(xRFIDQueueHandle, &rfidItem_QSend, 0, 0) != osOK)
              DEBUG_LOG1("RFID_DriverTask: Error: RFIDQueue Full...");
            break;

          case RFID_ERR_ANTICOLL_SEM_TIMEOUT: // antiColl timeout
            DEBUG_LOG1("RFID_RC522_antiColl: RFIDSem timeout\r\n");
            break;

          case RFID_ERR_ANTICOLL_RX: // antiColl readResponse error
            DEBUG_LOG1("RFID_RC522_antiColl: readResponse/Rx error\r\n");
            break;
          
          case RFID_ERR_ANTICOLL_BCC: // BCC check error
            DEBUG_LOG1("RFID_RC522_antiColl: BCC error\r\n");
            break;

          default:
            DEBUG_LOG1("RFID_RC522_antiColl: unknown error\r\n");
        } // switch antiColl status
        
        break;
      
      case RFID_ERR_RES_TIMEOUT: // readResponse timeout
        DEBUG_LOG1("RFID_RC522_readResponse: timeout\r\n");
        break;
      
      case RFID_ERR_RES_ERR_REG: // error
        DEBUG_LOG1("RFID_RC522_readResponse: error\r\n");
        break;
      
      case RFID_ERR_POLL_SEM_TIMEOUT: // poll timeout
        DEBUG_LOG1("RFID_RC522_poll: RFIDSem timeout\r\n");
        break;
      
      default:
        DEBUG_LOG1("RFID_RC522_poll: unknown error\r\n");
    } // switch poll status

    // TODO: implement SELECT + SAK to complete UID Selection (ISO 14443A)

    osDelay(50); // delay between polling 
  }

  /* USER CODE END RFID_DriverTask */
}

/* USER CODE BEGIN Header_RFID_AppTask */
/**
* @brief Function implementing the RFIDAppTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RFID_AppTask */
void RFID_AppTask(void *argument)
{
  /* USER CODE BEGIN RFID_AppTask */
  RFID_Item_t rfidItem_QRecv;

  DEBUG_LOG0("freeRTOS Started\r\n"); 

  for(;;)
  {
    // get rfidItem from RFIDQueue
    osMessageQueueGet(xRFIDQueueHandle, &rfidItem_QRecv, 0, osWaitForever);

    // print rfidItem
    DEBUG_LOG0("---------- RFID ITEM ----------\r\n");
    DEBUG_LOG0("ATQA: ");
    for (uint8_t i = 0; i < RFID_ATQA_LEN; i++)
      DEBUG_LOG0("%02X ", rfidItem_QRecv.atqa[i]);
    DEBUG_LOG0("\r\n");
    
    DEBUG_LOG0("UID:  ");
    for (uint8_t i = 0; i < rfidItem_QRecv.uidLen; i++)
      DEBUG_LOG0("%02X ", rfidItem_QRecv.uid[i]);
    DEBUG_LOG0("\r\n");
    DEBUG_LOG0("-------------------------------\r\n");

    // TODO: authentication logic

    osDelay(100);
  }
  /* USER CODE END RFID_AppTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
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

    //if (status != 0 || tmpLen != 5) // we're expecting 4-bytes UID 1-byte BCC
    if (status != 0)
      return RFID_ERR_ANTICOLL_RX;

    DEBUG_LOG3("AntiColl len: %d\r\n", tmpLen); // debug
    
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

  *rxLen = uidIndex;

  return RFID_OK;
}

void RFID_RC522_resetTxRxBuf(uint8_t *txBuf, uint8_t *rxBuf)
{
  memset(txBuf, 0, TX_BUF_SIZE);
  memset(rxBuf, 0, RX_BUF_SIZE);
}

/* USER CODE END Application */



