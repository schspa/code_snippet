/*
 * cpufreq-qos.c --- Description
 *
 * Copyright (C) 2020, schspa, all rights reserved.
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
#include <linux/cpu.h>
#include <linux/moduleparam.h>
#include <linux/cpumask.h>
#include <linux/cpufreq.h>
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
#include <linux/cpufreq.h>
#include <linux/cdev.h>


#define CPUFREQ_QOS_NAME  "cpufreq-qos"

static int dev_major = 0;
static int dev_minor = 0;
module_param(dev_major, int, 0444);
module_param(dev_minor, int, 0444);

typedef struct {
	struct cdev cdev;
	struct mutex lock;
	struct cpufreq_policy *policy;
	s32 min;
	s32 max;
	int cpu;
	struct device *dev;
} cpu_freq_qos_dev_t;

typedef struct {
	cpu_freq_qos_dev_t *dev;
	struct freq_qos_request qos_req_min, qos_req_max;
} cpu_freq_handle_t;

static DEFINE_PER_CPU(cpu_freq_qos_dev_t, cpufreq_qos_cpu_data);

static void add_cpu_dev_symlink(cpu_freq_qos_dev_t *cpu_data, unsigned int cpu)
{
	struct device *dev = get_cpu_device(cpu);

	if (unlikely(!dev))
		return;

	dev_dbg(dev, "%s: Adding symlink\n", __func__);
	if (sysfs_create_link(&dev->kobj, &cpu_data->dev->kobj, CPUFREQ_QOS_NAME))
		dev_err(dev, "cpufreq symlink creation failed\n");
}

static void remove_cpu_dev_symlink(cpu_freq_qos_dev_t *cpu_data,
				struct device *dev)
{
	(void) cpu_data;
	dev_dbg(dev, "%s: Removing symlink\n", __func__);
	sysfs_remove_link(&dev->kobj, CPUFREQ_QOS_NAME);
}

static int perf_qos_adjust_notify(struct notifier_block *nb, unsigned long event,
			      void *data)
{
	struct cpufreq_policy *policy = data;
	cpu_freq_qos_dev_t *cpu_data;
	int i;

	for_each_cpu(i, policy->related_cpus) {
		cpu_data = &per_cpu(cpufreq_qos_cpu_data, i);

		if (event == CPUFREQ_CREATE_POLICY) {
			cpu_data->policy = policy;

		} else if (event == CPUFREQ_REMOVE_POLICY) {
			cpu_data->policy = NULL; // Clear Policy
		}
	}

	return NOTIFY_OK;
}

static struct notifier_block perf_cpufreq_nb = {
	.notifier_call = perf_qos_adjust_notify,
};

static int cpu_freq_qos_open(struct inode *inode, struct file *filp)
{
	cpu_freq_qos_dev_t *cpu_freq_qos_dev;
	cpu_freq_handle_t *handle = kzalloc(sizeof *handle, GFP_KERNEL);
	int ret;

	if (!handle) {
		return -ENOMEM;
	}
	cpu_freq_qos_dev = container_of(inode->i_cdev, cpu_freq_qos_dev_t, cdev);
	handle->dev = cpu_freq_qos_dev;

	filp->private_data = handle;

	ret = freq_qos_add_request(&cpu_freq_qos_dev->policy->constraints,
			&handle->qos_req_min, FREQ_QOS_MIN, FREQ_QOS_MIN_DEFAULT_VALUE);
	if (ret < 0) {
		return ret;
	}
	ret = freq_qos_add_request(&cpu_freq_qos_dev->policy->constraints,
				&handle->qos_req_max, FREQ_QOS_MAX, FREQ_QOS_MAX_DEFAULT_VALUE);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int cpu_freq_qos_close(struct inode *inode, struct file *filp)
{
	cpu_freq_handle_t *handle = filp->private_data;

	(void) freq_qos_remove_request(&handle->qos_req_min);
	(void) freq_qos_remove_request(&handle->qos_req_max);

	kfree(handle);

	return 0;
}

static ssize_t
cpu_freq_qos_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	cpu_freq_handle_t *handle = file->private_data;
	cpu_freq_qos_dev_t *cpu_data = handle->dev;
	s32 min, max;

	dev_dbg(cpu_data->dev, "%s: count: %zu, pos: %lld\n",
		__func__, count, *ppos);

	if (count < sizeof(s32) * 2) {
		return -ENOBUFS;
	}

	if (*ppos != 0) {
		return 0;
	}

#if 0
	min = freq_qos_read_value(handle->qos_req_min.qos, FREQ_QOS_MIN);
	max = freq_qos_read_value(handle->qos_req_max.qos, FREQ_QOS_MAX);
#else
	min = READ_ONCE(handle->qos_req_min.qos->min_freq.target_value);
	max = READ_ONCE(handle->qos_req_min.qos->max_freq.target_value);
#endif

	if (copy_to_user(buf, &min, sizeof(min))) {
		return -EFAULT;
	}
	if (copy_to_user(buf + sizeof(s32), &max, sizeof(max))) {
		return -EFAULT;
	}

	return sizeof(s32) * 2;
}

static ssize_t cpu_freq_qos_write(struct file *file, const char __user *buf,
                         size_t count, loff_t *ppos)
{
	cpu_freq_handle_t *handle = file->private_data;
	cpu_freq_qos_dev_t *cpu_data = handle->dev;
	s32 min, max;
	int ret;
	ssize_t writen = 0;
	struct cpufreq_policy *policy = cpu_data->policy;

	if (*ppos != 0) {
		return -ENOBUFS;
	}

	if (!policy) {
		return -ENOENT;
	}

	if (count != sizeof(s32) * 2) {
		return -EINVAL;
	}

        if (copy_from_user(&min, buf, sizeof(s32)))
		return -EFAULT;
	if (copy_from_user(&max, buf + sizeof(s32), sizeof(s32)))
		return -EFAULT;

	cpu_data->min = min;
	cpu_data->max = max;

	down_write(&policy->rwsem);
	ret = freq_qos_update_request(&handle->qos_req_min, min);
	if (ret >= 0) {
		writen += sizeof(s32);
	}

	ret = freq_qos_update_request(&handle->qos_req_max, max);
	if (ret >= 0) {
		writen += sizeof(s32);
	}

	refresh_frequency_limits(policy);
	up_write(&policy->rwsem);

	return writen == count ? writen : -EINVAL;
}

static const struct file_operations cpu_freq_qos_fops = {
	.owner = THIS_MODULE,
	.open = cpu_freq_qos_open,
	.release = cpu_freq_qos_close,
	.read = cpu_freq_qos_read,
	.write = cpu_freq_qos_write,
	.llseek = default_llseek,
};

static struct class *cpufreq_qos_class = NULL;

static int __init cpu_freq_qos_drv_init(void)
{
	int ret;
	dev_t devno = 0;
	int cpu;
	cpu_freq_qos_dev_t *cpu_data = NULL;

	cpufreq_qos_class = class_create(THIS_MODULE, CPUFREQ_QOS_NAME);
	if (IS_ERR(cpufreq_qos_class)) {
		ret = PTR_ERR(cpufreq_qos_class);
		pr_warn("Unable to create fb class; errno = %d\n", ret);
		cpufreq_qos_class = NULL;
		return ret;
	}

	ret = alloc_chrdev_region(&devno, dev_minor, CONFIG_NR_CPUS, CPUFREQ_QOS_NAME);
	if (ret < 0) {
		pr_err("alloc chrdev regin faile with status %d\n", ret);
		goto destory_class;
	}

	dev_major = MAJOR(devno);
	dev_minor = MINOR(devno);

	ret = cpufreq_register_notifier(&perf_cpufreq_nb, CPUFREQ_POLICY_NOTIFIER);
	if (ret) {
		pr_err("Failed to register cpufreq notifier with status %d\n",
			ret);
		goto unregister_cpufreq_notifier;
	}

	for_each_present_cpu(cpu) {
		cpu_data = &per_cpu(cpufreq_qos_cpu_data, cpu);
		cpu_data->cpu = cpu;
		mutex_init(&cpu_data->lock);

		cpu_data->policy = cpufreq_cpu_get(cpu);

		cdev_init(&cpu_data->cdev, &cpu_freq_qos_fops);
		cpu_data->cdev.owner = THIS_MODULE;
		devno = MKDEV(dev_major, cpu);

		ret = cdev_add(&cpu_data->cdev, devno, 1);
		if (ret) {
			pr_err("Error %d adding %s\n", ret, CPUFREQ_QOS_NAME);
			goto FAIL;
		}

		cpu_data->dev = device_create(cpufreq_qos_class, NULL,
					devno, NULL, CPUFREQ_QOS_NAME "-%d", cpu);
		if (IS_ERR(cpu_data->dev)) {
			/* Not fatal */
			pr_warn("Unable to create device for "
				"cpufreq-qos-%d errno = %ld\n", cpu, PTR_ERR(cpu_data->dev));
			cpu_data->dev = NULL;
			ret = PTR_ERR(cpu_data->dev);
			goto FAIL;
		}
		add_cpu_dev_symlink(cpu_data, cpu);
	}

	return 0;

