# SPDX-License-Identifier: GPL-2.0-only
SRC_DIR = src
INC_DIR = $(PWD)/inc

C_SOURCES = \
	$(SRC_DIR)/bme280_attr_hmdt.c \
	$(SRC_DIR)/bmx280_attr.c \
	$(SRC_DIR)/bmx280_attr_pres.c \
	$(SRC_DIR)/bmx280_attr_temp.c \
	$(SRC_DIR)/bmx280_common.c \
	$(SRC_DIR)/bmx280_dev.c \
	$(SRC_DIR)/bmx280_if.c

C_INCLUDES = \
	-I$(INC_DIR)

OBJS = $(C_SOURCES:.c=.o)

obj-m += bmx280.o
bmx280-objs := $(OBJS)
EXTRA_CFLAGS := $(C_INCLUDES)
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

KVERSION ?= $(shell uname -r)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
