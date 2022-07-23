/*
 * work-test.c --- workqueue test
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
#define pr_fmt(fmt) "work-test:" fmt

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/spinlock.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/kprobes.h>
#include <linux/slab.h>
#include <linux/idr.h>

struct test_work {
	struct work_struct work;
	int last_run_cpu;
	int timer_cpu;
	int kthread_cpu;
	int delay;
	unsigned long delay_time;
	struct hrtimer timer;
	struct list_head node;
	struct task_struct *kthread;
	int id;
};

DEFINE_IDR(test_work_idr);
static DEFINE_MUTEX(test_work_lock);

static int num_work = 8;
module_param(num_work, int, 0444);

static struct proc_dir_entry *proc_test_info;

static void mdelay_with_yield(unsigned long timeout_ms)
{
	    unsigned long start = jiffies;

	    migrate_disable();
	    do {
		    yield();
	    } while (jiffies_to_msecs(jiffies - start) < timeout_ms);
	    migrate_enable();

            return;
}

static int c_show(struct seq_file *m, void *v)
{
	struct test_work *entry = v;

	seq_printf(m, "%5u %10d %10d %10d %15lu\n",
		entry->id,
		entry->timer_cpu,
		entry->last_run_cpu,
		entry->kthread_cpu,
		entry->delay_time);

        return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	void *entry;
	int id;

	seq_printf(m, "%5s %10s %10s %10s %15s\n", "id",
		"timer_cpu", "work_cpu",
		"kthread_cpu",  "delay_time");

	mutex_lock(&test_work_lock);

	id = *pos;

	entry = idr_get_next(&test_work_idr, &id);
	*pos = id + 1;

	return entry;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	void *entry;
	int id = *pos;

        entry = idr_get_next(&test_work_idr, &id);
	*pos = id + 1;

        return entry;
}

static void c_stop(struct seq_file *m, void *v)
{
	mutex_unlock(&test_work_lock);
}

static const struct seq_operations work_test_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_show
};

static int work_test_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &work_test_op);
}

static const struct proc_ops cpuinfo_proc_ops = {
	.proc_flags	= PROC_ENTRY_PERMANENT,
	.proc_open	= work_test_open,
	.proc_read_iter	= seq_read_iter,
	.proc_lseek	= seq_lseek,
	.proc_release	= seq_release,
};

static enum hrtimer_restart hrtimer_func(struct hrtimer *timer)
{
	struct test_work *entry = container_of(timer, struct test_work, timer);

	entry->timer_cpu = raw_smp_processor_id();
	hrtimer_forward_now(timer, ms_to_ktime(entry->delay_time));

	schedule_work(&entry->work);

	return HRTIMER_RESTART;
}

static void test_work_func(struct work_struct *work)
{
	struct test_work *entry = container_of(work, struct test_work, work);
	unsigned long delay_ms;

	entry->last_run_cpu = raw_smp_processor_id();

	pr_debug("%s: [%d run on %d]\n", __func__, entry->last_run_cpu,
		raw_smp_processor_id());
	delay_ms = get_random_u32() % (3);
	/* 200 -> 300 ms */
	entry->delay_time = get_random_u32() % 2;
	entry->delay_time += 5;

        mdelay_with_yield(delay_ms);
	if (!cpu_online(raw_smp_processor_id())) {
		mdelay_with_yield(200);
	}
}

static int test_kthread_func(void *data)
{
	struct test_work *entry = data;
	unsigned long sleep_time;
	unsigned long timeleft;

	while (!kthread_should_stop()) {
		/* sleep 1 - 1.5s*/
		sleep_time = get_random_u32() % (1000);
		timeleft = schedule_timeout_interruptible(sleep_time + 500);
		if (kthread_should_stop() || timeleft)
			break;

		entry->kthread_cpu = raw_smp_processor_id();
		/* delay for 0 - 500 ms*/
		sleep_time = get_random_u32() % (500);
		mdelay_with_yield(sleep_time);
		hrtimer_cancel(&entry->timer);
		hrtimer_start_expires(&entry->timer, HRTIMER_MODE_ABS_HARD);
	}

        return 0;
}

static char func_name[NAME_MAX] = "worker_thread";
module_param_string(func, func_name, NAME_MAX, S_IRUGO);
MODULE_PARM_DESC(func, "Function to kretprobe; this module will report the"
		" function's execution time");

/* per-instance private data */
struct my_data {
	ktime_t entry_stamp;
};

/* Here we use the entry_hanlder to timestamp function entry */
static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct my_data *data;


	data = (struct my_data *)ri->data;
	data->entry_stamp = ktime_get();
	pr_debug("%s: %s\n", __func__, func_name);

        mdelay(50);

        return 0;
}

