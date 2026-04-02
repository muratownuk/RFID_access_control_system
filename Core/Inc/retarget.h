#ifndef __RETARGET_H
#define __RETARGET_H

#include "main.h"
#include "stm32f4xx_hal_uart.h"

int _write(int file, char *ptr, int len); 

#endif