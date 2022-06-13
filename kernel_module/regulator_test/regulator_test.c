/*
 * regulator_test.c --- Regulator test
 *
 * Copyright (C) 2022, Schspa, all rights reserved.
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
#define pr_fmt(fmt) KBUILD_MODNAME ":%s: " fmt, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/arm-smccc.h>
#include <linux/regulator/consumer.h>

#define REGULATOR_TEST_NAME  "smc-helper"

static int dev_major = 0;
static int dev_minor = 0;
module_param(dev_major, int, 0444);
module_param(dev_minor, int, 0444);

struct smc_cmd_buf {
	struct list_head node;
	/* arm smccc use a0 - a7 for input */
	unsigned long cmd_buf[8];
	/* arm smccc use a0 - a3 for output */
	unsigned long result_buf[4];
};

struct regulator_test_dev_t {
	struct cdev cdev;
	struct mutex lock;
	struct list_head cmdb_head;
	struct device *dev;
};

static int regulator_test_open(struct inode *inode, struct file *filp)
{
	struct regulator_test_dev_t *regulator_test_dev;

	regulator_test_dev = container_of(inode->i_cdev, struct regulator_test_dev_t, cdev);

	filp->private_data = regulator_test_dev;

	return 0;
}

static int regulator_test_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t
smc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct regulator_test_dev_t *bd = file->private_data;
	ssize_t bytes_read = 0;
	struct smc_cmd_buf *cmdb, *tmp;

	mutex_lock(&bd->lock);
	list_for_each_entry_safe(cmdb, tmp, &bd->cmdb_head, node) {
		if (count < bytes_read + sizeof(cmdb->result_buf))
			break;

		if (copy_to_user(buf, cmdb->result_buf,
					sizeof(cmdb->result_buf))) {
			mutex_unlock(&bd->lock);
			return -EFAULT;
		}

		bytes_read += sizeof(cmdb->result_buf);
		list_del(&cmdb->node);
	}
	mutex_unlock(&bd->lock);

	return bytes_read;
}

static ssize_t smc_write(struct file *file, const char __user *buf,
                         size_t count, loff_t *ppos)
{
	struct regulator_test_dev_t *bd = file->private_data;
	struct arm_smccc_res res;
	struct smc_cmd_buf *cmdb = NULL;

	if (count != sizeof(cmdb->cmd_buf)) {
		pr_err("count %zd is invalid, must be %zd\n", count, sizeof(cmdb->cmd_buf));
		return -EINVAL;
	}

	cmdb = kzalloc(sizeof(*cmdb), GFP_KERNEL);
	if (!cmdb) {
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&cmdb->node);

	if (copy_from_user(cmdb->cmd_buf, buf, count))
		return -EFAULT;

	arm_smccc_smc(cmdb->cmd_buf[0], cmdb->cmd_buf[1], cmdb->cmd_buf[2],
		cmdb->cmd_buf[3], cmdb->cmd_buf[4], cmdb->cmd_buf[5],
		cmdb->cmd_buf[6], cmdb->cmd_buf[7], &res);

	cmdb->result_buf[0] = res.a0;
	cmdb->result_buf[1] = res.a1;
	cmdb->result_buf[2] = res.a2;
	cmdb->result_buf[3] = res.a3;

	mutex_lock(&bd->lock);
	list_add_tail(&cmdb->node, &bd->cmdb_head);
	mutex_unlock(&bd->lock);

	return count;
}

static const struct file_operations regulator_test_fops = {
	.owner = THIS_MODULE,
	.open = regulator_test_open,
	.release = regulator_test_close,
	.read = smc_read,
	.write = smc_write,
};

static struct class *regulator_test_class = NULL;

static int __init regulator_test_drv_init(void)
{
	struct regulator_test_dev_t *regulator_test_dev = NULL;
	struct regulator *regu;
	dev_t devno = 0;
	int ret;

	if (regulator_test_class == NULL) {
		regulator_test_class = class_create(THIS_MODULE, "regulator-test");
		if (IS_ERR(regulator_test_class)) {
			ret = PTR_ERR(regulator_test_class);
			pr_warn("Unable to create devfreq-qos class; errno = %d\n", ret);
			regulator_test_class = NULL;
			return ret;
		}
	}

	ret = alloc_chrdev_region(&devno, dev_minor, 1, REGULATOR_TEST_NAME);
	if (ret < 0) {
		pr_err("alloc chrdev regin faile with status %d\n", ret);
		return ret;
	}

	dev_major = MAJOR(devno);
	dev_minor = MINOR(devno);

	regulator_test_dev = kzalloc(sizeof(*regulator_test_dev), GFP_KERNEL);
	if (!regulator_test_dev) {
		ret = -ENOMEM;
		goto FAIL;
	}

	mutex_init(&regulator_test_dev->lock);
	INIT_LIST_HEAD(&regulator_test_dev->cmdb_head);

	cdev_init(&regulator_test_dev->cdev, &regulator_test_fops);
	regulator_test_dev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&regulator_test_dev->cdev, devno, 1);
	if (ret) {
		pr_err("Error %d adding %s\n", ret, REGULATOR_TEST_NAME);
		goto free_dev;
	}
	regulator_test_dev->dev = device_create(regulator_test_class, NULL, devno,
					regulator_test_dev, REGULATOR_TEST_NAME);
	if (IS_ERR(regulator_test_dev->dev)) {
		/* Not fatal */
		pr_warn("Unable to create device for " REGULATOR_TEST_NAME
			" errno = %ld\n",
			PTR_ERR(regulator_test_dev->dev));
		ret = PTR_ERR(regulator_test_dev->dev);
		regulator_test_dev->dev = NULL;
	}

	regu = devm_regulator_get(regulator_test_dev->dev, "VDD_BPU0_0V8");
	if (!IS_ERR(regu)) {
		ktime_t now;
		s64 delta;
		now = ktime_get();
		ret = regulator_set_voltage(regu, 850000, 910000);
		delta = ktime_to_ns(ktime_sub(ktime_get(), now));
		pr_info("set voltage returned %d and took %lld ns to execute\n",
			ret, (long long)delta);
	}

	pr_info("%s init ok, major=%d, minor=%d\n",
		REGULATOR_TEST_NAME, dev_major, dev_minor);

	return 0;

free_dev:
	kfree(regulator_test_dev);
FAIL:
	unregister_chrdev_region(devno, 1);
	return ret;
}

static int __exit regulator_test_destory(struct device *dev, void *_data)
{
	struct regulator_test_dev_t *regulator_test_dev = dev_get_drvdata(dev);
	(void) _data;

        device_destroy(regulator_test_class, dev->devt);
	cdev_del(&regulator_test_dev->cdev);
	kfree(regulator_test_dev);
	dev_minor--;

	return 0;
}

static void __exit regulator_test_drv_exit(void)
{
	dev_t devno = MKDEV(dev_major, 0);

        class_for_each_device(regulator_test_class, NULL, NULL, regulator_test_destory);

	unregister_chrdev_region(devno, 1);

	if (regulator_test_class) {
		class_destroy(regulator_test_class);
		regulator_test_class = NULL;
	}

	pr_info("%s exit ok, major=%d, minor=%d\n",
		REGULATOR_TEST_NAME, dev_major, dev_minor);
}

module_init(regulator_test_drv_init);
module_exit(regulator_test_drv_exit);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
