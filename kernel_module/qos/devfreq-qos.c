/*
 * devfreq-qos.c<qos> --- devfreq-qos
 *
 * Copyright (C) 2021, Schspa, all rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#define pr_fmt(fmt) KBUILD_MODNAME ":QOS: " fmt

//#define DEBUG
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/tick.h>
#include <trace/events/power.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/pm_qos.h>
#include <linux/cdev.h>
#include <linux/devfreq.h>

#define DEVFREQ_QOS_NAME  "devfreq_qos"
#define NR_DEVFREQS 10
#define HZ_PER_KHZ	1000

static int dev_major = 0;
static int dev_minor = 0;
module_param(dev_major, int, 0444);
module_param(dev_minor, int, 0444);
#ifdef CONFIG_DEVFREQ_QOS_DEVICE_COMPATIBLES
static char devfreq_compatibles[64] = CONFIG_DEVFREQ_QOS_DEVICE_COMPATIBLES;
#else
static char devfreq_compatibles[64] = "";
#endif
module_param_string(devfreq_compatibles, devfreq_compatibles, sizeof(devfreq_compatibles), 0);

typedef struct {
	struct cdev cdev;
	struct mutex lock;
	s32 min;
	s32 max;
	struct device *dev;
	struct devfreq *df;
} devfreq_qos_dev_t;

typedef struct {
	devfreq_qos_dev_t *dev;
	struct dev_pm_qos_request qos_req_min, qos_req_max;
} devfreq_qos_handle_t;

static int dev_freq_qos_open(struct inode *inode, struct file *filp)
{
	devfreq_qos_dev_t *dev_freq_qos_dev;
	devfreq_qos_handle_t *handle = kzalloc(sizeof *handle, GFP_KERNEL);
	int ret;

	if (!handle) {
		return -ENOMEM;
	}
	dev_freq_qos_dev = container_of(inode->i_cdev, devfreq_qos_dev_t, cdev);
	handle->dev = dev_freq_qos_dev;

	filp->private_data = handle;

	ret = dev_pm_qos_add_request(dev_freq_qos_dev->df->dev.parent,
				&handle->qos_req_min, DEV_PM_QOS_MIN_FREQUENCY,
				FREQ_QOS_MIN_DEFAULT_VALUE);
	if (ret < 0) {
		return ret;
	}
	ret = dev_pm_qos_add_request(dev_freq_qos_dev->df->dev.parent,
				&handle->qos_req_max, DEV_PM_QOS_MAX_FREQUENCY,
				FREQ_QOS_MAX_DEFAULT_VALUE);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int dev_freq_qos_close(struct inode *inode, struct file *filp)
{
	devfreq_qos_handle_t *handle = filp->private_data;

	(void) dev_pm_qos_remove_request(&handle->qos_req_min);
	(void) dev_pm_qos_remove_request(&handle->qos_req_max);

	kfree(handle);

	return 0;
}

static ssize_t
dev_freq_qos_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	devfreq_qos_handle_t *handle = file->private_data;
	devfreq_qos_dev_t *cpu_data = handle->dev;
	s32 min = 0, max = 0;

	dev_dbg(cpu_data->dev, "%s: count: %zu, pos: %lld\n",
		__func__, count, *ppos);

	if (count < sizeof(s32) * 2) {
		return -ENOBUFS;
	}

	if (*ppos != 0) {
		return 0;
	}

	min = cpu_data->min;
	max = cpu_data->max;

	if (copy_to_user(buf, &min, sizeof(min))) {
		return -EFAULT;
	}
	if (copy_to_user(buf + sizeof(s32), &max, sizeof(max))) {
		return -EFAULT;
	}

	return sizeof(s32) * 2;
}

static ssize_t dev_freq_qos_write(struct file *file, const char __user *buf,
                         size_t count, loff_t *ppos)
{
	devfreq_qos_handle_t *handle = file->private_data;
	devfreq_qos_dev_t *cpu_data = handle->dev;
	struct devfreq *df = cpu_data->df;
	unsigned long freq;
	s32 min, max;
	int ret;
	ssize_t writen = 0;

	if (*ppos != 0) {
		return -ENOBUFS;
	}

	if (count != sizeof(s32) * 2) {
		return -EINVAL;
	}
	if (copy_from_user(&min, buf, sizeof(s32)))
		return -EFAULT;
	if (copy_from_user(&max, buf + sizeof(s32), sizeof(s32)))
		return -EFAULT;

	cpu_data->min = min / HZ_PER_KHZ;
	cpu_data->max = max / HZ_PER_KHZ;

	dev_dbg(handle->dev->dev, "%s: limit: [%d, %d]\n", __func__,
		cpu_data->min, cpu_data->max);

	ret = dev_pm_qos_update_request(&handle->qos_req_min, cpu_data->min);
	if (ret >= 0) {
		writen += sizeof(s32);
	}

	ret = dev_pm_qos_update_request(&handle->qos_req_max, cpu_data->max);
	if (ret >= 0) {
		writen += sizeof(s32);
	}

	if (writen != count) {
		return -EINVAL;
	}

#if 0
	if (df->profile->get_dev_status) /* some driver have no this callback */
		devfreq_update_stats(df);
#else
	df->profile->get_dev_status(df->dev.parent, &df->last_status);
