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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */
typedef struct{
  uint8_t uid[5];   // REQA or full UID
  uint8_t uidLen;   // number of valid bytes
} RFID_Item_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define REQA_CMD                      0x26
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
/* Definitions for RFIDPollTask */
osThreadId_t RFIDPollTaskHandle;
const osThreadAttr_t RFIDPollTask_attributes = {
  .name = "RFIDPollTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for RFIDProcessTask */
osThreadId_t RFIDProcessTaskHandle;
const osThreadAttr_t RFIDProcessTask_attributes = {
  .name = "RFIDProcessTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for xRFIDQueue */
osMessageQueueId_t xRFIDQueueHandle;
uint8_t xRFIDQueueBuffer[ 10 * 10 ];
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

/* USER CODE END FunctionPrototypes */

void DefaultTask(void *argument);
void RFID_PollTask(void *argument);
void RFID_ProcessTask(void *argument);

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
  xRFIDQueueHandle = osMessageQueueNew (10, 10, &xRFIDQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(DefaultTask, NULL, &defaultTask_attributes);

  /* creation of RFIDPollTask */
  RFIDPollTaskHandle = osThreadNew(RFID_PollTask, NULL, &RFIDPollTask_attributes);

  /* creation of RFIDProcessTask */
  RFIDProcessTaskHandle = osThreadNew(RFID_ProcessTask, NULL, &RFIDProcessTask_attributes);

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

/* USER CODE BEGIN Header_RFID_PollTask */
/**
* @brief Function implementing the RFIDPollTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RFID_PollTask */
void RFID_PollTask(void *argument)
{
  /* USER CODE BEGIN RFID_PollTask */
  uint8_t txBuffer[2], rxBuffer[10], rxLen;
  uint8_t status;

  txBuffer[0] = REQA_CMD;

  /* Infinite loop */
  for(;;)
  {

    // start REQA transmission 
    RFID_RC522_startTransceive(txBuffer, 1);

    // wait until ISR (EXTI0 callback) releases semaphore (with timeout)
    if (osSemaphoreAcquire(RFIDSemHandle, RFID_SEM_ACQUIRE_TIMEOUT) != osOK)
      continue; // Timeout! Retrying...
    
    // read ATQA reception
    status = RFID_RC522_readResponse(rxBuffer, &rxLen);

    switch (status)
    {
      case 0: // success
        printf("RFID_PollTask: success\r\n");

        printf("rxBuffer: ");
        for (uint8_t i = 0; i < rxLen; i++)
          printf("%02X ", rxBuffer[i]);
        printf("\r\n");

        // push UID/ATQA into queue for RFID_ProcessTask
        osMessageQueuePut(xRFIDQueueHandle, rxBuffer, 0, 0);
        break;
      
      case 1: // timeout
        printf("RFID_PollTask: timeout\r\n");
        break;
      
      case 3: // error
        printf("RFID_PollTask: error\r\n");
        break;
      
      default:
        printf("RFID_PollTask: unknown error\r\n");
    }
    
    osDelay(50); // delay between polling 
  }
  /* USER CODE END RFID_PollTask */
}

/* USER CODE BEGIN Header_RFID_ProcessTask */
/**
* @brief Function implementing the RFIDProcessTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RFID_ProcessTask */
void RFID_ProcessTask(void *argument)
{
  /* USER CODE BEGIN RFID_ProcessTask */
  printf("freeRTOS Started\r\n"); 

  for(;;)
  {

    osDelay(1);
  }
  /* USER CODE END RFID_ProcessTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

