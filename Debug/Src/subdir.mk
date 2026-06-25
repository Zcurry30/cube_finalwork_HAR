################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Src/startup_stm32f767xx.s 

C_SRCS += \
../Src/app_x-cube-ai.c \
../Src/eth.c \
../Src/gpio.c \
../Src/har_app.c \
../Src/har_io_uart.c \
../Src/main.c \
../Src/network_3.c \
../Src/network_3_data.c \
../Src/network_3_data_params.c \
../Src/sdmmc.c \
../Src/stm32f7xx_hal_msp.c \
../Src/stm32f7xx_it.c \
../Src/syscalls.c \
../Src/system_stm32f7xx.c \
../Src/usart.c \
../Src/usb_device.c \
../Src/usbd_cdc_if.c \
../Src/usbd_conf.c \
../Src/usbd_desc.c 

OBJS += \
./Src/app_x-cube-ai.o \
./Src/eth.o \
./Src/gpio.o \
./Src/har_app.o \
./Src/har_io_uart.o \
./Src/main.o \
./Src/network_3.o \
./Src/network_3_data.o \
./Src/network_3_data_params.o \
./Src/sdmmc.o \
./Src/startup_stm32f767xx.o \
./Src/stm32f7xx_hal_msp.o \
./Src/stm32f7xx_it.o \
./Src/syscalls.o \
./Src/system_stm32f7xx.o \
./Src/usart.o \
./Src/usb_device.o \
./Src/usbd_cdc_if.o \
./Src/usbd_conf.o \
./Src/usbd_desc.o 

S_DEPS += \
./Src/startup_stm32f767xx.d 

C_DEPS += \
./Src/app_x-cube-ai.d \
./Src/eth.d \
./Src/gpio.d \
./Src/har_app.d \
./Src/har_io_uart.d \
./Src/main.d \
./Src/network_3.d \
./Src/network_3_data.d \
./Src/network_3_data_params.d \
./Src/sdmmc.d \
./Src/stm32f7xx_hal_msp.d \
./Src/stm32f7xx_it.d \
./Src/syscalls.d \
./Src/system_stm32f7xx.d \
./Src/usart.d \
./Src/usb_device.d \
./Src/usbd_cdc_if.d \
./Src/usbd_conf.d \
./Src/usbd_desc.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../Inc -I"E:/Desktop/HAR/har_project/Middlewares/ST/AI/Inc" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/%.o: ../Src/%.s Src/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -DDEBUG -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/app_x-cube-ai.cyclo ./Src/app_x-cube-ai.d ./Src/app_x-cube-ai.o ./Src/app_x-cube-ai.su ./Src/eth.cyclo ./Src/eth.d ./Src/eth.o ./Src/eth.su ./Src/gpio.cyclo ./Src/gpio.d ./Src/gpio.o ./Src/gpio.su ./Src/har_app.cyclo ./Src/har_app.d ./Src/har_app.o ./Src/har_app.su ./Src/har_io_uart.cyclo ./Src/har_io_uart.d ./Src/har_io_uart.o ./Src/har_io_uart.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/network_3.cyclo ./Src/network_3.d ./Src/network_3.o ./Src/network_3.su ./Src/network_3_data.cyclo ./Src/network_3_data.d ./Src/network_3_data.o ./Src/network_3_data.su ./Src/network_3_data_params.cyclo ./Src/network_3_data_params.d ./Src/network_3_data_params.o ./Src/network_3_data_params.su ./Src/sdmmc.cyclo ./Src/sdmmc.d ./Src/sdmmc.o ./Src/sdmmc.su ./Src/startup_stm32f767xx.d ./Src/startup_stm32f767xx.o ./Src/stm32f7xx_hal_msp.cyclo ./Src/stm32f7xx_hal_msp.d ./Src/stm32f7xx_hal_msp.o ./Src/stm32f7xx_hal_msp.su ./Src/stm32f7xx_it.cyclo ./Src/stm32f7xx_it.d ./Src/stm32f7xx_it.o ./Src/stm32f7xx_it.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/system_stm32f7xx.cyclo ./Src/system_stm32f7xx.d ./Src/system_stm32f7xx.o ./Src/system_stm32f7xx.su ./Src/usart.cyclo ./Src/usart.d ./Src/usart.o ./Src/usart.su ./Src/usb_device.cyclo ./Src/usb_device.d ./Src/usb_device.o ./Src/usb_device.su ./Src/usbd_cdc_if.cyclo ./Src/usbd_cdc_if.d ./Src/usbd_cdc_if.o ./Src/usbd_cdc_if.su ./Src/usbd_conf.cyclo ./Src/usbd_conf.d ./Src/usbd_conf.o ./Src/usbd_conf.su ./Src/usbd_desc.cyclo ./Src/usbd_desc.d ./Src/usbd_desc.o ./Src/usbd_desc.su

.PHONY: clean-Src

