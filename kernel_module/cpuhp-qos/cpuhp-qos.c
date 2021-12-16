/*
 * cpu_hp.c --- QOS helper module to limit cpu online number
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
#include <linux/cpumask.h>
#include <linux/cpufreq.h>
#include <trace/events/power.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>

#define CPUHP_QOS_NAME  "cpuhp_qos"

static int dev_major = 0;
static int dev_minor = 0;
module_param(dev_major, int, 0444);
module_param(dev_minor, int, 0444);

struct cpu_hp_dev_t {
	struct cdev cdev;
	struct mutex lock;
	struct device *dev;
};

/* To handle cpufreq min/max request */
struct cpuhp_status {
	unsigned int min;
	unsigned int max;
	spinlock_t lock;
	struct task_struct *hp_thread;
	unsigned int online_cpus; /* current onlined cpu */
};

static struct cpuhp_status cpuhp_stat;

static void wake_up_cpuhp_thread(struct cpuhp_status *stat);

static int need_adjuest(struct cpuhp_status *state)
{
	if (state->online_cpus < state->min || state->online_cpus > state->max)
		return 1;

	return 0;
}

static int cpufreq_hp_offline(unsigned int online_cpu)
{
	unsigned long flags;

	spin_lock_irqsave(&cpuhp_stat.lock, flags);
	cpuhp_stat.online_cpus--;
	if (need_adjuest(&cpuhp_stat)) {
		wake_up_cpuhp_thread(&cpuhp_stat);
	}
	spin_unlock_irqrestore(&cpuhp_stat.lock, flags);

	return 0;
}

static int cpufreq_hp_online(unsigned int online_cpu)
{
	unsigned long flags;

	spin_lock_irqsave(&cpuhp_stat.lock, flags);
	cpuhp_stat.online_cpus++;
	if (need_adjuest(&cpuhp_stat)) {
		wake_up_cpuhp_thread(&cpuhp_stat);
	}
	spin_unlock_irqrestore(&cpuhp_stat.lock, flags);

	return 0;
}

static int core_ctl_online_core(unsigned int cpu)
{
	int ret;

	ret = add_cpu(cpu);

	return ret;
}

static int core_ctl_offline_core(unsigned int cpu)
{
	int ret;

	ret = remove_cpu(cpu);

	return ret;
}

static void __ref do_core_ctl(struct cpuhp_status *stats)
{
	unsigned long flags;
	unsigned int min, max, online;

	spin_lock_irqsave(&cpuhp_stat.lock, flags);
	min = cpuhp_stat.min;
	max = cpuhp_stat.max;
	online = cpuhp_stat.online_cpus;
	spin_unlock_irqrestore(&cpuhp_stat.lock, flags);

	if (online > max) {
		core_ctl_offline_core(online - 1);
	} else if (online < min) {
		core_ctl_online_core(online);
	}
}

static int __ref try_core_ctl(void *data)
{
	struct cpuhp_status *stats = data;
	unsigned long flags;

	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock_irqsave(&stats->lock, flags);
		/* keep running until oneline cpu number is in min ~ max */
		if (!need_adjuest(stats)) {
			spin_unlock_irqrestore(&stats->lock, flags);
			schedule();
			if (kthread_should_stop())
				break;
			spin_lock_irqsave(&stats->lock, flags);
		}
		set_current_state(TASK_RUNNING);
		spin_unlock_irqrestore(&stats->lock, flags);

		do_core_ctl(stats);
	}

	return 0;
}

static void wake_up_cpuhp_thread(struct cpuhp_status *stat)
{
	wake_up_process(stat->hp_thread);
}

static int cpu_hp_open(struct inode *inode, struct file *filp)
{
	struct cpu_hp_dev_t *cpu_hp_dev;

	cpu_hp_dev = container_of(inode->i_cdev, struct cpu_hp_dev_t, cdev);

	filp->private_data = cpu_hp_dev;

	return 0;
}

static int cpu_hp_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t
cpuhp_qos_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned long flags;
	s32 min, max;

	if (count < sizeof(s32) * 2) {
		return -ENOBUFS;
	}

	if (*ppos != 0) {
		return 0;
	}

	spin_lock_irqsave(&cpuhp_stat.lock, flags);
	min = cpuhp_stat.min;
	max = cpuhp_stat.max;
	spin_unlock_irqrestore(&cpuhp_stat.lock, flags);

	if (copy_to_user(buf, &min, sizeof(min))) {
		return -EFAULT;
	}
	if (copy_to_user(buf + sizeof(s32), &max, sizeof(max))) {
		return -EFAULT;
	}

	return sizeof(s32) * 2;
}

