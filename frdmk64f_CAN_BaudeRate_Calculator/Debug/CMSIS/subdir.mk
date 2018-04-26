################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CMSIS/system_MK64F12.c 

OBJS += \
./CMSIS/system_MK64F12.o 

C_DEPS += \
./CMSIS/system_MK64F12.d 


# Each subdirectory must supply rules for building sources it contributes
CMSIS/%.o: ../CMSIS/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DPRINTF_FLOAT_ENABLE=0 -DSDK_DEBUGCONSOLE_UART -DFRDM_K64F -DCR_INTEGER_PRINTF -DFREEDOM -DSDK_DEBUGCONSOLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DCPU_MK64FN1M0VLL12_cm4 -DCPU_MK64FN1M0VLL12 -I"C:\Users\felic\Documents\MCUXpressoIDE_10.1.1_606\workspace\frdmk64f_CAN_BaudeRate_Calculator_fast\board" -I"C:\Users\felic\Documents\MCUXpressoIDE_10.1.1_606\workspace\frdmk64f_CAN_BaudeRate_Calculator_fast\source" -I"C:\Users\felic\Documents\MCUXpressoIDE_10.1.1_606\workspace\frdmk64f_CAN_BaudeRate_Calculator_fast" -I"C:\Users\felic\Documents\MCUXpressoIDE_10.1.1_606\workspace\frdmk64f_CAN_BaudeRate_Calculator_fast\drivers" -I"C:\Users\felic\Documents\MCUXpressoIDE_10.1.1_606\workspace\frdmk64f_CAN_BaudeRate_Calculator_fast\CMSIS" -I"C:\Users\felic\Documents\MCUXpressoIDE_10.1.1_606\workspace\frdmk64f_CAN_BaudeRate_Calculator_fast\utilities" -I"C:\Users\felic\Documents\MCUXpressoIDE_10.1.1_606\workspace\frdmk64f_CAN_BaudeRate_Calculator_fast\startup" -O0 -fno-common -g -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


