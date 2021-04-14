#define VIRTUALDISK_SIZE 8192
#define VIRTUALDISK_NUM 200
#define MEM_CLEAR 1
#define PORT1_SET 2
#define PORT2_SET 3

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

int Virtualdisk_open(struct inode *inode,struct file *fp)
{
	vdDriver *devp;
	fp->private_data = VirtualDisk_devp;
	devp = fp->private_data;
	devp->count++;
	return 0;
}
int Virtualdisk_release(struct inode *inode,struct file *fp)
{
	vdDriver *devp = fp->private_data;
	devp->count--;
	return 0;
}
static ssize_t Virtualdisk_read(struct file *fp,char __user *buff,size_t size,loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int result = 0;
	vdDriver *devp = fp->private_data;
	if(p>=VIRTUALDISK_SIZE)
	{
		return count ? - ENXIO:0;
	}
	if(count > VIRTUALDISK_SIZE - p)
	{
		count = VIRTUALDISK_SIZE - p;
	}
	if(copy_to_user(buff,(void*)(devp->memory+p),count))
	{
		result = -EFAULT;
	}
	else
	{
		*ppos += count;
		result = count;
		printk(KERN_INFO"Read %u byte(s) from %lu\n",count,p);
	}
	return result;
}
static ssize_t Virtualdisk_write(struct file *fp,const char __user *buff,size_t size,loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int result = 0;
	vdDriver *devp = fp->private_data;
	if(p>=VIRTUALDISK_SIZE)
	{
		return count ? - ENXIO:0;
	}
	if(count > VIRTUALDISK_SIZE - p)
	{
		count = VIRTUALDISK_SIZE - p;
	}
	if(copy_from_user((void*)(devp->memory+p),buff,count))
	{
		result = -EFAULT;
	}
	else
	{
		*ppos += count;
		result = count;
		printk(KERN_INFO"Written %u byte(s) from %lu\n",count,p);
	}
	return result;
}
static loff_t Virtualdisk_llseek(struct file *fp,loff_t offset,int o)
{
	loff_t result = 0;
	switch(o)
	{
		case SEEK_SET:
			if(offset < 0)
			{
				result = -EINVAL;
				break;
			}
			if((unsigned int)offset > VIRTUALDISK_SIZE)
			{
				result = -EINVAL;
				break;
			}
			fp->f_pos = (unsigned int)offset;
			result = fp->f_pos;
			break;
		case SEEK_CUR:
			if((fp->f_pos + offset)>VIRTUALDISK_SIZE)
			{
				result = -EINVAL;
				break;
			}
			if((fp->f_pos + offset)<0)
			{
				result = -EINVAL;
				break;
			}
			fp->f_pos += offset;
			result = fp->f_pos;
			break;
		default:
			result = -EINVAL;
			break;
	}
	return result;
}
static long Virtualdisk_ioctl(struct file *fp,unsigned int cmd,unsigned long arg)
{
	vdDriver *devp = fp->private_data;
	switch(cmd)
	{
		case MEM_CLEAR:
			memset(devp->memory,0,VIRTUALDISK_SIZE);
			printk(KERN_INFO"Clear Succeed\n");
			break;
		case PORT1_SET:
			devp->port1 = 0;
			break;
		case PORT2_SET:
			devp->port2 = 0;
			break;
		default:
			return -EINVAL;
			break;
	}
	return 0;
}

static const struct file_operations vdd_fops = {
	.owner = THIS_MODULE,
	.llseek = Virtualdisk_llseek,
	.read = Virtualdisk_read,
	.write = Virtualdisk_write,
	.unlocked_ioctl = Virtualdisk_ioctl,
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

