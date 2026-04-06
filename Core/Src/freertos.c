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
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "retarget.h"
#include "rfid_rc522.h"
#include <stdint.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern volatile uint8_t rfid_irq_flag; 

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RFID_RC522_Task */
osThreadId_t RFID_RC522_TaskHandle;
const osThreadAttr_t RFID_RC522_Task_attributes = {
  .name = "RFID_RC522_Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void Start_RFID_RC522_Task(void *argument);

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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of RFID_RC522_Task */
  RFID_RC522_TaskHandle = osThreadNew(Start_RFID_RC522_Task, NULL, &RFID_RC522_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  printf("RTOS Started\r\n"); 

  /* Infinite loop */
  for(;;)
  {
    /* old heartbeat led 
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
    printf("Tick\r\n");
    
    osDelay(1000);
    */
    /* for later use ... 
    if (rfid_irq_flag)
    {
      rfid_irq_flag = 0;

      printf("Card detected!\r\n");
      HAL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);
    } 
    */    
    osDelay(1); 

  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_Start_RFID_RC522_Task */
/**
* @brief Function implementing the RFID_RC522_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_RFID_RC522_Task */
void Start_RFID_RC522_Task(void *argument)
{
  /* USER CODE BEGIN Start_RFID_RC522_Task */
  uint8_t tag[5]; // REQA response 2 bytes, ATQA: UID implementation later...

  /* Infinite loop */
  for(;;)
  {
    printf("RFID \r\n");

    if (rfid_irq_flag)
    {
      printf("rfid_irq_flag \r\n");
      rfid_irq_flag = 0; // clear the flag 

      uint8_t status = RFID_RC522_Request(tag);

      switch (status)
      {
        case 0: // success
          printf("Card Detected: ");
          for (uint8_t i = 0; i < 2; i++) // backLen = 2 bytes for REQA
            printf("%02X ", tag[i]); 
          printf("\r\n");
          break;

        case 1: // timeout / no card
          printf("RFID Request: Timeout / No Card\r\n");
          break;
        
        case 2: // error
          printf("RFID Request: Transceive Error\r\n");
          break;

        default:
          printf("RFID Request: Unknown Status %d\r\n", status);
      }
    }
    osDelay(1000); // yield to other tasks
  }

  /* USER CODE END Start_RFID_RC522_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

