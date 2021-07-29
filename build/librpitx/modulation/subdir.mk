################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../librpitx/modulation/amdmasync.cpp \
../librpitx/modulation/atv.cpp \
../librpitx/modulation/dsp.cpp \
../librpitx/modulation/fmdmasync.cpp \
../librpitx/modulation/fskburst.cpp \
../librpitx/modulation/iqdmasync.cpp \
../librpitx/modulation/ngfmdmasync.cpp \
../librpitx/modulation/ookburst.cpp \
../librpitx/modulation/phasedmasync.cpp \
../librpitx/modulation/serialdmasync.cpp 

OBJS += \
./librpitx/modulation/amdmasync.o \
./librpitx/modulation/atv.o \
./librpitx/modulation/dsp.o \
./librpitx/modulation/fmdmasync.o \
./librpitx/modulation/fskburst.o \
./librpitx/modulation/iqdmasync.o \
./librpitx/modulation/ngfmdmasync.o \
./librpitx/modulation/ookburst.o \
./librpitx/modulation/phasedmasync.o \
./librpitx/modulation/serialdmasync.o 

CPP_DEPS += \
./librpitx/modulation/amdmasync.d \
./librpitx/modulation/atv.d \
./librpitx/modulation/dsp.d \
./librpitx/modulation/fmdmasync.d \
./librpitx/modulation/fskburst.d \
./librpitx/modulation/iqdmasync.d \
./librpitx/modulation/ngfmdmasync.d \
./librpitx/modulation/ookburst.d \
./librpitx/modulation/phasedmasync.d \
./librpitx/modulation/serialdmasync.d 


# Each subdirectory must supply rules for building sources it contributes
librpitx/modulation/%.o: ../librpitx/modulation/%.cpp librpitx/modulation/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/librpitx" -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/librpitx/core/include" -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/librpitx/modulation/include" -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/hpsdr/include" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


