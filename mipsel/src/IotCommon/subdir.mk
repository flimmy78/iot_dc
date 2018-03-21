################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/IotCommon/RgbSpaceMatrices.c \
../src/IotCommon/atoi.c \
../src/IotCommon/blackbody.c \
../src/IotCommon/colorConv.c \
../src/IotCommon/dump.c \
../src/IotCommon/fileCreate.c \
../src/IotCommon/gateway.c \
../src/IotCommon/iotError.c \
../src/IotCommon/iotSemaphore.c \
../src/IotCommon/iotSleep.c \
../src/IotCommon/iotTimer.c \
../src/IotCommon/json.c \
../src/IotCommon/jsonCreate.c \
../src/IotCommon/newDb.c \
../src/IotCommon/newLog.c \
../src/IotCommon/nibbles.c \
../src/IotCommon/plugUsage.c \
../src/IotCommon/queue.c \
../src/IotCommon/socket.c \
../src/IotCommon/strtoupper.c \
../src/IotCommon/systemtable.c 

OBJS += \
./src/IotCommon/RgbSpaceMatrices.o \
./src/IotCommon/atoi.o \
./src/IotCommon/blackbody.o \
./src/IotCommon/colorConv.o \
./src/IotCommon/dump.o \
./src/IotCommon/fileCreate.o \
./src/IotCommon/gateway.o \
./src/IotCommon/iotError.o \
./src/IotCommon/iotSemaphore.o \
./src/IotCommon/iotSleep.o \
./src/IotCommon/iotTimer.o \
./src/IotCommon/json.o \
./src/IotCommon/jsonCreate.o \
./src/IotCommon/newDb.o \
./src/IotCommon/newLog.o \
./src/IotCommon/nibbles.o \
./src/IotCommon/plugUsage.o \
./src/IotCommon/queue.o \
./src/IotCommon/socket.o \
./src/IotCommon/strtoupper.o \
./src/IotCommon/systemtable.o 

C_DEPS += \
./src/IotCommon/RgbSpaceMatrices.d \
./src/IotCommon/atoi.d \
./src/IotCommon/blackbody.d \
./src/IotCommon/colorConv.d \
./src/IotCommon/dump.d \
./src/IotCommon/fileCreate.d \
./src/IotCommon/gateway.d \
./src/IotCommon/iotError.d \
./src/IotCommon/iotSemaphore.d \
./src/IotCommon/iotSleep.d \
./src/IotCommon/iotTimer.d \
./src/IotCommon/json.d \
./src/IotCommon/jsonCreate.d \
./src/IotCommon/newDb.d \
./src/IotCommon/newLog.d \
./src/IotCommon/nibbles.d \
./src/IotCommon/plugUsage.d \
./src/IotCommon/queue.d \
./src/IotCommon/socket.d \
./src/IotCommon/strtoupper.d \
./src/IotCommon/systemtable.d 


# Each subdirectory must supply rules for building sources it contributes
src/IotCommon/%.o: ../src/IotCommon/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	/home/zhuang/openwrt/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-gcc -I"/home/zhuang/workspace/iot_dc/inc" -I"/home/zhuang/workspace/iot_dc/inc/edp_c" -I"/home/zhuang/workspace/iot_dc/src/IotCommon" -I"/home/zhuang/workspace/iot_dc/src/IotCommon/cJSON" -I"/home/zhuang/openwrt/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


