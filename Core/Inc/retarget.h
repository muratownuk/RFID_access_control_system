#ifndef __RETARGET_H
#define __RETARGET_H

#include "main.h"
#include "stm32f4xx_hal_uart.h"

#define DEBUG_PRINTF     1

    #if DEBUG_PRINTF
        #define DEBUG_LOG(...) printf(__VA_ARGS__)
    #else 
        #define DEBUG_LOG(...)
    #endif

int _write(int file, char *ptr, int len); 

#endif


