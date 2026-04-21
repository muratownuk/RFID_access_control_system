# RFID Access Control System - STM32F411RE + FreeRTOS

## Overview
This project implements an **interrupt-driven RFID access control system** using the STM32F411RE Nucleo-64 board and the MFRC522 RFID-RC522 reader.

The system is built with a **layered architecture** and **FreeRTOS-based multitasking**, demonstrating real-world embedded design patterns used in industry.

### Key Features
- SPI-based communication with MFRC522 RFID-RC522 module
- Interrupt-driven RFID detection (EXTI → ISR → semaphore)
- FreeRTOS multitasking (Driver + Application separation)
- Queue-based inter-task communication
- Anti-collision handling with cascade level support (UID up to 10 bytes)
- UART (USART2) debug logging system
- LED heartbeat task for system monitoring

---

## System Architecture
```
    RFID Card Detected
           |
    EXTI Interrupt (IRQ)
           |
 HAL_GPIO_EXTI_Callback()
           |
   (Semaphore Release)
           |
    RFID Driver Task
           |
 (REQA + Anti-collision)
           |
     Message Queue
           |
    RFID App Task
           |
   (UID Processing)
```

---

## Current Status

### Implemented
- FreeRTOS scheduler with 3 tasks:
  - **Default Task** -> LED heartbeat
  - **RFID Driver Task** -> Handles polling + anti-collision
  - **RFID App Task** -> Processes and prints RFID data
- SPI communication with MFRC522 RFID-RC522
- IRQ-based event handling using EXTI0
- REQA command (card detection)
- Anti-collision algorithm (Cascade Level 1–3 support)
- UID extraction (supports up to 10-byte UIDs)
- Queue-based communication between tasks
- Structured RFID data (`RFID_Item_t`)

---

## Project Structure 
- `Core/`                               # main.c, freertos.c 
- `Drivers/`                            # STM32F4 HAL drivers
- `Middlewares/`                        # FreeRTOS kernel + headers
- `cmake/`                              # CubeMX CMake helper files
- `CMakeLists.txt`                      # Build configuration 
- `build/`                              # Build output (ignored)
- `RFID_access_control_system.ioc`      # CubeMX project
- `startup_stm32f411xe.s`               # MCU startup file
- `STM32F411XX_FLASH.ld`                # Linker script
- `README.md`                           # Project overview
- `LICENSE`                             # MIT License

- `Core/Src/`
  - `rfid_rc522.c`                      # RFID-RC522 driver implementation
  - `freertos.c`                        # RTOS tasks and logic
  - `main.c`                            # System entry point

- `Core/Inc/`
  - `rfid_rc522.h`                      # RFID-RC522 API + data structures
  - `mfrc522_regs.h`                    # MFRC522 register definitions

---

## Hardware

- **MCU:** STM32F411RE (Nucleo-64)
- **RFID Module:** RFID-RC522 (SPI)
- **Interface:**
  - SPI1 -> RFID communication
  - EXTI0 -> RFID-RC522 IRQ pin
  - USART2 -> Debug logging
  - GPIO -> LED (heartbeat) + Relay (future use)

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

### Example Output (UART) using RealTerm
```
System Initialized
freeRTOS Started
-------------------------------
RFID Item Detected!
---------- RFID ITEM ----------
ATQA: 04 00
UID:  91 F7 64 06
-------------------------------
RFID Item Removed.
```

## Next Steps 
- [x] Add PB7 LED heartbeat task using FreeRTOS
- [x] Add USART2 debug prints
- [x] Implement RFID-RC522 SPI driver
- [x] Create IRQ-based RFID reading task
- [x] Add access control logic (relay/door unlock)
- [ ] Add STM32 flash read/write for UID authorization
- [ ] Update documentation with wiring diagrams and task flow

