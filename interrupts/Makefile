obj-m+=led.o

KERNELDIR := $(BUILD_KERNEL)
ARCH := arm
CROSS_COMPILE := $(CROSS_COMPILE)
GCC := $(CROSS_COMPILE)gcc

.PHONY: all clean

all:
	make -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(CURDIR) modules
	$(GCC) user_led.c -o user_led
clean:
	make -C $(KERNELDIR) M=$(CURDIR) clean
	rm user_led
