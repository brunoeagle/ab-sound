.SUFFIXES:				# ignore builtin rules
.PHONY: all debug release clean

##### Definitions
DEVICE = STM32F746BG
MCU = STM32F746xx
PROJECTNAME = ab-sound

##### Directories
OBJ_DIR = build
EXE_DIR = build/exe
LST_DIR = build/lst

##### Linux Commands (will be overwritten if on Windows)
RMDIRS     := rm -rf
RMFILES    := rm -rf
ALLFILES   := *.*
SHELLNAMES := $(ComSpec)$(COMSPEC)

##### Try autodetecting the environment
ifeq ($(SHELLNAMES),)
  # Assume we are making on a Linux platform
  TOOLDIR := $(LINUXCS)
else
  ifneq ($(COMSPEC),)
    # Assume we are making on a mingw/msys/cygwin platform running on Windows
    # This is a convenient place to override TOOLDIR, DO NOT add trailing
    # whitespace chars, they do matter !
    TOOLDIR := $(PROGRAMFILES)/$(WINDOWSCS)
    ifeq ($(findstring cygdrive,$(shell set)),)
      # We were not on a cygwin platform
      NULLDEVICE := NUL
    endif
  else
    # Assume we are making on a Windows platform
    # This is a convenient place to override TOOLDIR, DO NOT add trailing
    # whitespace chars, they do matter !
    SHELL      := $(SHELLNAMES)
    TOOLDIR    := $(ProgramFiles)/$(WINDOWSCS)
    RMDIRS     := rd /s /q
    RMFILES    := del /s /q
    ALLFILES   := \*.*
    NULLDEVICE := NUL
    OBJ_DIR = build
    EXE_DIR = build\exe
    LST_DIR = build\lst
  endif
endif

##### Create directories and do a clean which is compatible with parallell make
ifeq (clean,$(findstring clean, $(MAKECMDGOALS)))
  ifneq ($(filter $(MAKECMDGOALS),all debug release),)
    $(shell $(RMFILES) $(OBJ_DIR))
    $(shell $(RMFILES) $(EXE_DIR))
    $(shell $(RMFILES) $(LST_DIR))
  endif
else
    $(shell mkdir $(OBJ_DIR))
    $(shell mkdir $(EXE_DIR))
    $(shell mkdir $(LST_DIR))
endif

CC      = arm-none-eabi-gcc
CP      = arm-none-eabi-g++
LD      = arm-none-eabi-ld
AR      = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
DUMP    = arm-none-eabi-objdump
SIZE    = arm-none-eabi-size

##### Flags
DEPFLAGS = -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"

override CFLAGS += -mcpu=cortex-m7 -falign-functions=16 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -fsingle-precision-constant -D"$(MCU)" -mthumb -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -Wall -Wextra -Wno-unused-parameter $(DEPFLAGS)

override ASMFLAGS += -mcpu=cortex-m7 -mthumb -falign-functions=16 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -fsingle-precision-constant -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -x assembler-with-cpp -Wall -Wextra $(DEPFLAGS)

override LDFLAGS += -mcpu=cortex-m7 -mthumb -falign-functions=16 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -fsingle-precision-constant -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -T./linker/STM32F746BGTx_FLASH.ld -Xlinker --gc-sections -Wl,-Map=$(LST_DIR)/$(PROJECTNAME).map -specs=nosys.specs -specs=nano.specs

##### Include Paths
INCLUDEPATHS += \
-I../STM32Cube_FW_F7_V1.12.0/Drivers/CMSIS/Device/ST/STM32F7xx/Include \
-I../STM32Cube_FW_F7_V1.12.0/Drivers/CMSIS/Include \
-I../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Inc \
-I../FreeRTOSv9.0.0/FreeRTOS/Source/include \
-I../FreeRTOSv9.0.0/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 \
-I../u8g2/csrc \
-I../u8g2/cppsrc \
-I./src \
-I./src/peripherals \
-I./src/stm32

####################################################################
# Files                                                            #
####################################################################