static ssize_t cpuhp_qos_write(struct file *file, const char __user *buf,
                         size_t count, loff_t *ppos)
{
	unsigned long flags;
	s32 min, max;

	if (*ppos != 0) {
		return -ENOBUFS;
	}

	if (count == sizeof(s32) * 2) {
		if (copy_from_user(&min, buf, sizeof(s32)))
			return -EFAULT;
		if (copy_from_user(&max, buf + sizeof(s32), sizeof(s32)))
			return -EFAULT;
	}

	spin_lock_irqsave(&cpuhp_stat.lock, flags);
	cpuhp_stat.min = min;
	cpuhp_stat.max = max;
	spin_unlock_irqrestore(&cpuhp_stat.lock, flags);
	wake_up_cpuhp_thread(&cpuhp_stat);

	return count;
}

static const struct file_operations cpu_hp_fops = {
	.owner = THIS_MODULE,
	.open = cpu_hp_open,
	.release = cpu_hp_close,
	.read = cpuhp_qos_read,
	.write = cpuhp_qos_write,
};

struct cpu_hp_dev_t *cpu_hp_dev = NULL;
static struct class *cpuhp_qos_class = NULL;
static enum cpuhp_state cpuhp_online;

static int __init cpu_hp_drv_init(void)
{
	int ret;
	dev_t devno = 0;

	cpuhp_qos_class = class_create(THIS_MODULE, CPUHP_QOS_NAME);
	if (IS_ERR(cpuhp_qos_class)) {
		ret = PTR_ERR(cpuhp_qos_class);
		pr_warn("Unable to create fb class; errno = %d\n", ret);
		cpuhp_qos_class = NULL;
		return ret;
	}


	cpuhp_stat.min = 0;
	cpuhp_stat.max = NR_CPUS;
	spin_lock_init(&cpuhp_stat.lock);

	cpuhp_online = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "hb_perf/qos:online",
					cpufreq_hp_online, cpufreq_hp_offline);
	if (cpuhp_online < 0) {
		pr_warn("failed to register cpuhp online callbacks\n");
		goto destory_class;
	}

	cpuhp_stat.hp_thread =
		kthread_run(try_core_ctl, &cpuhp_stat, "core_ctl");
	if (IS_ERR(cpuhp_stat.hp_thread)) {
		ret = PTR_ERR(cpuhp_stat.hp_thread);
		goto remove_cpuhp_state;
	}

	ret = alloc_chrdev_region(&devno, dev_minor, 1, CPUHP_QOS_NAME);
	if (ret < 0) {
		pr_err("alloc chrdev regin faile with status %d\n", ret);
		goto remove_cpuhp_state;
	}

	dev_major = MAJOR(devno);
	dev_minor = MINOR(devno);

	cpu_hp_dev = kzalloc(sizeof(*cpu_hp_dev), GFP_KERNEL);
	if (!cpu_hp_dev) {
		ret = -ENOMEM;
		goto free_chrdev_region;
	}
	mutex_init(&cpu_hp_dev->lock);

	cdev_init(&cpu_hp_dev->cdev, &cpu_hp_fops);
	cpu_hp_dev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&cpu_hp_dev->cdev, devno, 1);
	if (ret) {
		pr_err("Error %d adding %s\n", ret, CPUHP_QOS_NAME);
		goto free_chrdev;
	}

	cpu_hp_dev->dev = device_create(cpuhp_qos_class, NULL, devno, NULL, CPUHP_QOS_NAME);
	if (IS_ERR(cpu_hp_dev->dev)) {
		/* Not fatal */
		pr_warn("Unable to create device for " CPUHP_QOS_NAME
			" errno = %ld\n", PTR_ERR((cpu_hp_dev->dev)));
		cpu_hp_dev->dev = NULL;
		ret = PTR_ERR((cpu_hp_dev->dev));
		goto remove_cdev;
	}

	pr_info("%s init ok, major=%d, minor=%d\n",
		CPUHP_QOS_NAME, dev_major, dev_minor);

	return 0;

remove_cdev:
	if (cpu_hp_dev) {
		cdev_del(&cpu_hp_dev->cdev);
	}
free_chrdev:
	if (cpu_hp_dev) {
		kfree(cpu_hp_dev);
		cpu_hp_dev = NULL;
	}
free_chrdev_region:
	unregister_chrdev_region(devno, 1);
remove_cpuhp_state:
	cpuhp_remove_state(cpuhp_online);
destory_class:
	if (cpuhp_qos_class) {
		class_destroy(cpuhp_qos_class);
		cpuhp_qos_class = NULL;
	}

	return ret;
}

static void __exit cpu_hp_drv_exit(void)
{
	dev_t devno = MKDEV(dev_major, dev_minor);

	device_destroy(cpuhp_qos_class, devno);

	if (cpu_hp_dev) {
		cdev_del(&cpu_hp_dev->cdev);
		kfree(cpu_hp_dev);
		cpu_hp_dev = NULL;
	}

	unregister_chrdev_region(devno, 1);

	cpuhp_remove_state(cpuhp_online);
	if (cpuhp_qos_class) {
		class_destroy(cpuhp_qos_class);
		cpuhp_qos_class = NULL;
	}

	pr_info("%s exit ok, major=%d, minor=%d\n",
		CPUHP_QOS_NAME, dev_major, dev_minor);
}

module_init(cpu_hp_drv_init);
module_exit(cpu_hp_drv_exit);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
