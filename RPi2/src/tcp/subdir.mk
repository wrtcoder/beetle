################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/tcp/TCPDeviceServer.cpp 

OBJS += \
./src/tcp/TCPDeviceServer.o 

CPP_DEPS += \
./src/tcp/TCPDeviceServer.d 


# Each subdirectory must supply rules for building sources it contributes
src/tcp/%.o: ../src/tcp/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -D__cplusplus=201402L -I../lib/include -I../include -O1 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


