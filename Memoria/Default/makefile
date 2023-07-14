-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := Memoria
BUILD_ARTIFACT_EXTENSION :=
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Memoria 
	
	
# Tool invocations
Memoria: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "Memoria" $(OBJS) $(USER_OBJS) $(LIBS) -lcommons -lreadline -lpthread
	@echo 'Finished building target: $@'
	@echo ' '

run:
	cd home/utnso/tp-2023-1c-Los-operadores/Memoria/Default/config && FILE_PATH=$(FILE_PATH) ./Memoria

# Other Targets
clean:
	-$(RM) Memoria
	-@echo ' '

.PHONY: all clean dependents main-build

-include ../makefile.targets