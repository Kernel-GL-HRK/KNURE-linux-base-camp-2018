
SRC_DIR_NAME := src


ifneq ($(KERNELRELEASE),)

TARGET = tmm
MODULE_DIR := $(M)
SRC := $(patsubst $(M)/%,%,$(wildcard $(M)/**/*.c))
ccflags-y := -std=gnu99 -Wall -Werror -I$(MODULE_DIR)/$(SRC_DIR)

$(TARGET)-objs += $(SRC:.c=.o)

obj-m := $(TARGET).o

else

OUT_DIR := build

.PHONY: all clean
all:
	$(MAKE) -C $(BUILD_KERNEL_DIR) M=$(CURDIR) modules
	@mkdir -p $(OUT_DIR)
	@mv *.ko $(OUT_DIR)/

clean:
	$(MAKE) -C $(BUILD_KERNEL_DIR) M=$(CURDIR) clean
	@rm -f $(OUT_DIR)/*

endif
