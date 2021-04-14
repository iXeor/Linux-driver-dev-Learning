ifeq ($(KERNELRELEASE),) #判断KERNELRELEASE变量是否为空。只有执行make命令的当前目录是内核源代码目录时，该变量非空。
	KERNELDIR ?= /usr/src/linux-2.6.39.4
	# 设置内核路径变量为使用的内核所在路径，比如我的内核路径为“/linux-2.6.34/linux-2.6.34”就写成 \
	KERNELDIR ?= /linux-2.6.34/linux-2.6.34 ,\
	“？=”表示如果该变量没有被赋值，则赋予等号后的值\
	即：KERNELRELEASE变量为空时，设置内核路径变量为 /linux-2.6.34/linux-2.6.34\
	避免不在内核源代码目录下编译不通过的问题
	PWD := $(shell pwd)
	# PWD设置为pwd命令获得的当前目录，“:=”是直接赋值，没有条件判断
modules: #make的子功能选项modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	#该语句会执行内核模块的编译 等价为“make -C /linux-2.6.34/linux-2.6.34 M=当前所在目录 modules”
modules_install: #make的子功能选项modules_install
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
	#该语句会执行内核模块的安装（安装至模块对应路径） 等价为“make -C /linux-2.6.34/linux-2.6.34 M=当前所在目录 modules_install”
clean: #make的子功能选项clean(清除)
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
	#强制删除所有当前目录下的.o、core、.depend、.cmd、.ko。mod.c、.tmp_versions后缀的文件
else 
	#如果KERNELRELEASE非空，直接将hello.o编译成hello.ko模块
	obj-m := hello.o
endif
	#判断结束
