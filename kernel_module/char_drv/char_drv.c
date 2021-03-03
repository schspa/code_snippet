/**
 * 字符设备驱动基本框架
 * 设备驱动名称: "chdrv"
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>

#define CHDRV_MAJOR 0 // 0 ----- 动态申请设备号
                      // 非0 --- 静态设备号
#define CHDRV_NAME  "chdrv"

int chdrv_major = CHDRV_MAJOR;
int chdrv_minor = 0;

/* 设备结构体 */
struct chdrv_dev_t {
	struct cdev cdev;
	// ... 其他成员
};


static int chdrv_open(struct inode *inode, struct file *filp)
{
	/* 从inode中通过cdev指针得到chdrv_dev_t指针 */
	struct chdrv_dev_t *chdrv_dev;
	chdrv_dev = container_of(inode->i_cdev, struct chdrv_dev_t, cdev);

	/**
	 * 将获取到的chdrv_dev_t指针保存在filp的private_data中，
	 * 以便后续其他函数（read/write/ioctl...）使用。
	 * 在这些原型中没有inode参数了，只有filp参数，
	 * 因此通过filp->private_data以保存私有信息，此处为chdrv_dev_t指针。
	 *
	 * inode ------- 描述文件系统中的一个文件（对应于应用程序中的pathname-文件名称）
	 * filp -------- 代表一个打开的文件（对应于应用程序中的fd-文件描述符）
	 */
	filp->private_data = chdrv_dev;

	// ... 其他必要动作

	return 0;
}

static int chdrv_close(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作集合 */
static const struct file_operations chdrv_fops = {
	.owner = THIS_MODULE,
	.open = chdrv_open,
	.release = chdrv_close,
	/*
	.read = chdrv_read,
	.write = chdrv_write,
	.llseek = chdrv_llseek,
	.unlocked_ioctl = chdrv_ioctl,
	*/
};

struct chdrv_dev_t *chdrv_dev;

static int __init char_drv_init(void)
{
	int ret;
	dev_t devno = 0;

	/**
	 * Step1: 设备号-注册静态设备号或动态申请设备号
	 */
	if (chdrv_major) {
		devno = MKDEV(chdrv_major, chdrv_minor);
		ret = register_chrdev_region(devno, 1, CHDRV_NAME);
	} else {
		ret = alloc_chrdev_region(&devno, chdrv_minor, 1, CHDRV_NAME);
		chdrv_major = MAJOR(devno);
	}
	if (ret < 0) {
		printk(KERN_WARNING "chdrv: can't get major %d\n", chdrv_major);
		return ret;
	}

	/**
	 * Step2: cdev结构体-字符设备描述结构体的分配，初始化，注册到系统中
	 */
	chdrv_dev = kmalloc(sizeof(struct chdrv_dev_t), GFP_KERNEL);
	if (!chdrv_dev) {
		ret = -ENOMEM;
		goto FAIL;
	}
	memset(chdrv_dev, 0, sizeof(struct chdrv_dev_t));

	devno = MKDEV(chdrv_major, chdrv_minor);
	cdev_init(&chdrv_dev->cdev, &chdrv_fops);
	chdrv_dev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&chdrv_dev->cdev, devno, 1);
	if (ret) {
		printk(KERN_NOTICE "Error %d adding %s\n", ret, CHDRV_NAME);
	}

	return 0;

FAIL:
	unregister_chrdev_region(devno, 1);
	return ret;
}

static void __exit char_drv_exit(void)
{
	dev_t devno = MKDEV(chdrv_major, chdrv_minor);

	/* Step1: 清理字符设备结构体空间 */
	if (chdrv_dev) {
		cdev_del(&chdrv_dev->cdev);
		kfree(chdrv_dev);
	}

	/* Step2: 清除占用的设备号 */
	unregister_chrdev_region(devno, 1);
}

module_init(char_drv_init);
module_exit(char_drv_exit);
MODULE_AUTHOR("iamcopper<kangpan519@gmail.com>");
MODULE_LICENSE("GPL v2");
