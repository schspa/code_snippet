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

static struct proc_dir_entry *test_proc = NULL;
struct work_struct test_work;

DEFINE_RAW_SPINLOCK(test_lock);

static void test_work_fn(struct work_struct *work)
{
	/* 10 s delay for test */
	raw_spin_lock_bh(&test_lock);
	mdelay(100 * 1000);
	raw_spin_unlock_bh(&test_lock);
}

static int workqueue_unbound_test_show(struct seq_file *m, void *v)
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

static int __init proc_workqueue_unbound_test_init(void)
{
	test_proc = proc_create_single("workqueue_unbound_test", 0, NULL, workqueue_unbound_test_show);
	return 0;
}

static void proc_workqueue_unbound_test_remove(void)
{
	proc_remove(test_proc);
}

module_init(proc_workqueue_unbound_test_init);
module_exit(proc_workqueue_unbound_test_remove);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
