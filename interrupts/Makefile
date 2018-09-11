obj-m+=led.o

KERNELDIR := $(BUILD_KERNEL)
ARCH := arm
CROSS_COMPILE := $(CROSS_COMPILE)

.PHONY: all clean

all:
	make -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(CURDIR) modules
clean:
	make -C $(KERNELDIR) M=$(CURDIR) clean
