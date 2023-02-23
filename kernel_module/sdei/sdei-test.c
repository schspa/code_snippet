/*
 * sdei-test.c --- SDEI test module
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
#define pr_fmt(fmt) KBUILD_MODNAME ":%s: " fmt, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/arm-smccc.h>
#include <linux/arm_sdei.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/smp.h>
#include <linux/errno.h>
#include <linux/cpu.h>


static u32 sdei_event = 0;
struct proc_dir_entry *sdei_test_proc = NULL;

#define SDEI_EVENT_SIGNAL_FID 0xC400002F

struct cpu_info_ctx {
	struct completion complete;
	uint64_t mpidr;
	int cpu;
};

static DEFINE_PER_CPU(struct cpu_info_ctx, cpuctxs);


static int cpu_info_correct(void *data)
{
	int cpu = raw_smp_processor_id();
        struct cpu_info_ctx *ctx = &per_cpu(cpuctxs, cpu);
	unsigned long reg;

	ctx->cpu = cpu;
	__asm__ volatile ("mrs %0, mpidr_el1" : "=r" (reg));
	ctx->mpidr = reg;

        return 0;
}


int sdei_event_test_handler(u32 event, struct pt_regs *regs, void *arg)
{
	int cpu = raw_smp_processor_id();
        struct cpu_info_ctx *ctx = &per_cpu(cpuctxs, cpu);

	pr_info("cpu[%d], sdei event %u handler\n", ctx->cpu, event);
	complete(&ctx->complete);

        return 0;
}

static int sdei_test_proc_show(struct seq_file *m, void *v)
{
	struct arm_smccc_res res;
	unsigned long timeout;
	int cpu;
	int ret = 0;

	cpus_read_lock();
	for_each_online_cpu(cpu) {
		struct cpu_info_ctx *ctx = &per_cpu(cpuctxs, cpu);

		init_completion(&ctx->complete);

		ret = smp_call_on_cpu(cpu, cpu_info_correct, NULL, true);
		if (ret) {
			pr_err("cpu[%d] smp call failed", cpu);
			goto error;
		}
		seq_printf(m, "cpu[%d] mpidr: 0x%016llx\n", ctx->cpu, ctx->mpidr);
		arm_smccc_smc(SDEI_EVENT_SIGNAL_FID, sdei_event, ctx->mpidr,
			0x0, 0x0, 0x0, 0x0, 0x0, &res);
		seq_printf(m, "smc event inject return 0x%016lx\n", res.a0);

                timeout = wait_for_completion_timeout(&ctx->complete, HZ * 5);
		if (timeout == 0) {
			pr_err("cpu[%d] sdei callback timedout\n",
				ctx->cpu);
			ret = -ETIMEDOUT;
			goto error;
		}

		seq_printf(m, "cpu[%d] sdei callback ok\n", ctx->cpu);
	}

error:
	cpus_read_unlock();

	return ret;
}

static int __init sdei_test_drv_init(void)
{
	int ret;

	ret = sdei_event_register(sdei_event, sdei_event_test_handler, NULL);
	if (ret) {
		pr_err("sdei_evnet_register failed with status %d\n", ret);
		return 0;
	}

	ret = sdei_event_enable(sdei_event);
	if (ret) {
		pr_err("sdei_evnet_enable failed with status %d\n", ret);
		goto unreg;
	}

        pr_info("sdei_evnet %d register success\n", sdei_event);

	sdei_test_proc = proc_create_single("sdei-test", 0, NULL, sdei_test_proc_show);

        return 0;
unreg:
	ret = sdei_event_unregister(sdei_event);
	if (ret)
		pr_err("sdei_evnet_unregister failed with status %d\n", ret);

	return ret;
}

static void __exit sdei_test_drv_exit(void)
{
	int ret;

	if (sdei_test_proc) {
		proc_remove(sdei_test_proc);
	}

        ret = sdei_event_unregister(sdei_event);
	if (ret)
		pr_err("sdei_evnet_unregister failed with status %d\n", ret);
}

module_init(sdei_test_drv_init);
module_exit(sdei_test_drv_exit);
MODULE_AUTHOR("Schspa <schspa@gmail.com>");
MODULE_LICENSE("GPL v2");
