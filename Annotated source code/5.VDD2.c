//Linux内核驱动开发代码4：虚拟字符设备驱动程序
/***********************************************************************/
#define VIRTUALDISK_SIZE 0xFFFF //宏定义，用于设定memory大小
#define VIRTUALDISK_NUM 200		//宏定义，设置默认分配的主设备号为200
#define MEM_CLEAR 0x1			//规定触发清空虚拟磁盘内容对应的宏
#define PORT1_SET 0x2			//规定挂载端口1的宏
#define PORT2_SET 0x3			//规定挂载端口2的宏

#include <linux/init.h>	  //Linux内核中定义了初始化数据相关的宏的头文件（module_init和moudle_exit的定义在这里）
#include <linux/module.h> //Linux内核中定义了模块相关处理的头文件，用于动态加载模块至内核（Dynamic loading of modules into the kernel.）
#include <linux/fs.h>	  //Linux内核中定义了一些重要的文件表结构之类的头文件 (This file has definitions for some important file table structures etc.)
#include <linux/slab.h>	  //Linux内存管理slab分配器/算法相关头文件
#include <linux/cdev.h>
/********************************************************************************************************
 * cdev是linux用来管理字符设备的结构体，
 * 其在内核中采用数组结构设计，这样系统中有多少个主设备号就约定了数组大小，
 * 此设备号采用链表管理，同一主设备号下可以有多个子设备。
 * 设备即文件，上层应用要访问设备，必须通过文件，cdev中包含file_operations等结构体，该结构体是驱动的文件操作集合。
 * *****************************************************************************************************/
#include <linux/sched.h> //Linux内核中负责任务调度相关函数的头文件
#include <linux/mm.h>	 //Linux内核中负责内存调度相关函数的头文件
#include <asm/system.h>	 //Linux内核中的通用系统定义头文件（Generic system definitions, based on MN10300 definitions.）
#include <asm/uaccess.h> //Linux内核中负责用户空余内存访问函数的头文件（User space memory access functions）
#include <asm/io.h>		 //用于通用I/O设备模拟的头文件

typedef struct VirtualDisk //构造VirtualDisk结构体
{
	struct cdev cdev;						//VirtualDisk结构体包含一个struct cdev结构体类型的cdev参数
	unsigned char memory[VIRTUALDISK_SIZE]; //还有一个用于存储的memory
	int port1;								//初始化port1
	long port2;								//初始化port2
	long count;								//初始化设备计数器
} vdDriver;									//使用typedef方法将其重新命名为vdDriver类型，方便之后使用

vdDriver *VirtualDisk_devp; //初始化一个vdDriver类型的指针

