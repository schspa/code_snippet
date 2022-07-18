/*
 * unbond_test.c --- Workqueue unbond test
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

#define MAX_WORK_NUM 20
#define MAX_KTHREAD_WORKER 512

struct test_entry;

struct entry_work{
	struct work_struct		work;
	struct test_entry *entry;
};

struct test_entry {
	struct mutex			work_lock;
	struct entry_work works[MAX_WORK_NUM];
	struct entry_work *kworks;
	struct hrtimer timer;
	int cpu;
	struct task_struct *thread;
};

DEFINE_PER_CPU(struct test_entry, pcpu_test_entry);

static enum hrtimer_restart hrtimer_func(struct hrtimer *timer)
{
	struct test_entry *entry = container_of(timer, struct test_entry, timer);
	int i;

	hrtimer_forward_now(timer, ns_to_ktime(2 * NSEC_PER_SEC));

	for (i = 0; i < MAX_WORK_NUM; i++)
		schedule_work(&entry->works[i].work);

	return HRTIMER_RESTART;
}

static void test_work_func(struct work_struct *work)
{
	struct entry_work *work_entry = container_of(work, struct entry_work, work);
	struct test_entry *entry = work_entry->entry;
	unsigned long delay_time;

	pr_debug("%s: [%d run on %d]\n", __func__, entry->cpu, smp_processor_id());
	delay_time = get_random_u32() % (100);

        mdelay(delay_time);
	if (!cpu_active(smp_processor_id())) {
		pr_err("%s: [%d run on a none active %d cpu]\n",
			__func__, entry->cpu, smp_processor_id());
		mdelay(10000);
	}
}

static int test_kthread_func(void *data)
{
	struct test_entry *entry = data;
	unsigned long sleep_time;
	unsigned long timeleft;
	unsigned long seed;
	int i;

	while (!kthread_should_stop()) {
		seed = get_random_u32();
		sleep_time = get_random_u32() % (10 * 1000);
		sleep_time += 300 * 1000;
		timeleft = schedule_timeout_interruptible(sleep_time);
		if (kthread_should_stop() || timeleft)
			break;

		for (i = 0; i < MAX_KTHREAD_WORKER; i+=32) {
			int j;
			seed = get_random_u32();
			for (j = 0; j < 32; j++) {
				if (seed & (1UL << j)) {
					schedule_work(&entry->kworks[i + j].work);
				}
			}
		}
	}

        return 0;
}

static void sched_test_work(void *ignored)
{
	struct test_entry *entry = this_cpu_ptr(&pcpu_test_entry);
	int i;

        for (i = 0; i < MAX_WORK_NUM; i++) {
		entry->works[i].entry = entry;
		INIT_WORK(&entry->works[i].work, test_work_func);
	}
	for (i = 0; i < MAX_KTHREAD_WORKER; i++) {
		entry->kworks[i].entry = entry;
		INIT_WORK(&entry->kworks[i].work, test_work_func);
	}

        hrtimer_init(&entry->timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_PINNED_HARD);
	entry->timer.function =	hrtimer_func;

	hrtimer_start(&entry->timer, ns_to_ktime(NSEC_PER_SEC), HRTIMER_MODE_ABS_PINNED_HARD);
	entry->cpu = smp_processor_id();
}

static int wq_test_cpu_online(unsigned int cpu) {
	struct test_entry *entry = this_cpu_ptr(&pcpu_test_entry);

        hrtimer_start(&entry->timer, ns_to_ktime(NSEC_PER_SEC), HRTIMER_MODE_ABS_PINNED_HARD);

        return 0;
}

static int wq_test_cpu_offline(unsigned int cpu) {
	struct test_entry *entry = this_cpu_ptr(&pcpu_test_entry);

	if (entry->cpu != -1)
		hrtimer_cancel(&entry->timer);

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

	if (!cpu_active(smp_processor_id())) {
		pr_err("Create new worker on a inactive cpu\n");
		mdelay(10000);
	}
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
		func_name, retval, (long long)delta);
	return 0;
}

static struct kretprobe my_kretprobe = {
	.handler		= ret_handler,
	.entry_handler		= entry_handler,
	.data_size		= sizeof(struct my_data),
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 512,
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
	return 0;
}

static void __exit kretprobe_exit(void)
{
	unregister_kretprobe(&my_kretprobe);
	pr_info("kretprobe at %p unregistered\n", my_kretprobe.kp.addr);

	/* nmissed > 0 suggests that maxactive was set too low. */
	pr_info("Missed probing %d instances of %s\n",
		my_kretprobe.nmissed, my_kretprobe.kp.symbol_name);
}

static int g_cpuhp_state = -1;
static int __init proc_workqueue_unbound_test_init(void)
{
	int cpu;
	struct test_entry *entry;

        for_each_possible_cpu(cpu) {
		entry = per_cpu_ptr(&pcpu_test_entry, cpu);
		entry->cpu = -1;
		entry->kworks = kzalloc(sizeof(entry->kworks[0]) * MAX_KTHREAD_WORKER, GFP_KERNEL);
	}

	(void) kretprobe_init();
        cpus_read_lock();
	on_each_cpu(sched_test_work, NULL, true);
	msleep(500);
	g_cpuhp_state = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN, "wq-test:online",
			wq_test_cpu_online, wq_test_cpu_offline);
	for_each_possible_cpu(cpu) {
		entry = per_cpu_ptr(&pcpu_test_entry, cpu);
		entry->thread = kthread_run(test_kthread_func, entry,
					"test_kthread%d", cpu);
	}
	cpus_read_unlock();

	return 0;
}

static void proc_workqueue_unbound_test_remove(void)
{
	struct test_entry *entry;
	int cpu;
	int i;

        for_each_possible_cpu(cpu) {
		entry = per_cpu_ptr(&pcpu_test_entry, cpu);
		kthread_stop(entry->thread);
	}

	if (g_cpuhp_state != -1)
		cpuhp_remove_state(g_cpuhp_state);

	for_each_possible_cpu(cpu) {
		entry = per_cpu_ptr(&pcpu_test_entry, cpu);

		if (entry->cpu == -1)
			continue;

                for (i = 0; i < MAX_WORK_NUM; i++) {
			cancel_work_sync(&entry->works[i].work);
		}
		for (i = 0; i < MAX_KTHREAD_WORKER; i++) {
			cancel_work_sync(&entry->kworks[i].work);
		}
	}

	kretprobe_exit();
        for_each_possible_cpu(cpu) {
		entry = per_cpu_ptr(&pcpu_test_entry, cpu);

		kfree(entry->kworks);
		entry->kworks = NULL;
	}
}

module_init(proc_workqueue_unbound_test_init);
module_exit(proc_workqueue_unbound_test_remove);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
