################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../brewing_kettle/subroutine.c 

OBJS += \
./brewing_kettle/subroutine.o 

C_DEPS += \
./brewing_kettle/subroutine.d 


# Each subdirectory must supply rules for building sources it contributes
brewing_kettle/subroutine.o: ../brewing_kettle/subroutine.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"E:/projekciki/stm32/praca_przejsciowa/brewing_kettle" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"brewing_kettle/subroutine.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

