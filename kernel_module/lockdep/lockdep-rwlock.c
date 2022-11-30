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
#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/seq_file.h>

static unsigned int time_secs;
module_param(time_secs, uint, 0600);
MODULE_PARM_DESC(time_secs, "lockup time in seconds, default 0");

static struct hrtimer test_timer;
static rwlock_t test_rw_lock; // hardirq unsafe
static spinlock_t eventlock; // hardirq safe

static enum hrtimer_restart lockdep_test_hrtimer_handler(struct hrtimer *timer)
{
	spin_lock(&eventlock);
	read_lock(&test_rw_lock);
	mdelay(1);
	read_unlock(&test_rw_lock);
	spin_unlock(&eventlock);

	return HRTIMER_NORESTART;
}

static int __init test_lockdep_init(void)
{
	unsigned long flags;

	hrtimer_init(&test_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_HARD);
	test_timer.function = lockdep_test_hrtimer_handler;

	spin_lock_init(&eventlock);
	rwlock_init(&test_rw_lock);
	hrtimer_start(&test_timer, NSEC_PER_SEC/ 100, HRTIMER_MODE_REL_HARD);

	read_lock(&test_rw_lock);
	mdelay(100);
	read_unlock(&test_rw_lock);

	spin_lock_irq(&eventlock);
	write_lock_irqsave(&test_rw_lock, flags);
	mdelay(1);
	write_unlock_irqrestore(&test_rw_lock, flags);
	spin_unlock_irq(&eventlock);

	return 0;
}
module_init(test_lockdep_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Schspa Shi <schspa@gmail.com>");
MODULE_DESCRIPTION("Test module to generate lockdep warnings");
