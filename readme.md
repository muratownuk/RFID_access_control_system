# RFID Access Control System - STM32F411RE + FreeRTOS Skeleton

## Overview
This is the initial CubeMX + FreeRTOS skeleton for an RFID-based access control system using the STM32F411RE Nucleo-64 board.  

This repository provides a **starting point** for building:  
- RFID SPI communication with MFRC522  
- FreeRTOS-based multitasking  
- LED heartbeat and debug via USART2  
- Incremental development toward a full access control system

---

## Current Status
- CubeMX project configured with:  
  - STM32F411RE peripherals (GPIO, USART2 for debug, SPI for RFID)  
  - FreeRTOS enabled with default task templates  
- Project compiles successfully with CMake  
- Scheduler is running (`osKernelStart()`)  
- No functional tasks yet (heartbeat LED, RFID task, and SPI not implemented)

---

## Project Structure 
Core/                               # main.c, freertos.c templates
Drivers/                            # STM32F4 HAL drivers
Middlewares/                        # FreeRTOS kernel + headers
CMake/                              # CubeMX CMake helper files
build/                              # CMake output (ignored)
README.md                           # Project overview
RFID_access_control_system.ioc      # CubeMX project
startup_stm32f411xe.s               # MCU startup file
STM32F411XX_FLASH.ld                # Linker script


---

## Build Instructions

### Using Command Line (Windows)
```bash
# Go to project root
cd path/to/RFID_access_control_system

# Create build folder
mkdir build && cd build

# Configure project
cmake ..

# Build project
cmake --build .

# Flash using ST-Link
st-flash write build/RFID_access_control_system.bin 0x8000000

# Or just build via Start Debugging in Visual Studio Code, choosing 
# STM32Cube: STM32 Launch STLink GDB Server 