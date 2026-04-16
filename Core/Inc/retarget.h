#ifndef __RETARGET_H
#define __RETARGET_H

#include "main.h"
#include "stm32f4xx_hal_uart.h"

/**
    Debug printfs based on level with level 0 being the highest in the 
    hierarchy. If all DEBUG_PRINTFx are 0, then nothing outputs to USART2.
    
 */
#define DEBUG_PRINTF0       1
#define DEBUG_PRINTF1       0
#define DEBUG_PRINTF2       0
#define DEBUG_PRINTF3       0

    #if DEBUG_PRINTF0
        #define DEBUG_LOG0(...) printf(__VA_ARGS__)
    #else 
        #define DEBUG_LOG0(...)
    #endif

    #if DEBUG_PRINTF1
        #define DEBUG_LOG1(...) printf(__VA_ARGS__)
    #else 
        #define DEBUG_LOG1(...)
    #endif

    #if DEBUG_PRINTF2
        #define DEBUG_LOG2(...) printf(__VA_ARGS__)
    #else 
        #define DEBUG_LOG2(...)
    #endif

    #if DEBUG_PRINTF3
        #define DEBUG_LOG3(...) printf(__VA_ARGS__)
    #else 
        #define DEBUG_LOG3(...)
    #endif

int _write(int file, char *ptr, int len); 

#endif

