ifeq ($(KERNELRELEASE),)
	KERNELDIR ?= /linux-2.6.34/linux-2.6.34
	PWD := $(shell pwd)
	KBUILD_EXTRA_SYMBOLS=$(obj)/../test/Module.symvers
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
else
	obj-m := test.o
endif