C_SRC +=  \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spi.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spdifrx.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c \
../STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai.c \
../FreeRTOSv9.0.0/FreeRTOS/Source/list.c \
../FreeRTOSv9.0.0/FreeRTOS/Source/queue.c \
../FreeRTOSv9.0.0/FreeRTOS/Source/tasks.c \
../FreeRTOSv9.0.0/FreeRTOS/Source/timers.c \
../FreeRTOSv9.0.0/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.c \
../FreeRTOSv9.0.0/FreeRTOS/Source/portable/MemMang/heap_1.c \
../u8g2/csrc/u8g2_d_setup.c \
../u8g2/csrc/u8x8_setup.c \
../u8g2/csrc/u8g2_d_memory.c \
../u8g2/csrc/u8g2_setup.c \
../u8g2/csrc/u8x8_cad.c \
../u8g2/csrc/u8x8_d_ssd1322.c \
../u8g2/csrc/u8g2_ll_hvline.c \
../u8g2/csrc/u8x8_display.c \
../u8g2/csrc/u8g2_font.c \
../u8g2/csrc/u8g2_hvline.c \
../u8g2/csrc/u8x8_gpio.c \
../u8g2/csrc/u8x8_byte.c \
../u8g2/csrc/u8g2_box.c \
../u8g2/csrc/u8g2_intersection.c \
../u8g2/csrc/u8g2_buffer.c \
../u8g2/csrc/u8x8_8x8.c \
../u8g2/csrc/u8x8_fonts.c \
../u8g2/csrc/u8g2_fonts.c \
../u8g2/csrc/u8g2_cleardisplay.c \
./src/stm32/system_stm32f7xx.c \
./src/stm32/stm32f7xx_it.c \
./src/stm32/spi1.c \
./src/stm32/sai1.c \
./src/stm32/usart2.c \
./src/stm32/clock.c \
./src/stm32/tim6_hal_tick.c \
./src/peripherals/digital_trimpots.c \
./src/peripherals/input_selector.c \
./src/peripherals/spdif.c \
./src/peripherals/dac.c \
./src/peripherals/esp8266.c \
./src/tasks/display.c \
./src/tasks/volume_control.c \
./src/tasks/digital_input.c \
./src/tasks/wifi.c \
./src/main.c

S_SRC += \
./startup/startup_stm32f746xx.S

####################################################################
# Rules                                                            #
####################################################################

C_FILES = $(notdir $(C_SRC) )
S_FILES = $(notdir $(S_SRC) )
#make list of source paths, sort also removes duplicates
C_PATHS = $(sort $(dir $(C_SRC) ) )
S_PATHS = $(sort $(dir $(S_SRC) ) )

C_OBJS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.o))
S_OBJS = $(if $(S_SRC), $(addprefix $(OBJ_DIR)/, $(S_FILES:.S=.o)))
C_DEPS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.d))
OBJS = $(C_OBJS) $(S_OBJS) $(s_OBJS)

vpath %.c $(C_PATHS)
vpath %.S $(S_PATHS)

# Default build is debug build
all:	debug

debug:	CFLAGS += -DDEBUG -O0
debug:	$(EXE_DIR)/$(PROJECTNAME).bin

release:  CFLAGS += -DNDEBUG -O0
release:  $(EXE_DIR)/$(PROJECTNAME).bin

# Create objects from C SRC files
$(OBJ_DIR)/%.o: %.c
	@echo "Building file: $<"
	$(CC) $(CFLAGS) $(INCLUDEPATHS) -c -o $@ $<

# Assemble .s/.S files
$(OBJ_DIR)/%.o: %.S
	@echo "Assembling $<"
	$(CC) $(ASMFLAGS) $(INCLUDEPATHS) -c -o $@ $<

# Link
$(EXE_DIR)/$(PROJECTNAME).out: $(OBJS)
	@echo "Linking target: $@"
	$(CP) $(LDFLAGS) $(OBJS) -o $(EXE_DIR)/$(PROJECTNAME).out

# Create binary file
$(EXE_DIR)/$(PROJECTNAME).bin: $(EXE_DIR)/$(PROJECTNAME).out
	@echo "Creating binary file"
	$(OBJCOPY) -O binary $(EXE_DIR)/$(PROJECTNAME).out $(PROJECTNAME).bin
	@echo "Used memories:"
	$(SIZE) $(EXE_DIR)/$(PROJECTNAME).out
# Uncomment next line to produce assembly listing of entire program
#	$(DUMP) -h -S -C $(EXE_DIR)/$(PROJECTNAME).out>$(LST_DIR)/$(PROJECTNAME)out.lst

clean:
ifeq ($(filter $(MAKECMDGOALS),all debug release),)
	$(RMDIRS) $(OBJ_DIR)
	$(RMFILES) $(PROJECTNAME).bin
endif

# include auto-generated dependency files (explicit rules)
ifneq (clean,$(findstring clean, $(MAKECMDGOALS)))
-include $(C_DEPS)
endif
