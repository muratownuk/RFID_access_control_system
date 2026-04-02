# RFID Access Control System - STM32F411RE + FreeRTOS Skeleton

## Overview
This is the initial CubeMX + FreeRTOS skeleton for an RFID-based access control system using the STM32F411RE Nucleo-64 board.  

This repository provides a **starting point** for building:  
- RFID SPI communication with RFID-RC522  
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
- No functional tasks yet (RFID task, and SPI not implemented)

---

## Project Structure 
- `Core/`                               # main.c, freertos.c 
- `Drivers/`                            # STM32F4 HAL drivers
- `Middlewares/`                        # FreeRTOS kernel + headers
- `cmake/`                              # CubeMX CMake helper files
- `CMakeLists.txt`                      # CMake list 
- `build/`                              # CMake output (ignored)
- `RFID_access_control_system.ioc`      # CubeMX project
- `startup_stm32f411xe.s`               # MCU startup file
- `STM32F411XX_FLASH.ld`                # Linker script
- `README.md`                           # Project overview
- `LICENSE`                             # MIT License


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

```
### Using Visual Studio Code 
- Open project folder in VSCode
- Use CMake extension to configure and build 
- Flash via ST-Link (start debugging choosing STM32Cube: STM32 Launch STLink 
  GDB Server)

## Next Steps 
- [x] Add PB7 LED heartbeat task using FreeRTOS
- [x] Add USART2 debug prints
- [ ] Implement RFID-RC522 SPI driver
- [ ] Create IRQ-based RFID reading task
- [ ] Implement key fob authentication logic
- [ ] Update documentation with wiring diagrams and task flow

