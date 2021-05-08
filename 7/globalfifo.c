/*======================================================================
    A globalfifo driver as an example of char device drivers  
    This example is to introduce poll,blocking and non-blocking access
      
    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.

    P.S.:In the Kernel newer than 2.6.37,the function "init_MUTEX" has
    been abolished and the function called "sema_init" is used in the 
    newer version Kernel. Modified by Akatsuki. <github.com/iXeor>
======================================================================*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/slab.h> //kmalloc kfree

#define GLOBALFIFO_SIZE 0x1000 /*全局内存大小*/
#define FIFO_CLEAR 0x1         /*FIFO清零命令*/
#define GLOBALFIFO_MAJOR 200   /*主设备号*/

static int globalfifo_major = GLOBALFIFO_MAJOR;
/*globalfifo设备结构体定义*/
struct globalfifo_dev
{
  struct cdev cdev;
  unsigned int current_len;           /*有效数据长度*/
  unsigned char mem[GLOBALFIFO_SIZE]; /*全局内存*/
  struct semaphore sem;               /*控制并发的信号量*/
  wait_queue_head_t r_wait;           /*阻塞用读队列头*/
  wait_queue_head_t w_wait;           /*阻塞用写队列头*/
};

struct globalfifo_dev *globalfifo_devp; /*定义设备操作指针*/
/*open函数*/
int globalfifo_open(struct inode *inode, struct file *filp)
{
  filp->private_data = globalfifo_devp;
  return 0;
}
/*close函数*/
int globalfifo_release(struct inode *inode, struct file *filp)
{
  return 0;
}

/* ioctl函数 */
static long globalfifo_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  struct globalfifo_dev *dev = filp->private_data; /*����豸�ṹ��ָ��*/

  switch (cmd)
  {
  case FIFO_CLEAR:
    down(&dev->sem); //获取设备指针
    dev->current_len = 0;
    memset(dev->mem, 0, GLOBALFIFO_SIZE);
    up(&dev->sem); //释放设备指针

    printk(KERN_INFO "globalfifo is set to zero\n");
    break;

  default:
    return -EINVAL;
  }
  return 0;
}

static unsigned int globalfifo_poll(struct file *filp, poll_table *wait)
{
  unsigned int mask = 0;
  struct globalfifo_dev *dev = filp->private_data;

  down(&dev->sem);

  poll_wait(filp, &dev->r_wait, wait);
  poll_wait(filp, &dev->w_wait, wait);
  /*fifo非空*/
  if (dev->current_len != 0)
  {
    mask |= POLLIN | POLLRDNORM; 
  }
  /*fifo非满*/
  if (dev->current_len != GLOBALFIFO_SIZE)
  {
    mask |= POLLOUT | POLLWRNORM;
  }

  up(&dev->sem);
  return mask;
}

/*globalfifo读函数*/
static ssize_t globalfifo_read(struct file *filp, char __user *buf, size_t count,
                               loff_t *ppos)
{
  int ret;
  struct globalfifo_dev *dev = filp->private_data; //获取设备操作指针
  DECLARE_WAITQUEUE(wait, current);                //定义等待队列

  down(&dev->sem);                     //获取信号量
  add_wait_queue(&dev->r_wait, &wait); //将当前进程，加入“读”等待队列。

  /*等待数据缓冲区非空，则可读取数据*/
  while (dev->current_len == 0)
  {
    if (filp->f_flags & O_NONBLOCK)
    {
      ret = -EAGAIN;
      goto out;
    }
    __set_current_state(TASK_INTERRUPTIBLE); //改变当前进程为睡眠态，可被信号唤醒。
    up(&dev->sem);

    schedule();                  //调度其他进程运行
    if (signal_pending(current)) //如果是因为信号唤醒
    {
      ret = -ERESTARTSYS;
      goto out2;
    }

    down(&dev->sem); //再次获取信号量，完成数据读取工作。
  }

  //有数据可读，拷贝数据到用户空间
  if (count > dev->current_len)
    count = dev->current_len;

  if (copy_to_user(buf, dev->mem, count))
  {
    ret = -EFAULT;
    goto out;
  }
  else
  {
    memcpy(dev->mem, dev->mem + count, dev->current_len - count); //移动剩余的数据。
    dev->current_len -= count;                                    //有效数据长度减少
    printk(KERN_INFO "read %d bytes(s),current_len:%d\n", count, dev->current_len);

    wake_up_interruptible(&dev->w_wait); //唤醒“写”等待队列。

    ret = count;
  }
out:
  up(&dev->sem); //释放信号量
out2:
  remove_wait_queue(&dev->r_wait, &wait); //将当前进程从读队列中移除
  set_current_state(TASK_RUNNING);
  return ret;
}

