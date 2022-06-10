/*
 * smc.c --- smc command to perform smc invocation
 *
 * Copyright (C) 2021, Schspa, all rights reserved.
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#define SMC_HELPER_PATH "/dev/smc-helper"
#define SMC_CMD_LENGTH 8
#define SMC_RES_LENGTH 4

#define	HB_IPI_CONTEXT_FID        0x82000100
#define	HB_IPI_CONTEXT_VERSION    0x0
#define	HB_IPI_CONTEXT_READ       0x1
#define	HB_IPI_CONTEXT_TRIGGER    0x2


#define HB_IPI_MAX_CORES 256
#define HB_IPI_MAX_REGS  512

static int smc_call(FILE *smc_f, uint32_t fid, uint32_t x1, uint32_t x2, uint32_t x3,
                    uint32_t *res0, uint32_t *res1)
{
        uint64_t cmd[SMC_CMD_LENGTH] = { 0x0 };
        uint64_t res[SMC_RES_LENGTH] = { 0x0 };
	size_t count;

	cmd[0] = fid;
	cmd[1] = x1;
	cmd[2] = x2;
	cmd[3] = x3;

        count = fwrite(cmd, sizeof(cmd), 1, smc_f);
	if (count < 1) {
		return -EIO;
	}

	count = fread(res, sizeof(res), 1, smc_f);
	if (count < 1) {
		return -EIO;
	}

	if (res[0] != 0) {
		return -EINVAL;
	}
	*res0 = res[1] & 0xffffffff;
	*res1 = res[2] & 0xffffffff;

        return 0;
}

int main (int argc, char **argv)
{
	char *smc_helper_path = SMC_HELPER_PATH;
	char *output_path = "saved_cpu_context.bin";
	int c;
	FILE *smc_f, *out_f;
	int ret;
	union {
		struct {
			uint32_t high;
			uint32_t low;
		};
		uint64_t val;
	} hb_ipi_res;

	while ((c = getopt (argc, argv, "hd:o:")) != -1)
		switch (c)
		{
		case 'h':
			fprintf(stdout, "Usage %s -o <filename> ...\n", argv[0]);
			return 0;
			break;
		case 'd':
			smc_helper_path = optarg;
			break;
		case 'o':
			output_path = optarg;
			break;
		case '?':
			if (optopt == 'd')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr,
					"Unknown option character `\\x%x'.\n",
					optopt);
			return 1;
		default:
			abort ();
		}

        smc_f = fopen(smc_helper_path, "r+");
	if (!smc_f) {
		perror(smc_helper_path);
		return errno;
	}
	out_f = fopen(output_path, "wb+");
	if (!out_f) {
		perror(output_path);
		fclose(smc_f);
		return errno;
	}

	for (int i = 0; i < HB_IPI_MAX_CORES; i++) {
		for (int j = 0; j < HB_IPI_MAX_REGS; j++) {
			ret = smc_call(smc_f, HB_IPI_CONTEXT_FID, HB_IPI_CONTEXT_READ, i, j, &hb_ipi_res.high, &hb_ipi_res.low);
			if (ret)
				break;

			fwrite(&hb_ipi_res.val, sizeof(hb_ipi_res.val), 1, out_f);
			printf("core[%d], reg[%d] = 0x%016lx\n", i, j, hb_ipi_res.val);
		}
	}

	fclose(smc_f);
	fclose(out_f);

	return 0;
}
