//Linux内核驱动开发代码4：字符设备驱动程序框架。
/***********************************************************************/
#define VIRTUALDISK_SIZE 100 //宏定义，用于设定memory大小
#define VIRTUALDISK_NUM 200	 //宏定义，设置虚拟磁盘最大100Byte

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

typedef struct VirtualDisk //构造VirtualDisk结构体
{
	struct cdev cdev;						//VirtualDisk结构体包含一个struct cdev结构体类型的cdev参数
	unsigned char memory[VIRTUALDISK_SIZE]; //还有一个用于存储的memory
} vdDriver;									//使用typedef方法将其重新命名为vdDriver类型，方便之后使用

vdDriver *VirtualDisk_devp;						   //初始化一个vdDriver类型的指针
static const struct file_operations vdd_fops = {}; //使用file_operations初始化一个vdd_fops，为初始化设备提供接口

static int majorno = VIRTUALDISK_NUM; //初始化静态整形变量majorno存储主设备号（暂定为VIRTUALDISK_NUM的值【200】）

static void setup_cdev(vdDriver *devp, dev_t devno) //setup_cdev函数，用于 初始化 和 注册 cdev结构体
{
	int result;										//初始化整形result，用于后续确定添加设备是否成功
	cdev_init(&devp->cdev, &vdd_fops);				//初始化cdev
	devp->cdev.owner = THIS_MODULE;					//使传入的设备属于该模块
	result = cdev_add(&devp->cdev, devno, 1);		//将cdev注册到系统中，也就是将字符设备加入到内核中，并将结果赋值给result
	if (result < 0)									//如果result<0（注册失败）
		printk(KERN_NOTICE "ERROR IN ADD cdev!\n"); //打印到ERROR IN ADD cdev!内核信息里（警告）。
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

/*****************************************************************************************
 * void cdev_init(struct cdev *cdev, const struct file_operations *fops)——初始化一个cdev结构
 * @cdev:要初始化的结构
 * @fops:该设备的file_operations
 * 初始化@cdev，存储@fops，使其准备好被cdev_add()添加到系统中。
 * ***************************************************************************************
 * int cdev_add(struct cdev *p, dev_t dev, unsigned count) -向系统中添加一个char类型的设备
 * @p:设备的cdev结构体
 * @dev:这个设备被分配到的第一个设备号
 * @count:该设备对应的连续从设备号个数
 * dev_add()将@p所表示的设备添加到系统中，使其立即处于活动状态。失败时返回一个错误代码。
 * ***************************************************************************************
 * void cdev_del(struct cdev *p) -从系统中移除一个cdev结构体
 * @p:要删除的cdev结构体
 * cdev_del()从系统中移除@p，应该是释放结构体本身。
 * ***************************************************************************************
 * void *kmalloc(size_t size, int flags) -分配内存
 * @size:要分配内存的大小. 以字节为单位.
 * @flags:要分配内存的类型。
 * 参数@flags可以是:
 * %GFP_USER -代表user分配内存。可以睡眠。
 * %GFP_KERNEL -分配普通的内核ram。可以睡眠。
 * %GFP_ATOMIC -分配不会休眠。可以使用应急池。比如，使用内部中断处理程序。
 * %GFP_HIGHUSER -从高内存分配页面。
 * %GFP_NOIO -在获取内存时，不要做任何I/O操作。
 * %GFP_NOFS -当试图获取内存时，不要进行任何fs调用。
 * %GFP_NOWAIT -分配不会休眠。
 * %GFP_THISNODE -只分配节点本地内存。
 * %GFP_DMA -适用于DMA的分配。只能用于kmalloc()缓存。否则，使用SLAB_DMA创建的slab。
 * 也可以通过以下一个或多个附加的@flags来设置不同的标记:
 * %__GFP_COLD -请求cache-cold页面，而不是试图返回cache-warm页面。
 * %__GFP_HIGH -此分配具有高优先级，并可能使用紧急池。
 * %__GFP_NOFAIL -表示这个分配以任何方式都不允许失败（用这个要三思而后行。）
 * %__GFP_NORETRY -如果内存不是立即可用的，那么立即放弃。
 * %__GFP_NOWARN -如果分配失败，不发出任何警告。
 * %__GFP_REPEAT -如果初始分配失败，在失败之前再试一次。
 * 还有其他可用的标志，但这些不是通用的，所以这里没有记录。要获得潜在标志的完整列表，请参考linux/gfp.h。
 * ******************************************************************************************/