FAIL:
	for_each_present_cpu(cpu) {
		cpu_data = &per_cpu(cpufreq_qos_cpu_data, cpu);
		remove_cpu_dev_symlink(cpu_data, get_cpu_device(cpu));

		devno = MKDEV(dev_major, cpu);
		device_destroy(cpufreq_qos_class, devno);
		cdev_del(&cpu_data->cdev);

		cpufreq_cpu_put(cpu_data->policy);
		cpu_data->policy = NULL;
	}

	devno = MKDEV(dev_major, 0);
	unregister_chrdev_region(devno, CONFIG_NR_CPUS);

unregister_cpufreq_notifier:
	cpufreq_unregister_notifier(&perf_cpufreq_nb, CPUFREQ_POLICY_NOTIFIER);

destory_class:
	if (cpufreq_qos_class) {
		class_destroy(cpufreq_qos_class);
		cpufreq_qos_class = NULL;
	}

	return ret;
}

static void __exit cpu_freq_qos_drv_exit(void)
{
	dev_t devno = MKDEV(dev_major, 0);
	cpu_freq_qos_dev_t *cpu_data = NULL;
	int cpu;

	for_each_present_cpu(cpu) {
		cpu_data = &per_cpu(cpufreq_qos_cpu_data, cpu);
		remove_cpu_dev_symlink(cpu_data, get_cpu_device(cpu));

		devno = MKDEV(dev_major, cpu);
		device_destroy(cpufreq_qos_class, devno);
		cdev_del(&cpu_data->cdev);
		cpufreq_cpu_put(cpu_data->policy);
		cpu_data->policy = NULL;
	}

	cpufreq_unregister_notifier(&perf_cpufreq_nb, CPUFREQ_POLICY_NOTIFIER);

	if (dev_major != 0) {
		unregister_chrdev_region(devno, CONFIG_NR_CPUS);
	}

	if (cpufreq_qos_class) {
		class_destroy(cpufreq_qos_class);
		cpufreq_qos_class = NULL;
	}
}

module_init(cpu_freq_qos_drv_init);
module_exit(cpu_freq_qos_drv_exit);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
