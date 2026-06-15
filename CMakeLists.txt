cmake_minimum_required(VERSION 3.20)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TOOLCHAIN_DIR "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin")
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/arm-none-eabi-gcc.exe)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/arm-none-eabi-gcc.exe)
set(CMAKE_OBJCOPY ${TOOLCHAIN_DIR}/arm-none-eabi-objcopy.exe)
set(CMAKE_SIZE ${TOOLCHAIN_DIR}/arm-none-eabi-size.exe)

project(GetTemp_Hum C ASM)

set(CMAKE_C_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32F103xB -DUSE_HAL_DRIVER -std=gnu11 -Wall -Wextra -Wno-unused-parameter -O2 -ffunction-sections -fdata-sections")
set(CMAKE_ASM_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -x assembler-with-cpp -DSTM32F103xB")

set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -T${CMAKE_SOURCE_DIR}/STM32F103XB_FLASH.ld -Wl,--gc-sections -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,-u,_printf_float --specs=nano.specs --specs=nosys.specs")

add_definitions(-DSTM32F103xB -DUSE_HAL_DRIVER)

file(GLOB_RECURSE HAL_SOURCES
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
)

set(CORE_SOURCES
  ${CMAKE_SOURCE_DIR}/Core/Src/main.c
  ${CMAKE_SOURCE_DIR}/Core/Src/dht12.c
  ${CMAKE_SOURCE_DIR}/Core/Src/gpio.c
  ${CMAKE_SOURCE_DIR}/Core/Src/i2c.c
  ${CMAKE_SOURCE_DIR}/Core/Src/usart.c
  ${CMAKE_SOURCE_DIR}/Core/Src/stm32f1xx_hal_msp.c
  ${CMAKE_SOURCE_DIR}/Core/Src/stm32f1xx_it.c
  ${CMAKE_SOURCE_DIR}/Core/Src/system_stm32f1xx.c
)

set(STARTUP_SOURCE
  ${CMAKE_SOURCE_DIR}/startup_stm32f103xb.s
)

add_executable(${CMAKE_PROJECT_NAME}.elf
  ${HAL_SOURCES}
  ${CORE_SOURCES}
  ${STARTUP_SOURCE}
)

target_include_directories(${CMAKE_PROJECT_NAME}.elf PRIVATE
  ${CMAKE_SOURCE_DIR}/Core/Inc
  ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
  ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
  COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf
  COMMENT "Generating .hex and .bin files"
)
