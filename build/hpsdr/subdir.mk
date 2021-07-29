################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hpsdr/hpsdr_debug.c \
../hpsdr/hpsdr_functions.c \
../hpsdr/hpsdr_main.c \
../hpsdr/hpsdr_newprotocol.c \
../hpsdr/hpsdr_oldprotocol.c 

CPP_SRCS += \
../hpsdr/librpitx_c.cpp 

OBJS += \
./hpsdr/hpsdr_debug.o \
./hpsdr/hpsdr_functions.o \
./hpsdr/hpsdr_main.o \
./hpsdr/hpsdr_newprotocol.o \
./hpsdr/hpsdr_oldprotocol.o \
./hpsdr/librpitx_c.o 

C_DEPS += \
./hpsdr/hpsdr_debug.d \
./hpsdr/hpsdr_functions.d \
./hpsdr/hpsdr_main.d \
./hpsdr/hpsdr_newprotocol.d \
./hpsdr/hpsdr_oldprotocol.d 

CPP_DEPS += \
./hpsdr/librpitx_c.d 


# Each subdirectory must supply rules for building sources it contributes
hpsdr/%.o: ../hpsdr/%.c hpsdr/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/librpitx" -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/librpitx/core/include" -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/librpitx/modulation/include" -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/hpsdr/include" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

hpsdr/%.o: ../hpsdr/%.cpp hpsdr/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/librpitx" -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/librpitx/core/include" -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/librpitx/modulation/include" -I"/home/opt/Desarrollos/workspace-rpitx/hpsdr-rpitx/hpsdr/include" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