int Virtualdisk_open(struct inode *inode, struct file *fp) //方法Virtualdisk_open
{
	vdDriver *devp;	 //初始化一个vdDriver类型的指针，将文件结构体中的私有数据传递给该指针(避免C90混合警告，先声明变量，后赋值)
	fp->private_data = VirtualDisk_devp; //将VirtualDisk_devp的地址递交到文件结构体中的私有数据，完成初步挂载。
	devp = fp->private_data;
	devp->count++;						 //设备计数器+1
	return 0;							 //正常打开设备返回0
}
int Virtualdisk_release(struct inode *inode, struct file *fp) //方法Virtualdisk_release
{
	vdDriver *devp = fp->private_data; //初始化一个vdDriver类型的指针，将文件结构体中的私有数据传递给该指针
	devp->count--;					   //设备计数器-1，销毁挂载
	return 0;						   //正常释放设备返回0
}
static ssize_t Virtualdisk_read(struct file *fp, char __user *buff, size_t size, loff_t *ppos) //方法Virtualdisk_read
{
	unsigned long p = *ppos;		   //初始化文件偏移量p
	unsigned int count = size;		   //read内部计数器设置为单次读取的大小
	int result = 0;					   //初始化返回结果
	vdDriver *devp = fp->private_data; //初始化一个vdDriver类型的指针，将文件结构体中的私有数据传递给该指针
	if (p >= VIRTUALDISK_SIZE)		   //一旦偏移量超过虚拟磁盘的大小
	{
		return count ? -ENXIO : 0; //判断计数器是否大于0，如果大于零返回 -ENXIO （No such device or address，没有该设备或该地址），如果小于0就返回0，程序终止
	}
	if (count > VIRTUALDISK_SIZE - p) //如果计数器比剪切过偏移量之后的剩余空间大
	{
		count = VIRTUALDISK_SIZE - p; //则将剩余空间作为计数器最大值
	}
	if (copy_to_user(buff, (void *)(devp->memory + p), count)) //内核空间-->用户空间 如果拷贝成功返回0
	{
		result = -EFAULT; //拷贝不成功，结果赋值为 -EFAULT（Bad address，原因：空间不足完成拷贝，数据传入空指针造成丢失）
	}
	else //如果拷贝成功
	{
		*ppos += count;											  //偏移量增加计数器个位移
		result = count;											  //结果即计数器
		printk(KERN_INFO "Read %u byte(s) from %lu\n", count, p); //将从 @p 读取到 @count 个字节的信息打印到内核信息里。
	}
	return result; //返回result
}
static ssize_t Virtualdisk_write(struct file *fp, const char __user *buff, size_t size, loff_t *ppos) //方法Virtualdisk_write
{
	unsigned long p = *ppos;		   //初始化文件偏移量p
	unsigned int count = size;		   //read内部计数器设置为单次读取的大小
	int result = 0;					   //初始化返回结果
	vdDriver *devp = fp->private_data; //初始化一个vdDriver类型的指针，将文件结构体中的私有数据传递给该指针
	if (p >= VIRTUALDISK_SIZE)		   //一旦偏移量超过虚拟磁盘的大小
	{
		return count ? -ENXIO : 0; //判断计数器是否大于0，如果大于零返回 -ENXIO （No such device or address，没有该设备或该地址），如果小于0就返回0，程序终止
	}
	if (count > VIRTUALDISK_SIZE - p) //如果计数器比剪切过偏移量之后的剩余空间大
	{
		count = VIRTUALDISK_SIZE - p; //则将剩余空间作为计数器最大值
	}
	if (copy_from_user((void *)(devp->memory + p), buff, count)) //用户空间-->内核空间 如果拷贝成功返回0
	{
		result = -EFAULT; //拷贝不成功，结果赋值为 -EFAULT（Bad address，原因：空间不足完成拷贝，数据传入空指针造成丢失）
	}
	else //如果拷贝成功
	{
		*ppos += count;												 //偏移量增加计数器个位移
		result = count;												 //结果即计数器
		printk(KERN_INFO "Written %u byte(s) from %lu\n", count, p); //将从 @p 写入 @count 个字节的信息打印到内核信息里。
	}
	return result; //返回result
}
static loff_t Virtualdisk_llseek(struct file *fp, loff_t offset, int o) //方法Virtualdisk_llseek
{
	loff_t result = 0; //初始化返回结果
	switch (o)		   //分支判断指针位置
	{
	case SEEK_SET:		//如果在文件头部
		if (offset < 0) //一旦偏移量小于0
		{
			result = -EINVAL; //返回 -EINVAL（Invalid argument，无效参数）
			break;			  //跳出循环
		}
		if ((unsigned int)offset > VIRTUALDISK_SIZE) //如果偏移量超过磁盘大小
		{
			result = -EINVAL; //返回 -EINVAL（Invalid argument，无效参数）
			break;			  //跳出循环
		}
		fp->f_pos = (unsigned int)offset;			 //文件位置设置在偏移量为@offset的位置开始
		result = fp->f_pos;							 //结果设为文件结构体读取文件的指针位置
		break;										 //跳出循环
	case SEEK_CUR:									 //如果在文件当前位置
		if ((fp->f_pos + offset) > VIRTUALDISK_SIZE) //如果文件指针当前位置与偏移量之和超过虚拟磁盘大小
		{
			result = -EINVAL; //返回 -EINVAL（Invalid argument，无效参数）
			break;			  //跳出循环
		}
		if ((fp->f_pos + offset) < 0) //如果文件指针当前位置与偏移量之和小于0
		{
			result = -EINVAL; //返回 -EINVAL（Invalid argument，无效参数）
			break;			  //跳出循环
		}
		fp->f_pos += offset; //文件位置设置在偏移量为当前位置增加@offset之后的位置开始
		result = fp->f_pos;	 //结果设为文件结构体读取文件的指针位置
		break;				 //跳出循环
	default:				 //其他情况
		result = -EINVAL;	 //返回 -EINVAL（Invalid argument，无效参数）
		break;				 //跳出循环
	}
	return result; //返回结果值
}
//static int Virtualdisk_ioctl(struct inode* inode,struct file* fp, unsigned int cmd,unsigned long arg) //旧版本的ioctl指针
static long Virtualdisk_ioctl(struct file *fp,unsigned int cmd,unsigned long arg) //方法Virtualdisk_ioctl （在Kernel 2.6.36中已经完全删除了file_operations中的ioctl函数指针，取而代之的是unlocked_ioctl，这里使用unlocked_ioctl）
{
	vdDriver *devp = fp->private_data; //初始化一个vdDriver类型的指针，将文件结构体中的私有数据传递给该指针
	switch (cmd)					   //分支选择，根据传入的指令码
	{
	case MEM_CLEAR:								   //如果是执行清空内存的指令码
		memset(devp->memory, 0, VIRTUALDISK_SIZE); //将虚拟磁盘全部空间填0
		printk(KERN_INFO "Clear Succeed\n");	   //在内核消息中打印Clear Succeed
		break;									   //跳出循环
	case PORT1_SET:								   //如果是设置端口1的指令码
		devp->port1 = 0;						   //初始化devp的端口1
		break;									   //跳出循环
	case PORT2_SET:								   //如果是设置端口2的指令码
		devp->port2 = 0;						   //初始化devp的端口2
		break;									   //跳出循环
	default:									   //其他指令
		return -EINVAL;							   //返回-EINVAL（Invalid argument，无效参数）
		break;									   //跳出循环
	}
	return 0; //成功执行返回0
}

