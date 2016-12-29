################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/gklib4/app.cpp \
../src/gklib4/output.cpp 

OBJS += \
./src/gklib4/app.o \
./src/gklib4/output.o 

CPP_DEPS += \
./src/gklib4/app.d \
./src/gklib4/output.d 


# Each subdirectory must supply rules for building sources it contributes
src/gklib4/%.o: ../src/gklib4/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


