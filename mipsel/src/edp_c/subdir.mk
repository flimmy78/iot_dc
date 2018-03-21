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
	/home/zhuang/openwrt/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-gcc -I"/home/zhuang/workspace/iot_dc/inc" -I"/home/zhuang/workspace/iot_dc/inc/edp_c" -I"/home/zhuang/workspace/iot_dc/src/IotCommon" -I"/home/zhuang/workspace/iot_dc/src/IotCommon/cJSON" -I"/home/zhuang/openwrt/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