#endif

	mutex_lock(&df->lock);
	(void) update_devfreq(df);
	mutex_unlock(&df->lock);

	if (df->profile->get_cur_freq &&
		!df->profile->get_cur_freq(df->dev.parent, &freq))
		dev_dbg(handle->dev->dev, "%s: freq: %lu\n", __func__, freq);

	return writen;
}

static const struct file_operations dev_freq_qos_fops = {
	.owner = THIS_MODULE,
	.open = dev_freq_qos_open,
	.release = dev_freq_qos_close,
	.read = dev_freq_qos_read,
	.write = dev_freq_qos_write,
	.llseek = default_llseek,
};

static struct class *devfreq_qos_class = NULL;

static int devfreq_qos_register_by_compatible(char *compatible)
{
	struct device_node *np;
	struct devfreq *df;
	devfreq_qos_dev_t *cpu_data = NULL;
	dev_t devno;
	int ret;

        np = of_find_compatible_node(NULL, NULL, compatible);
	if (!np) {
		return -ENOENT;
	}

        df = devfreq_get_devfreq_by_node(np);
	if (!df) {
		pr_err("Faile to get devfreq from %pOF\n", np);
		return -ENODEV;
	}

	cpu_data = kzalloc(sizeof *cpu_data, GFP_KERNEL);
	if (!cpu_data) {
		return -ENOMEM;
	}

	cpu_data->df = df;
	cdev_init(&cpu_data->cdev, &dev_freq_qos_fops);
	cpu_data->cdev.owner = THIS_MODULE;
	devno = MKDEV(dev_major, dev_minor++);

	ret = cdev_add(&cpu_data->cdev, devno, 1);
	if (ret) {
		pr_err("Error %d adding %s\n", ret, DEVFREQ_QOS_NAME);
		goto free_dev;
	}
	cpu_data->dev = device_create(devfreq_qos_class, NULL,
				devno, cpu_data, DEVFREQ_QOS_NAME "-%s",
				dev_name(&cpu_data->df->dev));
	if (IS_ERR(cpu_data->dev)) {
		/* Not fatal */
		pr_warn("Unable to create device for "
			"devfreq-qos-%s errno = %ld\n",
			dev_name(&cpu_data->df->dev), PTR_ERR(cpu_data->dev));
		ret = PTR_ERR(cpu_data->dev);
		cpu_data->dev = NULL;
		goto cdev_del;
	}
	pr_debug("%s devfreq-qos-%s init ok, major=%d, minor=%d\n",
		DEVFREQ_QOS_NAME, dev_name(&cpu_data->df->dev), dev_major, dev_minor);

	return 0;

cdev_del:
	cdev_del(&cpu_data->cdev);
free_dev:
	dev_minor--;
	kfree(cpu_data);

	return ret;
}

static int __init dev_freq_qos_drv_init(void)
{
	int ret;
	dev_t devno = 0;
	char *devfreq_comp, *sep;

	if (devfreq_qos_class == NULL) {
		devfreq_qos_class = class_create(THIS_MODULE, "devfreq-qos");
		if (IS_ERR(devfreq_qos_class)) {
			ret = PTR_ERR(devfreq_qos_class);
			pr_warn("Unable to create devfreq-qos class; errno = %d\n", ret);
			devfreq_qos_class = NULL;
			return ret;
		}
	}

	ret = alloc_chrdev_region(&devno, dev_minor, NR_DEVFREQS, DEVFREQ_QOS_NAME);
	if (ret < 0) {
		pr_err("alloc chrdev regin faile with status %d\n", ret);
		goto destory_class;
	}

	dev_major = MAJOR(devno);
	dev_minor = MINOR(devno);

	pr_debug("%s: devfreq_compatibles = %s\n", __func__, devfreq_compatibles);
	sep = kstrdup(devfreq_compatibles, GFP_KERNEL);
	while ((devfreq_comp = strsep(&sep, ";")) != NULL) {
		pr_debug("devfreq compatible: %s\n", devfreq_comp);
		(void) devfreq_qos_register_by_compatible(devfreq_comp);
	}
	kfree(sep);

	return 0;

destory_class:
	if (devfreq_qos_class) {
		class_destroy(devfreq_qos_class);
		devfreq_qos_class = NULL;
	}

	return ret;
}

static int __exit devfreq_qos_destory(struct device *dev, void *_data)
{
	devfreq_qos_dev_t *cpu_data = dev_get_drvdata(dev);

	device_destroy(devfreq_qos_class, dev->devt);
	cdev_del(&cpu_data->cdev);

	return 0;
}

static void __exit dev_freq_qos_drv_exit(void)
{
	dev_t devno = MKDEV(dev_major, 0);

	class_for_each_device(devfreq_qos_class, NULL, NULL, devfreq_qos_destory);

	if (dev_major != 0) {
		unregister_chrdev_region(devno, NR_DEVFREQS);
	}

	if (devfreq_qos_class) {
		class_destroy(devfreq_qos_class);
		devfreq_qos_class = NULL;
	}
	pr_debug("%s exit ok, major=%d, minor=%d\n",
		DEVFREQ_QOS_NAME, dev_major, dev_minor);
}

module_init(dev_freq_qos_drv_init);
module_exit(dev_freq_qos_drv_exit);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");

