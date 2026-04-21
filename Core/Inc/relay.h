/**
  *****************************************************************************
  * @file               : relay.h
  * @brief              : header for relay.c file.
  *****************************************************************************
 */
#ifndef __RELAY_H
#define __RELAY_H
#include "main.h"
#include <stdint.h>

// - TYPEDEFS - 
// typedef enum
typedef enum 
{
    RELAY_CMD_LOCK,
    RELAY_CMD_UNLOCK
} RelayCmd_t;

// typedef struct
typedef struct
{
    RelayCmd_t cmd;             // lock/unlock command
    uint32_t duration_ms;       // how long to unlock
} RelayMessage_t;

// - DEFINES -
#define ACCESS_UNLOCK_TIME_MS       3000

// - FUNCTION PROTOTYPES -
void turnRelayOFF(void);
void turnRelayON(void);
void resetRelay(void);

#endif


