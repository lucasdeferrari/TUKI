# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/consola.c \
../src/utils.c 

C_DEPS += \
./src/consola.d \
./src/utils.d 

OBJS += \
./src/consola.o \
./src/utils.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/consola.d ./src/consola.o ./src/utils.d ./src/utils.o

.PHONY: clean-src
