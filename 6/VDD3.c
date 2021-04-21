#define VIRTUALDISK_SIZE 8192
#define VIRTUALDISK_NUM 200

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/io.h>

typedef struct VirtualDisk
{
	struct cdev cdev;
	unsigned char memory[VIRTUALDISK_SIZE];
	int port1;
	long port2;
	long count;
} vdDriver;

vdDriver *VirtualDisk_devp;
static struct semaphore sem;

int Virtualdisk_open(struct inode *inode,struct file *fp)
{
	if(down_interruptible(&sem))
	{
		return -ERESTARTSYS;
	}
	vdDriver *devp;
	fp->private_data = VirtualDisk_devp;
	devp = fp->private_data;
	devp->count++;
	return 0;
}
int Virtualdisk_release(struct inode *inode,struct file *fp)
{
	vdDriver *devp	= fp->private_data;
	devp->count--;
	up(&sem);
	return 0;
}

static const struct file_operations vdd_fops = {
	.owner = THIS_MODULE,
	.open = Virtualdisk_open,
	.release = Virtualdisk_release,
};

static int majorno = VIRTUALDISK_NUM;

static void setup_cdev(vdDriver *devp,dev_t devno)
{
	int result;
	cdev_init(&devp->cdev,&vdd_fops);
	devp->cdev.owner = THIS_MODULE;
	devp->cdev.ops = &vdd_fops;
	result = cdev_add(&devp->cdev,devno,1);
	if (result<0)
		printk(KERN_NOTICE"ERROR IN ADD cdev!\n");
}
static int VirtualDisk_init(void)
{
	int result;
	dev_t devno = MKDEV(majorno,0);
	if(majorno)
	{
		result = register_chrdev_region(devno,1,"VirtualDisk");
	}
	else 
	{
		result = alloc_chrdev_region(&devno,0,1,"VirtualDisk");
		majorno = MAJOR(devno);
	}
	if(result<0) return result;
	VirtualDisk_devp = kmalloc(sizeof(vdDriver),GFP_KERNEL);
	if(!VirtualDisk_devp)
	{
		result = -ENOMEM;
		goto fail_malloc;
	}
	memset(VirtualDisk_devp,0,sizeof(vdDriver));
	setup_cdev(VirtualDisk_devp,devno);
	printk("Device Regist Successed! devno=%d\n",devno);
	sema_init(&sem,1);
fail_malloc:
	unregister_chrdev_region(devno,1);
	return result;
}
static void VirtualDisk_exit(void)
{
	cdev_del(&VirtualDisk_devp->cdev);
	kfree(VirtualDisk_devp);
	unregister_chrdev_region(MKDEV(majorno,0),1);
	printk("Device Unregist Succeed!\n");
}
module_init(VirtualDisk_init);
module_exit(VirtualDisk_exit);
MODULE_LICENSE("Dual BSD/GPL");

