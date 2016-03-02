################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Systems/Beacon.cpp \
../src/Systems/Drive.cpp \
../src/Systems/MPC.cpp \
../src/Systems/Operator.cpp \
../src/Systems/Pneumatics.cpp \
../src/Systems/Sensing.cpp 

OBJS += \
./src/Systems/Beacon.o \
./src/Systems/Drive.o \
./src/Systems/MPC.o \
./src/Systems/Operator.o \
./src/Systems/Pneumatics.o \
./src/Systems/Sensing.o 

CPP_DEPS += \
./src/Systems/Beacon.d \
./src/Systems/Drive.d \
./src/Systems/MPC.d \
./src/Systems/Operator.d \
./src/Systems/Pneumatics.d \
./src/Systems/Sensing.d 


# Each subdirectory must supply rules for building sources it contributes
src/Systems/%.o: ../src/Systems/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"C:\Users\Lucas\Desktop\Bronc Botz Coding\FRC workspace\BroncBotz_Auton\FRC%202016%20CPP/src" -IC:\Users\Lucas/wpilib/cpp/current/sim/include -I/usr/include -I/usr/include/gazebo-3.2 -I/usr/include/sdformat-2.2 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