/*globalfifo写函数*/
static ssize_t globalfifo_write(struct file *filp, const char __user *buf,
                                size_t count, loff_t *ppos)
{
  struct globalfifo_dev *dev = filp->private_data; //获取设备操作指针
  int ret;
  DECLARE_WAITQUEUE(wait, current); //定义等待队列

  down(&dev->sem);                     //获取信号量
  add_wait_queue(&dev->w_wait, &wait); //将当前进程加入“写”等待队列。

  /* 等待FIFO非满，可写 */
  while (dev->current_len == GLOBALFIFO_SIZE)
  {
    if (filp->f_flags & O_NONBLOCK) //非阻塞访问
    {
      ret = -EAGAIN;
      goto out;
    }
    __set_current_state(TASK_INTERRUPTIBLE); //改变进程状态为睡眠态，信号可唤醒。
    up(&dev->sem);

    schedule();                  //调度其他就绪态进程运行
    if (signal_pending(current)) //因为信号唤醒进程
    {
      ret = -ERESTARTSYS;
      goto out2;
    }

    down(&dev->sem); //获取信号量
  }

  //从用户空间拷贝数据到内核空间，写。
  if (count > GLOBALFIFO_SIZE - dev->current_len)
    count = GLOBALFIFO_SIZE - dev->current_len;

  if (copy_from_user(dev->mem + dev->current_len, buf, count))
  {
    ret = -EFAULT;
    goto out;
  }
  else
  {
    dev->current_len += count;
    printk(KERN_INFO "written %d bytes(s),current_len:%d\n", count, dev->current_len);

    wake_up_interruptible(&dev->r_wait); //唤醒“读”队列

    ret = count;
  }

out:
  up(&dev->sem); //释放信号量
out2:
  remove_wait_queue(&dev->w_wait, &wait); //将当前进程，从“写”队列移除。
  set_current_state(TASK_RUNNING);
  return ret;
}

static const struct file_operations globalfifo_fops =
    {
        .owner = THIS_MODULE,
        .read = globalfifo_read,
        .write = globalfifo_write,
        .unlocked_ioctl = globalfifo_ioctl,
        .poll = globalfifo_poll,
        .open = globalfifo_open,
        .release = globalfifo_release,
};

/*初始化cdev*/
static void globalfifo_setup_cdev(struct globalfifo_dev *dev, int index)
{
  int err, devno = MKDEV(globalfifo_major, index);

  cdev_init(&dev->cdev, &globalfifo_fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops = &globalfifo_fops;
  err = cdev_add(&dev->cdev, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error = %d， add cdev %d\n", err, index);
}

/*FIFO设备初始化*/
int globalfifo_init(void)
{
  int ret;
  dev_t devno = MKDEV(globalfifo_major, 0);

  if (globalfifo_major)
    ret = register_chrdev_region(devno, 1, "globalfifo");
  else
  {
    ret = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
    globalfifo_major = MAJOR(devno);
  }
  if (ret < 0)
    return ret;

  globalfifo_devp = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);

  if (!globalfifo_devp)
  {
    ret = -ENOMEM;
    goto fail_malloc;
  }

  memset(globalfifo_devp, 0, sizeof(struct globalfifo_dev));

  globalfifo_setup_cdev(globalfifo_devp, 0);

  sema_init(&globalfifo_devp->sem, 1);           /*初始化信号量为互斥信号量*/
  init_waitqueue_head(&globalfifo_devp->r_wait); /*初始化写队列头*/
  init_waitqueue_head(&globalfifo_devp->w_wait); /*初始化读队列头*/

  return 0;

fail_malloc:
  unregister_chrdev_region(devno, 1);
  return ret;
}

void globalfifo_exit(void)
{
  cdev_del(&globalfifo_devp->cdev);
  kfree(globalfifo_devp);
  unregister_chrdev_region(MKDEV(globalfifo_major, 0), 1);
}

MODULE_LICENSE("Dual BSD/GPL");

module_param(globalfifo_major, int, S_IRUGO);

module_init(globalfifo_init);
module_exit(globalfifo_exit);
