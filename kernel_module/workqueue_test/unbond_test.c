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
#include <linux/smp.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>

#define MAX_WORK_NUM 10

struct test_entry;

struct entry_work{
	struct work_struct		work;
	struct test_entry *entry;
};

struct test_entry {
	struct mutex			work_lock;
	struct entry_work works[MAX_WORK_NUM];
	struct hrtimer timer;
	int cpu;
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
	unsigned long delay;

	pr_debug("%s: [%d run on %d]\n", __func__, entry->cpu, smp_processor_id());
	delay = get_random_u32() % 2000;

        mdelay(delay);
}

static void sched_test_work(void *ignored)
{
	struct test_entry *entry = this_cpu_ptr(&pcpu_test_entry);
	int i;

        for (i = 0; i < MAX_WORK_NUM; i++) {
		entry->works[i].entry = entry;
		entry->cpu = smp_processor_id();
		INIT_WORK(&entry->works[i].work, test_work_func);
	}

        hrtimer_init(&entry->timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_PINNED_HARD);
	entry->timer.function =	hrtimer_func;

	hrtimer_start(&entry->timer, ns_to_ktime(NSEC_PER_SEC), HRTIMER_MODE_ABS_PINNED_HARD);
}

static int wq_test_cpu_online(unsigned int cpu) {
	struct test_entry *entry = this_cpu_ptr(&pcpu_test_entry);

        hrtimer_start(&entry->timer, ns_to_ktime(NSEC_PER_SEC), HRTIMER_MODE_ABS_PINNED_HARD);

        return 0;
}

static int wq_test_cpu_offline(unsigned int cpu) {
	struct test_entry *entry = this_cpu_ptr(&pcpu_test_entry);

	hrtimer_cancel(&entry->timer);

        return 0;
}

static int __init proc_workqueue_unbound_test_init(void)
{

	get_online_cpus();
	on_each_cpu(sched_test_work, NULL, true);
	cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN, "wq-test:online",
			wq_test_cpu_online, wq_test_cpu_offline);
	put_online_cpus();

	return 0;
}

static void proc_workqueue_unbound_test_remove(void)
{
}

module_init(proc_workqueue_unbound_test_init);
module_exit(proc_workqueue_unbound_test_remove);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
