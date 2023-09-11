/*
 * hb_breakpoint.c --- Description
 *
 * Copyright (C) 2023, Schspa, all rights reserved.
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
#include <linux/types.h>
#include <linux/cpumask.h>
#include <linux/hw_breakpoint.h>
#include <linux/kthread.h>
#include <linux/perf_event.h>
#include <asm/hw_breakpoint.h>
#include <linux/module.h>

static uint64_t test_pointer = 0;

static DEFINE_PER_CPU(struct perf_event *, test_bps) = { NULL };

void test_watch_handler(struct perf_event *pe, struct perf_sample_data *psd,
                        struct pt_regs *regs)
{
	uint32_t *code = (void *)instruction_pointer(regs);
	pr_info("%s: code: [%08x]", __func__, *code);
	dump_stack();
	instruction_pointer_set(regs, ((unsigned long) code) + 4);
}


static int hw_breakpoint_test_init(void)
{
	struct perf_event_attr attr = {};
	int cpu;

	cpus_read_lock();
	for_each_online_cpu(cpu) {
		hw_breakpoint_init(&attr);
		attr.bp_addr = (uint64_t)&test_pointer;
		attr.bp_len = HW_BREAKPOINT_LEN_8;
		attr.bp_type = HW_BREAKPOINT_RW;
		attr.type = PERF_TYPE_BREAKPOINT;
		per_cpu(test_bps, cpu) =
			perf_event_create_kernel_counter(&attr, cpu, NULL, test_watch_handler, NULL);
		if (IS_ERR(per_cpu(test_bps, cpu))) {
			pr_err("faile dto create hw breakpoint on cpu %d with status %ld",
				cpu, PTR_ERR(per_cpu(test_bps, cpu)));
			per_cpu(test_bps, cpu) = NULL;
		}
	}
	cpus_read_unlock();
	test_pointer = 0x0;
	test_pointer++;
	pr_info("Test pointer is %lu\n", test_pointer);

	return 0;
}

static void hw_breakpoint_test_exit(void)
{
	int cpu;

	cpus_read_lock();
        for_each_online_cpu(cpu) {
                if (per_cpu(test_bps, cpu))
			perf_event_release_kernel(per_cpu(test_bps, cpu));
	}
	cpus_read_unlock();

	pr_info("Hardware breakpoint monitoring disabled\n");
}

module_init(hw_breakpoint_test_init);
module_exit(hw_breakpoint_test_exit);
MODULE_LICENSE("GPL");
