/**
  *****************************************************************************
  * @file               : relay.c
  * @brief              : relay driver through STM32 GPIO
  *****************************************************************************
 */
// includes
#include "relay.h"
#include "main.h"
#include "stm32f4xx_hal_gpio.h"

// - FUNCTIONS - 
void turnRelayOFF(void)
{
    // turn green LED OFF
    HAL_GPIO_WritePin(LED_UNLOCKED_G_GPIO_Port, LED_UNLOCKED_G_Pin, GPIO_PIN_RESET);

    // turn relay OFF
    HAL_GPIO_WritePin(OUTPUT_RELAY_GPIO_Port, OUTPUT_RELAY_Pin, GPIO_PIN_RESET);

    // turn red LED ON
    HAL_GPIO_WritePin(LED_LOCKED_R_GPIO_Port, LED_LOCKED_R_Pin, GPIO_PIN_SET);
}

void turnRelayON(void)
{
    // turn green LED ON
    HAL_GPIO_WritePin(LED_UNLOCKED_G_GPIO_Port, LED_UNLOCKED_G_Pin, GPIO_PIN_SET);

    // turn relay ON
    HAL_GPIO_WritePin(OUTPUT_RELAY_GPIO_Port, OUTPUT_RELAY_Pin, GPIO_PIN_SET);

    // turn red LED OFF
    HAL_GPIO_WritePin(LED_LOCKED_R_GPIO_Port, LED_LOCKED_R_Pin, GPIO_PIN_RESET);
}

void resetRelay(void)
{
    // turn green LED OFF if ON
    if (HAL_GPIO_ReadPin(LED_UNLOCKED_G_GPIO_Port,  LED_UNLOCKED_G_Pin) == \
    GPIO_PIN_SET)
        HAL_GPIO_WritePin(LED_UNLOCKED_G_GPIO_Port, LED_UNLOCKED_G_Pin, \
            GPIO_PIN_RESET);
    
    // turn relay OFF if ON
    if (HAL_GPIO_ReadPin(OUTPUT_RELAY_GPIO_Port, OUTPUT_RELAY_Pin) == \
    GPIO_PIN_SET)
        HAL_GPIO_WritePin(OUTPUT_RELAY_GPIO_Port, OUTPUT_RELAY_Pin, \
            GPIO_PIN_RESET);
        
    // turn red LED ON if OFF
    if (HAL_GPIO_ReadPin(LED_LOCKED_R_GPIO_Port, LED_LOCKED_R_Pin) == \
    GPIO_PIN_RESET)
        HAL_GPIO_WritePin(LED_LOCKED_R_GPIO_Port, LED_LOCKED_R_Pin, \
            GPIO_PIN_SET);
}



