################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../librpitx/core/dma.cpp \
../librpitx/core/gpio.cpp \
../librpitx/core/mailbox.cpp \
../librpitx/core/raspberry_pi_revision.cpp \
../librpitx/core/rpi.cpp \
../librpitx/core/util.cpp 

OBJS += \
./librpitx/core/dma.o \
./librpitx/core/gpio.o \
./librpitx/core/mailbox.o \
./librpitx/core/raspberry_pi_revision.o \
./librpitx/core/rpi.o \
./librpitx/core/util.o 

CPP_DEPS += \
./librpitx/core/dma.d \
./librpitx/core/gpio.d \
./librpitx/core/mailbox.d \
./librpitx/core/raspberry_pi_revision.d \
./librpitx/core/rpi.d \
./librpitx/core/util.d 


# Each subdirectory must supply rules for building sources it contributes
librpitx/core/%.o: ../librpitx/core/%.cpp librpitx/core/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"../librpitx" -I"../librpitx/core/include" -I"../librpitx/modulation/include" -I"../hpsdr/include" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


