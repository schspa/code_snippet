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
#include <linux/hrtimer.h>

static struct proc_dir_entry *test_proc = NULL;
struct work_struct test_work;

DEFINE_RAW_SPINLOCK(test_lock);
static DEFINE_PER_CPU(struct hrtimer, diag_cpu_hrtimer);

static enum hrtimer_restart hrtimer_func(struct hrtimer *timer)
{
	ktime_t set_time;

	core0_heartbeat++;
	if (core0_heartbeat >= DIAG_ACORE_MAX_HEARTBEAT) { /* MAX heartbeat check */
		core0_heartbeat = 0;
	}
	__raw_writel(core0_heartbeat, g_acore_cpu0_heart_cnt);

	set_time = ktime_set(DIAG_HEATBEAT_TIME_INTERVAL / 1000,
			(DIAG_HEATBEAT_TIME_INTERVAL % 1000) * 1000000);
	hrtimer_forward(timer, timer->base->get_time(), set_time);

	return HRTIMER_RESTART;
}

static void test_work_fn(struct work_struct *work)
{
	/* 10 s delay for test */
	raw_spin_lock_bh(&test_lock);
	mdelay(100 * 1000);
	raw_spin_unlock_bh(&test_lock);
}

static int bh_test_show(struct seq_file *m, void *v)
{
	struct workqueue_struct *test_wq;
	seq_puts(m, __func__);
	seq_putc(m, '\n');
	test_wq = alloc_workqueue("unbound_test_workqueue", WQ_HIGHPRI, 0);
	INIT_WORK(&test_work, test_work_fn);
	queue_work_on(1, test_wq, &test_work);
	destroy_workqueue(test_wq);
	return 0;
}

static int __init proc_bh_test_init(void)
{
	test_proc = proc_create_single("bh_test", 0, NULL, bh_test_show);
	return 0;
}

static void proc_bh_test_remove(void)
{
	proc_remove(test_proc);
}

module_init(proc_bh_test_init);
module_exit(proc_bh_test_remove);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
