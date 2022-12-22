/*
 * complete-uaf.c --- UAF test for complete
 *
 * Copyright (C) 2022, Schspa Shi, all rights reserved.
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

#define pr_fmt(fmt) "complete-uaf-test:" fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/completion.h>

struct test_work {
	struct completion *complete;
	struct pid *caller_pid;
	unsigned long delay_time;
	int id;
};

#define MAX_WAIT_TIMEOUT (50)

static atomic_t test_instance_count;

static bool use_fix = false;
module_param(use_fix, bool, 0444);
MODULE_PARM_DESC(use_fix, "Use fix");

static void mdelay_with_yield(unsigned long timeout_ms)
{
	unsigned long start = jiffies;

	do {
		yield();
	} while (jiffies_to_msecs(jiffies - start) < timeout_ms);

	return;
}

static void test_work_complete(struct test_work *workdata)
{
	struct completion *comp = xchg(&workdata->complete, NULL);

	/* Sleep for 1 millisecond to simulate preemption */
	msleep(100);
	if (comp)
		complete(comp);

	kfree(workdata);
}

static int completion_thread(void *data)
{
	struct test_work *workdata = data;

	mdelay_with_yield(workdata->delay_time);

	/* Simulate an external kill signal */
	kill_pid(workdata->caller_pid, SIGKILL, 1);

	test_work_complete(workdata);

	return 0;
}

static int complete_uaf_test_proc_show(struct seq_file *m, void *v)
{
	struct task_struct *thread;
	DECLARE_COMPLETION_ONSTACK(done);
	struct test_work *workdata;
	int retval;
	int id;

	workdata = kzalloc(sizeof(*workdata), GFP_KERNEL);
        if (!workdata) {
		return -ENOMEM;
	}

	id = atomic_inc_return(&test_instance_count);

	workdata->complete = &done;
	workdata->id = id;
	workdata->delay_time = get_random_u32() % (MAX_WAIT_TIMEOUT);
	workdata->caller_pid = get_pid(task_tgid(current));

	thread = kthread_run(completion_thread, workdata,
			"complete_uaf_test_kthread-%d", workdata->id);
	if (IS_ERR(thread)) {
		seq_printf(m, "kthread create failed with status %ld",
			PTR_ERR(thread));
		kfree(workdata);
		return PTR_ERR(thread);
	}

	retval = wait_for_completion_killable(&done);
	if (retval) {
		if (xchg(&workdata->complete, NULL))
			goto exit;

		if (use_fix) {
			wait_for_completion(&done);
		}
	}

	seq_printf(m, "test %d success\n", id);
exit:
	return 0;
}

static int __init complete_uaf_test_init(void)
{
	proc_create_single("complete_uaf_test", 0, NULL,
			complete_uaf_test_proc_show);

	return 0;
}

module_init(complete_uaf_test_init);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
