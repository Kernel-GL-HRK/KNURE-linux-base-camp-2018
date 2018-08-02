
ifneq ($(KERNELRELEASE),)

MMS_TARGET := mms
MMT_TARGET := mmt

SRC_DIR := src

ccflags-y := -std=gnu99 -Wall -Werror

obj-m := $(MMS_TARGET).o
obj-m += $(MMT_TARGET).o

$(MMS_TARGET)-y := $(SRC_DIR)/$(MMS_TARGET).o $(SRC_DIR)/allocator.o
$(MMT_TARGET)-y := $(SRC_DIR)/$(MMT_TARGET).o $(SRC_DIR)/allocator.o

else

.PHONY: all clean
all:
	$(MAKE) -C $(BUILD_KERNEL_DIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(BUILD_KERNEL_DIR) M=$(CURDIR) clean

endif
