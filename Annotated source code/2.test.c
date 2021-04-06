//Linux内核驱动开发代码2：模块之间的通信。教材第84、85页5.3.3
/*******************************************************/
#include <linux/init.h>		   //Linux内核中定义了初始化数据相关的宏的头文件（module_init和moudle_exit的定义在这里）
#include <linux/module.h>	   //Linux内核中定义了模块相关处理的头文件，用于动态加载模块至内核（Dynamic loading of modules into the kernel.）
#include "add.h"			   //调用本地定义的头文件add.h
static int a = 15;			   //定义静态（static）整形变量a，赋值等于15
static int b = 17;			   //定义静态（static）整形变量b，赋值等于17
module_param(a, int, S_IRUGO); //指定模块加载参数a，类型为整形（int），所有者和所有者所在组及其他用户均可修改该参数。
module_param(b, int, S_IRUGO); //指定模块加载参数b，类型为整形（int），所有者和所有者所在组及其他用户均可修改该参数。
/**************************************************************************
 * 对于module_param三个参数的解释：
 * module_param(name, type, perm);
 * 第一个参数是参数的名称，是自己定义的a、b、c等等
 * 第二个参数是变量的类型，比如int,long，char,float等
 * 第三个参数是权限，类似于文件的权限。这里需要指出哪些用户和组可以修改这个参数。
*************************************************************************/
static int hello_init(void) //hello_init函数
{
	{
		long res = 0;								 //定义长整型变量res，初始化为0，用于接收运算结果
		res = add_integer(a, b);					 //给res赋值，调用add.h内申明的方法“add_integer”（该方法的实现位于add.c）
		printk(KERN_ALERT "Hello World,%ld\n", res); //打印Hello World，运算结果res到内核信息里，可以通过dmesg命令查看。
	}
	return 0; //因为该函数返回值为整形，如果顺利完成，就返回0（否则会隐性地返回其他值）。
}
static void hello_exit(void) //hello_exit函数
{
	printk(KERN_ALERT "Goodbye\n"); //打印Goodbye到内核信息里，可以通过dmesg命令查看。
}
module_init(hello_init);		//指定模块加载函数为“hello_init”
module_exit(hello_exit);		//指定模块卸载函数为“hello_exit”
MODULE_LICENSE("Dual BSD/GPL"); //指定许可权为BSD/GPL双许可

/*******************************
 * 注：与add.c需要分别编译成模块
 * 此模块编译需用add.c模块编译后
 * 产生的Module.symvers作为符号表
 * 以实现模块之间的通信
*******************************/