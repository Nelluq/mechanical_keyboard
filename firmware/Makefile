PROJECT = keyboard
BUILD_DIR = build

INCLUDE_DIR = inc
SOURCE_DIR = src
CFILES = $(notdir $(wildcard $(SOURCE_DIR)/*.c))

LDSCRIPT = stm32f070c6-cm3.ld
OPENCM3_LIB = opencm3_stm32f0
OPENCM3_DEFS = -DSTM32F0
ARCH_FLAGS = -mthumb -mcpu=cortex-m0 -msoft-float

INCLUDES += $(patsubst %,-I%, . $(INCLUDE_DIR))
OPENCM3_DIR=libopencm3

VPATH += $(SOURCE_DIR)

include rules.mk