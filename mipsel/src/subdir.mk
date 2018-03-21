################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CHttpClient.cpp \
../src/DeviceController.cpp \
../src/DeviceModel.cpp \
../src/NotifyServer.cpp \
../src/ZigbeeController.cpp \
../src/dc_common.cpp \
../src/gc_thread.cpp \
../src/main.cpp \
../src/onenet.cpp \
../src/sql_ops.cpp 

OBJS += \
./src/CHttpClient.o \
./src/DeviceController.o \
./src/DeviceModel.o \
./src/NotifyServer.o \
./src/ZigbeeController.o \
./src/dc_common.o \
./src/gc_thread.o \
./src/main.o \
./src/onenet.o \
./src/sql_ops.o 

CPP_DEPS += \
./src/CHttpClient.d \
./src/DeviceController.d \
./src/DeviceModel.d \
./src/NotifyServer.d \
./src/ZigbeeController.d \
./src/dc_common.d \
./src/gc_thread.d \
./src/main.d \
./src/onenet.d \
./src/sql_ops.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	/home/zhuang/openwrt/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-g++ -I"/home/zhuang/workspace/iot_dc/inc" -I"/home/zhuang/workspace/iot_dc/src/IotCommon" -I"/home/zhuang/workspace/iot_dc/src/IotCommon/cJSON" -I"/home/zhuang/openwrt/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


