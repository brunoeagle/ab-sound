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
DEPFLAGS = -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"

override CFLAGS += -mcpu=cortex-m7 -D"$(MCU)" -mthumb -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -Wall -Wextra -Wno-unused-parameter $(DEPFLAGS)

override ASMFLAGS += -mcpu=cortex-m7 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -x assembler-with-cpp -Wall -Wextra $(DEPFLAGS)

override LDFLAGS += -mcpu=cortex-m7 -mthumb -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -T./linker/STM32F746BGTx_FLASH.ld -Xlinker --gc-sections -Wl,-Map=$(LST_DIR)/$(PROJECTNAME).map -specs=nosys.specs -specs=nano.specs

##### Include Paths
INCLUDEPATHS += \
-I../STM32Cube_FW_F7_V1.5.0/Drivers/CMSIS/Device/ST/STM32F7xx/Include \
-I../STM32Cube_FW_F7_V1.5.0/Drivers/CMSIS/Include \
-I../STM32Cube_FW_F7_V1.5.0/Drivers/STM32F7xx_HAL_Driver/Inc \
-I./src \
-I./src/peripherals \
-I./src/stm32

####################################################################
# Files                                                            #
####################################################################

C_SRC +=  \
../STM32Cube_FW_F7_V1.5.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c \
../STM32Cube_FW_F7_V1.5.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c \
../STM32Cube_FW_F7_V1.5.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c \
./src/stm32/system_stm32f7xx.c \
./src/stm32/stm32f7xx_it.c \
./src/peripherals/lcd.c \
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
