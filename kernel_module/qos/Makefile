#$(warning KERNELRELEASE=$(KERNELRELEASE))

#EXTRA_CFLAGS = -g -O0

ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install: modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

.PHONY: utils modules modules_install clean

else

obj-$(CONFIG_CPUFREQ_QOS) += cpufreq-qos.o
obj-$(CONFIG_DEVFREQ_QOS) += devfreq-qos.o
obj-$(CONFIG_CPUHP_QOS)   += cpuhp-qos.o

endif
