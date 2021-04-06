//Linux内核驱动开发代码3：字符设备驱动程序框架。教材第96页6.1
/*******************************************************/
#include <linux/init.h>	  //Linux内核中定义了初始化数据相关的宏的头文件（module_init和moudle_exit的定义在这里）
#include <linux/module.h> //Linux内核中定义了模块相关处理的头文件，用于动态加载模块至内核（Dynamic loading of modules into the kernel.）
#include <linux/fs.h>	  //Linux内核中定义了一些重要的文件表结构之类的头文件 (This file has definitions for some important file table structures etc.)
static int majorno = 200; //定义静态整形变量majorno(主设备号)，并赋值为200.

static int VirtualDisk_init(void) //VirtualDisk_init函数
{
	int result;						 //定义整型变量result，用于获取设备号。
	dev_t devno = MKDEV(majorno, 0); //dev_t的申明：typedef unsigned int _dev_t; （位于types.h，实质为非负整形） 初始化dev_t类型的devno（设别号），获取通过宏MKDEV获取指定的设备号
	if (majorno)					 //判断事先手动设置的主设备号（majorno）是否可用
	{
		result = register_chrdev_region(devno, 1, "VirtualDisk"); //如果该主设备号可用，使用register_chrdev_region申请指定主设备号的设备，数量为1，名字是"VirtualDisk”，并把设备号传给result
	}
	else
	{
		result = alloc_chrdev_region(&devno, 0, 1, "VirtualDisk"); //否则使用alloc_chrdev_region动态申请从设备号为0，数量为1，名字是“VirtualDisk”的设备，并把设备号传给result
		majorno = MAJOR(devno);									   //将原先的majorno换成当前的majorno
	}
	printk("Alloc Successed! devno=%d\n", devno); //打印Alloc Successed! devno=设备号到内核信息里（申请成功），可以通过dmesg命令查看。
	return result;								  //因为该函数返回值为整形，如果顺利完成，就返回result（否则会隐性地返回其他值）。
}
static void VirtualDisk_exit(void) //VirtualDisk_exit函数
{
	unregister_chrdev_region(MKDEV(majorno, 0), 1); //使用unregister_chrdev_region释放当前申请的设备号
	printk("Unregister Succeed!\n");				//打印Unregister Succeed!内核信息里，可以通过dmesg命令查看。（释放成功）
}
module_init(VirtualDisk_init);	//指定模块加载函数为“VirtualDisk_init”
module_exit(VirtualDisk_exit);	//指定模块加载函数为“VirtualDisk_exit”
MODULE_LICENSE("Dual BSD/GPL"); //指定许可权为BSD/GPL双许可

/*****************************************************************************************
 * register_chrdev_region(dev_t from, unsigned count, const char *name) -注册一个设备号段
 * @from:需要的设备号范围中的第一个;必须包含主设备号。
 * @count:设定需要的设备数量。
 * @name:设备或驱动程序的名称。
 * ***************************************************************************************
 * alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,const char *name) 
 * -注册一个字符型设备号段
 * @dev:第一个分配的编号的输出参数（申请到的东西会传到这个地址）
 * @baseminor:申请的从设备范围中的第一个（通常很可能设成0）
 * @count:从设备号的数量（设定需要的设备数量。）
 * @name:关联设备或驱动程序的名称
 * 分配一个字符型设备号段。其主设备号将被动态选择，并在@dev中返回(连同第一个从设备号)。
 * 如果有错误将返回零或负数值。
 * ***************************************************************************************
 * unregister_chrdev_region(dev_t from, unsigned count)——释放一个设备号段
 * @from:要释放的设备号段中的第一个设备号
 * @count:要释放的设备数量
 * 这个函数将释放一个从@from开始，数量为@count设备号段。通常唤起的应该是最先被分配这些号码的设备
 * **************************************************************************************/