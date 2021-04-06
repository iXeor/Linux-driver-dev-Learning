//Linux内核驱动开发代码1：Hello World模块。教材第78页5.2.2
/*****************************************************/
#include <linux/init.h>	  //Linux内核中定义了初始化数据相关的宏的头文件（module_init和moudle_exit的定义在这里）
#include <linux/module.h> //Linux内核中定义了模块相关处理的头文件，用于动态加载模块至内核（Dynamic loading of modules into the kernel.）

static int hello_init(void) //hello_init函数
{
	printk(KERN_ALERT "Hello World\n"); //打印Hello World到内核信息里，可以通过dmesg命令查看。
	return 0;							//因为该函数返回值为整形，如果顺利完成，就返回0（否则会隐性地返回其他值）。
}
static void hello_exit(void) //hello_exit函数
{
	printk(KERN_ALERT "Goodbye\n"); //打印Goodbye到内核信息里，可以通过dmesg命令查看。
}
module_init(hello_init);		//指定模块加载函数为“hello_init”
module_exit(hello_exit);		//指定模块卸载函数为“hello_exit”
MODULE_LICENSE("Dual BSD/GPL"); //指定许可权为BSD/GPL双许可

/********************************************************************************
 * 注：从2.4.10版本内核开始，模块必须通过MODULE_LICENSE宏声明此模块的许可证，否则在加载
 * 此模块时，会收到内核被污染 “kernel tainted” 的警告。从linux/module.h文件中可以看到，
 * 被内核接受的有意义的许可证有 “GPL”，“GPL v2”，“GPL and additional rights”，
 * “Dual BSD/GPL”，“Dual MPL/GPL”，“Proprietary”。
 * ****************************************************************************/