#$(warning KERNELRELEASE=$(KERNELRELEASE))

#EXTRA_CFLAGS = -g -O0

ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
ROOT_DIR ?= $(shell pwd)

utils: modules
	-mkdir -p ${ROOT_DIR}/build
	cmake -S utils -B ${ROOT_DIR}/build $(MAKECMDGOALS)
	make -C ${ROOT_DIR}/build

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install: modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

.PHONY: utils modules modules_install clean

else

obj-m := smc_helper.o

endif
