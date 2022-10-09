// SPDX-License-Identifier: GPL-2.0
/*
 * Test module to generate lockups
 *
 * Copyright (C) 2022, Schspa, all rights reserved.
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/clock.h>
#include <linux/cpu.h>
#include <linux/nmi.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static unsigned int time_secs;
module_param(time_secs, uint, 0600);
MODULE_PARM_DESC(time_secs, "lockup time in seconds, default 0");

static struct hrtimer test_timer;
static rwlock_t test_rw_lock;
static spinlock_t lock;

static enum hrtimer_restart lockdep_test_hrtimer_handler(struct hrtimer *timer)
{
        spin_lock(&lock);
        read_lock(&test_rw_lock);
        mdelay(1);
        read_unlock(&test_rw_lock);
        spin_unlock(&lock);

        hrtimer_forward_now(timer, NSEC_PER_SEC/ 100);

        return HRTIMER_RESTART;
}

static int cmdline_proc_show(struct seq_file *m, void *v)
{
        write_lock_irq(&test_rw_lock);
        mdelay(1);
        write_unlock_irq(&test_rw_lock);
        seq_putc(m, '\n');
        return 0;
}

static int write_proc_show(struct seq_file *m, void *v)
{
        read_lock(&test_rw_lock);
        mdelay(100);
        read_unlock(&test_rw_lock);
        seq_putc(m, '\n');
        return 0;
}

static int __init test_lockdep_init(void)
{
        hrtimer_init(&test_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_HARD);
        test_timer.function = lockdep_test_hrtimer_handler;

        spin_lock_init(&lock);
        rwlock_init(&test_rw_lock);
        hrtimer_start(&test_timer, NSEC_PER_SEC/ 100, HRTIMER_MODE_REL_HARD);

        proc_create_single("lockdep-test-read", 0, NULL, cmdline_proc_show);
        proc_create_single("lockdep-test-write", 0, NULL, write_proc_show);

        return 0;
}
module_init(test_lockdep_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Schspa Shi <schspa@gmail.com>");
MODULE_DESCRIPTION("Test module to generate lockdep warnings");
