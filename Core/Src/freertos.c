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
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "retarget.h"
//#include "stm32f4xx_hal_def.h"
#include "flash_storage.h"
#include "rfid_services.h"
#include "relay.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */
typedef enum 
{
  ACCESS_DENIED,
  ACCESS_GRANTED
} Access_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// timeouts and delays 
#define RFID_APP_TASK_DELAY           100

// LED defs
#define LED_TOGGLE_DELAY              125
#define LED_BLIP_COUNT_1              1
#define LED_BLIP_COUNT_2              2

// flash (temp) 
#define WHITELIST_MAX                 5
#define PROGRAM_FLASH                 0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static uint8_t whiteList[WHITELIST_MAX][RFID_MAX_UID_LEN] = 
{
  {0x91, 0xF7, 0x64, 0x06},
  {0x08, 0xF5, 0x23, 0xB7}, 
  {0x12, 0x34, 0x56, 0x78}
};

static uint8_t whiteList_len[WHITELIST_MAX] = 
{
  4,
  4,
  4
};

/* USER CODE END Variables */
/* Definitions for relayTask */
osThreadId_t relayTaskHandle;
const osThreadAttr_t relayTask_attributes = {
  .name = "relayTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh1,
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
/* Definitions for xRelayQueue */
osMessageQueueId_t xRelayQueueHandle;
uint8_t xRelayQueueBuffer[ 5 * 8 ];
osStaticMessageQDef_t xRelayQueueControlBlock;
const osMessageQueueAttr_t xRelayQueue_attributes = {
  .name = "xRelayQueue",
  .cb_mem = &xRelayQueueControlBlock,
  .cb_size = sizeof(xRelayQueueControlBlock),
  .mq_mem = &xRelayQueueBuffer,
  .mq_size = sizeof(xRelayQueueBuffer)
};
/* Definitions for RFIDSem */
osSemaphoreId_t RFIDSemHandle;
const osSemaphoreAttr_t RFIDSem_attributes = {
  .name = "RFIDSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void blipGLED(uint8_t blipCount);
void blipRLED(uint8_t blipCount);

//static Access_t isUIDAuthorized(uint8_t *uid, uint8_t uidLen);
void addUIDData_Init(void);

/* USER CODE END FunctionPrototypes */

void RelayTask(void *argument);
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
  addUIDData_Init();

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

  /* creation of xRelayQueue */
  xRelayQueueHandle = osMessageQueueNew (5, 8, &xRelayQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of relayTask */
  relayTaskHandle = osThreadNew(RelayTask, NULL, &relayTask_attributes);

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

/* USER CODE BEGIN Header_RelayTask */
/**
  * @brief  Function implementing the relayTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_RelayTask */
void RelayTask(void *argument)
{
  /* USER CODE BEGIN RelayTask */
  RelayMessage_t relayMsg_QRecv;

  /* Infinite loop */
  for(;;)
  {
    osMessageQueueGet(xRelayQueueHandle, &relayMsg_QRecv, 0, osWaitForever);

    if (relayMsg_QRecv.cmd == RELAY_CMD_LOCK)
    {
      blipRLED(LED_BLIP_COUNT_2);
      resetRelay();
    }
    else if (relayMsg_QRecv.cmd == RELAY_CMD_UNLOCK )
    {
      // turn relay on to unlock
      turnRelayON();

      // keep door unlocked for .duration_ms
      vTaskDelay(pdMS_TO_TICKS(relayMsg_QRecv.duration_ms));

      // turn relay off and lock
      turnRelayOFF();
    }
    else
    {
      DEBUG_LOG0("RelayTask: Unknown RelayCmd_t");
    }

  }

  /* USER CODE END RelayTask */
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
  static TickType_t lastWakeTime;
  lastWakeTime = xTaskGetTickCount();

  static uint8_t lastUID[10] = {0};
  static uint8_t lastUIDLen = 0;

  static TickType_t lastSeenTick = 0, now;
  static RFID_ItemEvent_t evt = RFID_ITEM_REMOVED; 

  uint8_t txBuffer[TX_BUF_SIZE], rxBuffer[RX_BUF_SIZE], rxLen;
  RFID_Status_t status;
  RFID_Item_t rfidItem_QSend;

  /* Infinite loop */
  for(;;)
  {
    // poll schedule
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(RFID_POLL_PERIOD));

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
        if (rxLen == 2)
          memcpy(rfidItem_QSend.atqa, rxBuffer, rxLen);
        else
        {
          DEBUG_LOG1("RFID_RC522_poll: ATQA length error\r\n");
          continue;
        }

        // reset Tx & Rx buffer before anticollision
        RFID_RC522_resetTxRxBuf(txBuffer, rxBuffer);
        
        status = RFID_RC522_antiColl(txBuffer, rxBuffer, &rxLen);

        switch (status)
        {
          case RFID_OK: // success
            DEBUG_LOG1("RFID_RC522_antiColl: success\r\n");

            lastSeenTick = xTaskGetTickCount();
            evt = RFID_ITEM_PRESENT;

            rfidItem_QSend.event = (uint8_t) evt;

            // copy UID and its length
            memcpy(rfidItem_QSend.uid, rxBuffer, rxLen);
            rfidItem_QSend.uidLen = rxLen;

            // duplicate filter
            if ((rfidItem_QSend.uidLen == lastUIDLen) && (memcmp(rfidItem_QSend.uid, lastUID, lastUIDLen) == 0))
            {
              // same tag... ignore
              break;
            }
            else
            {
              memcpy(lastUID, rfidItem_QSend.uid, rfidItem_QSend.uidLen);
              lastUIDLen = rfidItem_QSend.uidLen;
              
              // put rfidItem_QSend in RFIDQueue
              if (osMessageQueuePut(xRFIDQueueHandle, &rfidItem_QSend, 0, 0) != osOK)
                DEBUG_LOG1("RFID_DriverTask: Error: RFIDQueue Full...");
              
              break;
            }
            
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

    // detect RFID item removal
    now = xTaskGetTickCount();

    if ((evt == RFID_ITEM_PRESENT) && ((now - lastSeenTick) > \
    pdMS_TO_TICKS(RFID_ITEM_REMOVE_TIMEOUT)))
    {
      evt = RFID_ITEM_REMOVED;

      memset(&rfidItem_QSend, 0, sizeof(rfidItem_QSend));
      rfidItem_QSend.event = (uint8_t) evt;

      // put rfidItem_QSend in RFIDQueue
      if (osMessageQueuePut(xRFIDQueueHandle, &rfidItem_QSend, 0, 0) != osOK)
        DEBUG_LOG1("RFID_DriverTask: Error: RFIDQueue Full...");

      memset(lastUID, 0, sizeof(lastUID));
      lastUIDLen = 0;
    }

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
  DEBUG_LOG0("freeRTOS Started\r\n");
  DEBUG_LOG0("-------------------------------\r\n");
  
  RFID_Item_t rfidItem_QRecv;
  RFID_ItemEvent_t event = RFID_ITEM_REMOVED;
  RelayMessage_t relayMsg_QSend;
  Access_t auth = ACCESS_DENIED;

  /* Infinite loop */
  for(;;)
  {
    // get rfidItem from RFIDQueue
    osMessageQueueGet(xRFIDQueueHandle, &rfidItem_QRecv, 0, osWaitForever);

    event = (RFID_ItemEvent_t) rfidItem_QRecv.event;

    if (event == RFID_ITEM_PRESENT)
    {
      // print rfidItem
      DEBUG_LOG0("RFID Item Detected!\r\n");
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

      /*
      // check whiteList
      auth = isUIDAuthorized(rfidItem_QRecv.uid, rfidItem_QRecv.uidLen);
      */

      // check UID database in flash
      if (FlashStorage_Exists(rfidItem_QRecv.uid, rfidItem_QRecv.uidLen))
        auth = ACCESS_GRANTED;
      else
        auth = ACCESS_DENIED;

      if (auth == ACCESS_DENIED)
      {
        DEBUG_LOG0("Access Denied!\r\n");

        // send lock message to RelayTask
        relayMsg_QSend.cmd = RELAY_CMD_LOCK;
        relayMsg_QSend.duration_ms = 0;

        if (osMessageQueuePut(xRelayQueueHandle, &relayMsg_QSend, 0, 0) != osOK)
          DEBUG_LOG1("RFID_AppTask: Error: RelayQueue Full...\r\n");
      }
      else if (auth == ACCESS_GRANTED)
      {
        DEBUG_LOG0("Access Granted!\r\n");

        // send unlock message to RelayTask
        relayMsg_QSend.cmd = RELAY_CMD_UNLOCK;
        relayMsg_QSend.duration_ms = ACCESS_UNLOCK_TIME_MS;

        if (osMessageQueuePut(xRelayQueueHandle, &relayMsg_QSend, 0, 0) != osOK)
          DEBUG_LOG1("RFID_AppTask: Error: RelayQueue Full...\r\n");
      }
      else 
        DEBUG_LOG0("RFID_AppTask: Error: Incorrect Access_t...\r\n");

    }
    else if (event == RFID_ITEM_REMOVED)
    {
      DEBUG_LOG0("RFID Item Removed.\r\n");
      blipRLED(LED_BLIP_COUNT_1); // blip red LED to show removal
    }
    else
      DEBUG_LOG0("RFID_AppTask: Error: Incorrect RFID_ItemEvent_t...\r\n");

    osDelay(RFID_APP_TASK_DELAY);
  }
  /* USER CODE END RFID_AppTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void blipGLED(uint8_t blipCount)
{
  for (uint8_t i = 0; i < blipCount; i++)
  {
      toggleGLED();                       // ON
      osDelay(LED_TOGGLE_DELAY);

      toggleGLED();                       // OFF
      osDelay(LED_TOGGLE_DELAY);
  }
}

void blipRLED(uint8_t blipCount)
{
  for (uint8_t i = 0; i < blipCount; i++)
  {
    toggleRLED();                       // ON
    osDelay(LED_TOGGLE_DELAY);

    toggleRLED();                       // OFF
    osDelay(LED_TOGGLE_DELAY);
  }
}

void addUIDData_Init(void)
{
  if (PROGRAM_FLASH)
  {
    for (uint8_t i = 0; i < 3; i++)
    {
      if (FlashStorage_Add(whiteList[i], whiteList_len[i]) != HAL_OK)
        return;
      DEBUG_LOG0("UID data %d added to flash.\r\n", i);
    }

    DEBUG_LOG0("All UID data added to flash.\r\n");
  }
}

/*
static Access_t isUIDAuthorized(uint8_t *uid, uint8_t uidLen)
{
  for (uint8_t i = 0; i < WHITELIST_MAX; i++)
  {
    if ((uidLen == whiteList_len[i]) && (memcmp(uid, whiteList[i], uidLen) == 0))
      return ACCESS_GRANTED; // authorized
  }
  return ACCESS_DENIED; // not authorized
}
*/

/* USER CODE END Application */



