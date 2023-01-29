######################################
# target
######################################
TARGET = ab-sound

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og

#######################################
# paths
#######################################
BUILD_DIR = 	    build
SDK_ROOT :=       ../STM32Cube_FW_F7_V1.17.0
FREERTOS_DIR :=   ../FreeRTOSv10.3.1
U8G2_DIR :=       ../u8g2
PROJ_DIR :=       .

#######################################
# commands
#######################################
RM_DIR := 	  rm -rf
ifeq ($(OS),Windows_NT)
	RM_DIR := 	  rd /s /q
endif

######################################
# source
######################################
# C sources
C_SOURCES =  \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spi.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spdifrx.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c \
  $(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai.c \
  $(FREERTOS_DIR)/list.c \
  $(FREERTOS_DIR)/queue.c \
  $(FREERTOS_DIR)/tasks.c \
  $(FREERTOS_DIR)/timers.c \
  $(FREERTOS_DIR)/portable/GCC/ARM_CM7/r0p1/port.c \
  $(FREERTOS_DIR)/portable/MemMang/heap_1.c \
  $(U8G2_DIR)/csrc/u8g2_d_setup.c \
  $(U8G2_DIR)/csrc/u8x8_setup.c \
  $(U8G2_DIR)/csrc/u8g2_d_memory.c \
  $(U8G2_DIR)/csrc/u8g2_setup.c \
  $(U8G2_DIR)/csrc/u8x8_cad.c \
  $(U8G2_DIR)/csrc/u8x8_d_ssd1322.c \
  $(U8G2_DIR)/csrc/u8g2_ll_hvline.c \
  $(U8G2_DIR)/csrc/u8x8_display.c \
  $(U8G2_DIR)/csrc/u8g2_font.c \
  $(U8G2_DIR)/csrc/u8g2_hvline.c \
  $(U8G2_DIR)/csrc/u8g2_polygon.c \
  $(U8G2_DIR)/csrc/u8x8_gpio.c \
  $(U8G2_DIR)/csrc/u8x8_byte.c \
  $(U8G2_DIR)/csrc/u8g2_box.c \
  $(U8G2_DIR)/csrc/u8g2_intersection.c \
  $(U8G2_DIR)/csrc/u8g2_buffer.c \
  $(U8G2_DIR)/csrc/u8x8_8x8.c \
  $(U8G2_DIR)/csrc/u8x8_fonts.c \
  $(U8G2_DIR)/csrc/u8g2_fonts.c \
  $(U8G2_DIR)/csrc/u8g2_cleardisplay.c \
  $(PROJ_DIR)/src/stm32/system_stm32f7xx.c \
  $(PROJ_DIR)/src/stm32/stm32f7xx_it.c \
  $(PROJ_DIR)/src/stm32/spi1.c \
  $(PROJ_DIR)/src/stm32/sai1.c \
  $(PROJ_DIR)/src/stm32/usart2.c \
  $(PROJ_DIR)/src/stm32/clock.c \
  $(PROJ_DIR)/src/stm32/tim6_hal_tick.c \
  $(PROJ_DIR)/src/stm32/exti.c \
  $(PROJ_DIR)/src/peripherals/digital_trimpots.c \
  $(PROJ_DIR)/src/peripherals/input_selector.c \
  $(PROJ_DIR)/src/peripherals/spdif.c \
  $(PROJ_DIR)/src/peripherals/dac.c \
  $(PROJ_DIR)/src/peripherals/esp8266.c \
  $(PROJ_DIR)/src/tasks/display.c \
  $(PROJ_DIR)/src/tasks/volume_control.c \
  $(PROJ_DIR)/src/tasks/digital_input.c \
  $(PROJ_DIR)/src/tasks/wifi.c \
  $(PROJ_DIR)/src/main.c

# ASM sources
ASM_SOURCES =  \
  $(PROJ_DIR)/startup/startup_stm32f746xx.S


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-sp-d16 -fsingle-precision-constant

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F746xx

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-I$(SDK_ROOT)/Drivers/CMSIS/Device/ST/STM32F7xx/Include \
-I$(SDK_ROOT)/Drivers/CMSIS/Include \
-I$(SDK_ROOT)/Drivers/STM32F7xx_HAL_Driver/Inc \
-I$(FREERTOS_DIR)/include \
-I$(FREERTOS_DIR)/portable/GCC/ARM_CM7/r0p1 \
-I$(U8G2_DIR)/csrc \
-I$(U8G2_DIR)/cppsrc \
-I$(PROJ_DIR)/src \
-I$(PROJ_DIR)/src/peripherals \
-I$(PROJ_DIR)/src/stm32

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -Wfatal-errors -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wfatal-errors -fdata-sections -ffunction-sections
CFLAGS += -Wall

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = linker/STM32F746BGTx_FLASH.ld

# libraries
LIBS = -lc -lm -specs=nano.specs
LIBDIR = 
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "Building: $<"
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "Building assembly: $<"
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "Linking $<"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@
	
$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
clean:
	$(RM_DIR) $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
