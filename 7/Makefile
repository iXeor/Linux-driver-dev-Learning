ifeq ($(KERNELRELEASE),)
	KERNELDIR ?= /usr/src/linux-2.6.39.4
	PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order
else
	obj-m := globalfifo.o
endif
