################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Config/XML/pugixml.cpp 

OBJS += \
./src/Config/XML/pugixml.o 

CPP_DEPS += \
./src/Config/XML/pugixml.d 


# Each subdirectory must supply rules for building sources it contributes
src/Config/XML/%.o: ../src/Config/XML/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"C:\Users\Lucas\Desktop\Bronc Botz Coding\FRC workspace\BroncBotz_Auton\FRC%202016%20CPP/src" -IC:\Users\Lucas/wpilib/cpp/current/sim/include -I/usr/include -I/usr/include/gazebo-3.2 -I/usr/include/sdformat-2.2 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


