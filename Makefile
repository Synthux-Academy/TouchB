# Config Options
# DEBUG=1
ifeq ($(DEBUG), 1)
C_DEFS += -DINFS_LOG=1
C_DEFS += -DINFS_LOG_TARGET=daisy::LOGGER_INTERNAL
endif

# Project Name
TARGET = TouchB

USE_DAISYSP_LGPL=1

# Library Locations
LIBDAISY_DIR = lib/libDaisy
DAISYSP_DIR = lib/DaisySP
CMSIS_DSP_SRC_DIR = ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source

CPP_STANDARD = -std=gnu++17

# Daisy Bootloader - SRAM Linkage
# APP_TYPE = BOOT_SRAM
# LDSCRIPT = alt_sram.lds
# BOOT_BIN = bootloader-v2.bin

C_INCLUDES = -Isrc/ -Ilib/ -Isrc/common/
C_USR_FLAGS = -ffast-math -funroll-loops

# Sources
CPP_SOURCES = \
	app.cpp \
	main.cpp \
	$(wildcard src/**/*.cpp)

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

libs:
	cd $(LIBDAISY_DIR) && $(MAKE)
	cd $(DAISYSP_DIR) && $(MAKE)

clean-libs:
	cd $(LIBDAISY_DIR) && $(MAKE) clean
	cd $(DAISYSP_DIR) && $(MAKE) clean
