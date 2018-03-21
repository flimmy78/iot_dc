################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/edp_c/EdpKit.c \
../src/edp_c/Openssl.c 

OBJS += \
./src/edp_c/EdpKit.o \
./src/edp_c/Openssl.o 

C_DEPS += \
./src/edp_c/EdpKit.d \
./src/edp_c/Openssl.d 


# Each subdirectory must supply rules for building sources it contributes
src/edp_c/%.o: ../src/edp_c/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/zhuang/workspace/iot_dc/inc" -I"/home/zhuang/workspace/iot_dc/src/IotCommon" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


