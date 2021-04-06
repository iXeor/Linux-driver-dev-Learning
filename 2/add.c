//Linux内核驱动开发代码2：模块之间的通信。教材第84、85页5.3.3
/*******************************************************/
#include <linux/init.h>   //Linux内核中定义了初始化数据相关的宏的头文件（module_init和moudle_exit的定义在这里）
#include <linux/module.h> //Linux内核中定义了模块相关处理的头文件，用于动态加载模块至内核（Dynamic loading of modules into the kernel.）
#include "add.h"          //调用本地定义的头文件add.h

long add_integer(int a, int b) //设计方法add_integer
{
    return a + b; //返回长整型的a+b
}
EXPORT_SYMBOL(add_integer);     //导出add_integer函数到内核符号表
MODULE_LICENSE("Dual BSD/GPL"); //指定许可权为BSD/GPL双许可

/**********************************
 * 注：与test.c需要分别编译成模块
 * 这个模块编译产生的Module.symvers
 * 被test.c编译模块时所需用。
**********************************/