static const struct file_operations vdd_fops = {
	//使用file_operations初始化一个vdd_fops，为初始化设备提供接口
	.owner = THIS_MODULE,			//使传入的设备属于该模块
	.llseek = Virtualdisk_llseek,	//设置指针方法为Virtualdisk_llseek
	.read = Virtualdisk_read,		//设置读操作方法为Virtualdisk_read
	.write = Virtualdisk_write,		//设置写操作方法为Virtualdisk_write
	//.ioctl = Virtualdisk_ioctl,		//设置输入输出控制方法为Virtualdisk_ioctl
	.unlocked_ioctl = Virtualdisk_ioctl,//设置输入输出控制方法为Virtualdisk_ioctl（Kernel 2.6.36以上版本使用该语句）
	.open = Virtualdisk_open,		//设置挂载方法为Virtualdisk_open
	.release = Virtualdisk_release, //设置释放方法为Virtualdisk_release
};

static int majorno = VIRTUALDISK_NUM; //初始化静态整形变量majorno存储主设备号（暂定为VIRTUALDISK_NUM的值【200】）

static void setup_cdev(vdDriver *devp, dev_t devno) //setup_cdev函数，用于 初始化 和 注册 cdev结构体
{
	int result;										//初始化整形result，用于后续确定添加设备是否成功
	cdev_init(&devp->cdev, &vdd_fops);				//初始化cdev
	devp->cdev.owner = THIS_MODULE;					//使传入的设备属于该模块
	devp->cdev.ops = &vdd_fops;						//挂载cdev结构体的操作为vdd_fops
	result = cdev_add(&devp->cdev, devno, 1);		//将cdev注册到系统中，也就是将字符设备加入到内核中，并将结果赋值给result
	if (result < 0)									//如果result<0（注册失败）
		printk(KERN_NOTICE "ERROR IN ADD cdev!\n"); //打印ERROR IN ADD cdev!到内核信息里（警告）。
}
static int VirtualDisk_init(void) //VirtualDisk_init函数
{
	int result;						 //初始化整形result，用于后续确定驱动加载是否成功
	dev_t devno = MKDEV(majorno, 0); //初始化设备号
	if (majorno)					 //判断事先手动设置的主设备号（majorno）是否可用
	{
		result = register_chrdev_region(devno, 1, "VirtualDisk"); //如果该主设备号可用，使用register_chrdev_region申请指定主设备号的设备，数量为1，名字是"VirtualDisk”，并把设备号传给result
	}
	else
	{
		result = alloc_chrdev_region(&devno, 0, 1, "VirtualDisk"); //否则使用alloc_chrdev_region动态申请从设备号为0，数量为1，名字是“VirtualDisk”的设备，并把设备号传给result
		majorno = MAJOR(devno);									   //将原先的majorno换成当前的majorno
	}
	if (result < 0)											  //如果申请不到设备号（result<0）
		return result;										  //返回result的值，退出。
	VirtualDisk_devp = kmalloc(sizeof(vdDriver), GFP_KERNEL); //申请到设备号之后先动态申请设备VirtualDisk_dev结构体的内存
	if (!VirtualDisk_devp)									  //如果申请内存失败
	{
		result = -ENOMEM; //result赋值-12（ENOMEM ： errno 12，表示内存分配失败）
		goto fail_malloc; //goto语句跳转到fail_malloc方法并执行该方法
	}
	memset(VirtualDisk_devp, 0, sizeof(vdDriver));		  //将VirtualDisk_dev的内存清零
	setup_cdev(VirtualDisk_devp, devno);				  //初始化和注册与VirtualDisk_dev相关的cdev结构体
	printk("Device Regist Successed! devno=%d\n", devno); //打印Device Regist Successed! devno=设备号到内核信息里（设备注册成功），可以通过dmesg命令查看。
fail_malloc:											  //fail_malloc方法段
	unregister_chrdev_region(devno, 1);					  //使用unregister_chrdev_region释放当前申请的设备号
	return result;										  //返回result（内存分配失败）
}
static void VirtualDisk_exit(void) //VirtualDisk_exit函数
{
	cdev_del(&VirtualDisk_devp->cdev);				//注销与VirtualDisk_dev相关的cdev结构体
	kfree(VirtualDisk_devp);						//释放VirtualDisk_dev设备结构体占用的内存
	unregister_chrdev_region(MKDEV(majorno, 0), 1); //使用unregister_chrdev_region释放当前设备的设备号
	printk("Device Unregist Succeed!\n");			//打印Device Unregist Succeed!内核信息里，可以通过dmesg命令查看。（释放成功）
}
module_init(VirtualDisk_init);	//指定模块加载函数为“VirtualDisk_init”
module_exit(VirtualDisk_exit);	//指定模块加载函数为“VirtualDisk_exit”
MODULE_LICENSE("Dual BSD/GPL"); //指定许可权为BSD/GPL双许可