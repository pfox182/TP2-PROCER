################################################################################
# Automatically-generated file. Do not edit!
################################################################################

#-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
################################################################################
# Automatically-generated file. Do not edit!
################################################################################

O_SRCS := 
C_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
OBJS := 
C_DEPS := 
EXECUTABLES := 

# Every subdirectory with source files must be described here
SUBDIRS := \
. \
PP \
Log \
FuncionesPropias \
Estructuras \

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../PP/IOT.c \
../PP/LTS.c \
../PP/LTS_demorado.c \
../PP/LTS_suspendido.c \
../PP/PROCER.c \
../PP/PROCER_funciones.c \
../PP/STS.c 

OBJS += \
./PP/IOT.o \
./PP/LTS.o \
./PP/LTS_demorado.o \
./PP/LTS_suspendido.o \
./PP/PROCER.o \
./PP/PROCER_funciones.o \
./PP/STS.o 

C_DEPS += \
./PP/IOT.d \
./PP/LTS.d \
./PP/LTS_demorado.d \
./PP/LTS_suspendido.d \
./PP/PROCER.d \
./PP/PROCER_funciones.d \
./PP/STS.d 


# Each subdirectory must supply rules for building sources it contributes
PP/%.o: ../PP/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Log/manejo_log.c 

OBJS += \
./Log/manejo_log.o 

C_DEPS += \
./Log/manejo_log.d 


# Each subdirectory must supply rules for building sources it contributes
Log/%.o: ../Log/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FuncionesPropias/manejo_archivos.c 

OBJS += \
./FuncionesPropias/manejo_archivos.o 

C_DEPS += \
./FuncionesPropias/manejo_archivos.d 


# Each subdirectory must supply rules for building sources it contributes
FuncionesPropias/%.o: ../FuncionesPropias/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Estructuras/colaConeccionesDemoradas.c \
../Estructuras/manejo_listas.c \
../Estructuras/manejo_listas_funciones.c \
../Estructuras/manejo_mensajes.c \
../Estructuras/manejo_pila_ejecucion.c \
../Estructuras/manejo_semaforos.c \
../Estructuras/proceso.c 

OBJS += \
./Estructuras/colaConeccionesDemoradas.o \
./Estructuras/manejo_listas.o \
./Estructuras/manejo_listas_funciones.o \
./Estructuras/manejo_mensajes.o \
./Estructuras/manejo_pila_ejecucion.o \
./Estructuras/manejo_semaforos.o \
./Estructuras/proceso.o 

C_DEPS += \
./Estructuras/colaConeccionesDemoradas.d \
./Estructuras/manejo_listas.d \
./Estructuras/manejo_listas_funciones.d \
./Estructuras/manejo_mensajes.d \
./Estructuras/manejo_pila_ejecucion.d \
./Estructuras/manejo_semaforos.d \
./Estructuras/proceso.d 


# Each subdirectory must supply rules for building sources it contributes
Estructuras/%.o: ../Estructuras/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../pp.c 

OBJS += \
./pp.o 

C_DEPS += \
./pp.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

USER_OBJS :=

LIBS := -lpthread


#++++++++++++++++++++++++++++++++++++++++++++++++++++

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

#-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: TP2-PROCER

# Tool invocations
TP2-PROCER: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "TP2-PROCER" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) TP2-PROCER
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

#-include ../makefile.targets
