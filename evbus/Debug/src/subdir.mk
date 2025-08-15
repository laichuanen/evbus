################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ADC.c \
../src/CAN.c \
../src/MPC57xx__Interrupt_Init.c \
../src/PWM.c \
../src/SPI.c \
../src/Vector.c \
../src/adc_smoke_test.c \
../src/can_globals.c \
../src/can_smoke_test.c \
../src/flashrchw.c \
../src/intc_SW_mode_isr_vectors_MPC5744P.c \
../src/main.c \
../src/main_integration.c \
../src/main_smoke.c \
../src/pwm_smoke_test.c \
../src/spi_smoke_test.c 

S_UPPER_SRCS += \
../src/intc_sw_handlers.S 

OBJS += \
./src/ADC.o \
./src/CAN.o \
./src/MPC57xx__Interrupt_Init.o \
./src/PWM.o \
./src/SPI.o \
./src/Vector.o \
./src/adc_smoke_test.o \
./src/can_globals.o \
./src/can_smoke_test.o \
./src/flashrchw.o \
./src/intc_SW_mode_isr_vectors_MPC5744P.o \
./src/intc_sw_handlers.o \
./src/main.o \
./src/main_integration.o \
./src/main_smoke.o \
./src/pwm_smoke_test.o \
./src/spi_smoke_test.o 

C_DEPS += \
./src/ADC.d \
./src/CAN.d \
./src/MPC57xx__Interrupt_Init.d \
./src/PWM.d \
./src/SPI.d \
./src/Vector.d \
./src/adc_smoke_test.d \
./src/can_globals.d \
./src/can_smoke_test.d \
./src/flashrchw.d \
./src/intc_SW_mode_isr_vectors_MPC5744P.d \
./src/main.d \
./src/main_integration.d \
./src/main_smoke.d \
./src/pwm_smoke_test.d \
./src/spi_smoke_test.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@src/ADC.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS Assembler'
	powerpc-eabivle-gcc "@src/intc_sw_handlers.args" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