/*
 * Return-probe handler: Log the return value and duration. Duration may turn
 * out to be zero consistently, depending upon the granularity of time
 * accounting on the platform.
 */
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	unsigned long retval = regs_return_value(regs);
	struct my_data *data = (struct my_data *)ri->data;
	s64 delta;
	ktime_t now;

	now = ktime_get();
	delta = ktime_to_ns(ktime_sub(now, data->entry_stamp));
	pr_info("%s returned %lu and took %lld ns to execute\n",
		__func__, retval, (long long)delta);

        return 0;
}

static struct kretprobe my_kretprobe = {
	.handler		= ret_handler,
	.entry_handler		= entry_handler,
	.data_size		= sizeof(struct my_data),
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 512,
};

/* Here we use the entry_hanlder to timestamp function entry */
static int wq_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct my_data *data;


	data = (struct my_data *)ri->data;
	data->entry_stamp = ktime_get();
	pr_debug("%s: [%d]\n", __func__, raw_smp_processor_id());

        return 0;
}

/*
 * Return-probe handler: Log the return value and duration. Duration may turn
 * out to be zero consistently, depending upon the granularity of time
 * accounting on the platform.
 */
static int wq_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	unsigned long retval = regs_return_value(regs);
	struct my_data *data = (struct my_data *)ri->data;
	s64 delta;
	ktime_t now;

	now = ktime_get();
	delta = ktime_to_ns(ktime_sub(now, data->entry_stamp));
	pr_info("%s returned %lu and took %lld ns to execute\n",
		__func__, retval, (long long)delta);

	return 0;
}

static struct kretprobe my_wq_offline_kretprobe = {
	.handler = wq_ret_handler,
	.entry_handler = wq_entry_handler,
	.data_size = sizeof(struct my_data),
	.maxactive = 128,
};

static int __init kretprobe_init(void)
{
	int ret;

	my_kretprobe.kp.symbol_name = func_name;
	ret = register_kretprobe(&my_kretprobe);
	if (ret < 0) {
		pr_err("register_kretprobe failed, returned %d\n", ret);
		return -1;
	}
	pr_info("Planted return probe at %s: %p\n",
		my_kretprobe.kp.symbol_name, my_kretprobe.kp.addr);

	my_wq_offline_kretprobe.kp.symbol_name = "workqueue_offline_cpu";
	ret = register_kretprobe(&my_wq_offline_kretprobe);
	if (ret < 0) {
		pr_err("register_kretprobe failed, returned %d\n", ret);
		return -1;
	}
	pr_info("Planted return probe at %s: %p\n",
		my_wq_offline_kretprobe.kp.symbol_name, my_wq_offline_kretprobe.kp.addr);


	return 0;
}

static void __exit kretprobe_exit(void)
{
	unregister_kretprobe(&my_kretprobe);
	pr_info("kretprobe at %p unregistered\n", my_kretprobe.kp.addr);

	unregister_kretprobe(&my_wq_offline_kretprobe);
	pr_info("kretprobe at %p unregistered\n", my_wq_offline_kretprobe.kp.addr);
}

static int __init proc_workqueue_unbound_test_init(void)
{
	struct test_work *entry;
	int rc;
	int i;

	(void) kretprobe_init();

	for (i = 0; i < num_work; i++) {
		mutex_lock(&test_work_lock);
		rc = idr_alloc(&test_work_idr, NULL, 0, num_work + 1, GFP_KERNEL);
		mutex_unlock(&test_work_lock);

		if (rc < 0) {
			pr_err("%s: No available test_work numbers\n", __func__);
			return rc;
		}

		entry = kzalloc(sizeof(*entry), GFP_KERNEL);

		entry->id = rc;
		INIT_WORK(&entry->work, test_work_func);

		hrtimer_init(&entry->timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_HARD);
		entry->timer.function =	hrtimer_func;
		hrtimer_start(&entry->timer, ns_to_ktime(NSEC_PER_SEC), HRTIMER_MODE_ABS_HARD);

		entry->kthread = kthread_run(test_kthread_func, entry,
					"test_kthread%d", entry->id);
		mutex_lock(&test_work_lock);
		idr_replace(&test_work_idr, entry, entry->id);
		mutex_unlock(&test_work_lock);
	}

	proc_test_info = proc_create("wq-test-info", 0, NULL, &cpuinfo_proc_ops);

	return 0;
}

int item_idr_free(int id, void *p, void *data)
{
	struct test_work *entry = p;

	if (!entry)
		return 0;

	kthread_stop(entry->kthread);
	hrtimer_cancel(&entry->timer);
	cancel_work_sync(&entry->work);

	kfree(entry);

	return 0;
}

static void proc_workqueue_unbound_test_remove(void)
{
	proc_remove(proc_test_info);

	mutex_lock(&test_work_lock);
	idr_for_each(&test_work_idr, item_idr_free, &test_work_idr);
	idr_destroy(&test_work_idr);
	mutex_unlock(&test_work_lock);

	kretprobe_exit();
}

module_init(proc_workqueue_unbound_test_init);
module_exit(proc_workqueue_unbound_test_remove);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
