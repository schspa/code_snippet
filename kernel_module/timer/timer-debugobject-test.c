/*
 * timer-debugobject-test.c --- Timer Debug Object test
 *
 * Copyright (C) 2023, Schspa Shi, all rights reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/atomic.h>
#include <linux/kprobes.h>
#include <linux/workqueue.h>

void ktest_timer_func(struct timer_list *);
struct proc_dir_entry *test_entry = NULL;
static DEFINE_TIMER(ktest_timer, ktest_timer_func);
static int timer_stop = 0;
DEFINE_SPINLOCK(tlock);

static DEFINE_PER_CPU(struct work_struct, timer_debugobject_test_work);

static void timer_debugobject_workfn(struct work_struct *work)
{
	mod_timer(&ktest_timer, jiffies + (5 * HZ));
}

static int timer_test_proc_show(struct seq_file *m, void *v)
{
	int cpu;
	seq_printf(m, "%s:0x%p: entry.next = %p, entry.pprev = %p\n",
		__func__, timer_test_proc_show, ktest_timer.entry.next, ktest_timer.entry.pprev);

        for_each_online_cpu(cpu) {
		struct work_struct *work =
			&per_cpu(timer_debugobject_test_work, cpu);
		INIT_WORK(work, timer_debugobject_workfn);
		schedule_work_on(cpu, work);
	}

	return 0;
}

/*
 * Reaper for links from keyrings to dead keys.
 */
void ktest_timer_func(struct timer_list *t)
{
	unsigned long flags;

	spin_lock_irqsave(&tlock, flags);
	if (!timer_stop)
		mod_timer(&ktest_timer, jiffies + (1 * HZ));
	spin_unlock_irqrestore(&tlock, flags);
}

/* per-instance private data */
struct my_data {
	ktime_t entry_stamp;
};

/* Here we use the entry_hanlder to timestamp function entry */
static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct my_data *data;
	int this_cpu = smp_processor_id();

	data = (struct my_data *)ri->data;
	data->entry_stamp = ktime_get();
	pr_info("enter %s\n", ri->rph->rp->kp.symbol_name);
	mdelay(this_cpu * 100);

	return 0;
}

/*
 * Return-probe handler: Log the return value and duration. Duration may turn
 * out to be zero consistently, depending upon the granularity of time
 * accounting on the platform.
 */
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	// struct kretprobe *krp = ri->rp;
	unsigned long retval = regs_return_value(regs);
	struct my_data *data = (struct my_data *)ri->data;
	s64 delta;
	ktime_t now;

	now = ktime_get();
	delta = ktime_to_ns(ktime_sub(now, data->entry_stamp));

	// krp->kp.symbol_name
	pr_info("%s returned %lu and took %lld ns to execute\n",
		ri->rph->rp->kp.symbol_name, retval, (long long)delta);

	return 0;
}

static struct kretprobe my_kretprobes[] = {
	{
		.handler = ret_handler,
		.entry_handler = entry_handler,
		.data_size = sizeof(struct my_data),
		/* Probe up to 20 instances concurrently. */
		.maxactive = 512,
		.kp =
		{
		 .symbol_name = "timer_is_static_object",
		}
	}
};

static int __init kretprobe_init(void)
{
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(my_kretprobes); i++) {
		ret = register_kretprobe(&my_kretprobes[i]);
		if (ret < 0) {
			pr_err("register_kretprobe for %s failed, returned %d\n",
				my_kretprobes[i].kp.symbol_name, ret);
			return -1;
		}
	}

	return 0;
}

static void __exit kretprobe_exit(void)
{
	int i;

	for (i = ARRAY_SIZE(my_kretprobes) - 1; i >= 0; i--) {
		unregister_kretprobe(&my_kretprobes[i]);
		pr_info("kretprobe %s at %p unregistered\n",
			my_kretprobes[i].kp.symbol_name,
			my_kretprobes[i].kp.addr);
	}
}


static __init int timer_debugobject_init(void)
{
	kretprobe_init();

	test_entry = proc_create_single("timer-test", 0, NULL, timer_test_proc_show);

	return 0;
}

static __exit void timer_debugobject_fini(void)
{
	unsigned long flags;

	if (test_entry)
		proc_remove(test_entry);

	test_entry = NULL;

	spin_lock_irqsave(&tlock, flags);
	timer_stop = 0;
	spin_unlock_irqrestore(&tlock, flags);

	del_timer_sync(&ktest_timer);

	kretprobe_exit();
}

module_init(timer_debugobject_init);
module_exit(timer_debugobject_fini);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL");
