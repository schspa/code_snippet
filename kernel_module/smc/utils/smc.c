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

/**
 * xtou64
 * Take a hex string and convert it to a 64bit number (max 16 hex digits).
 * The string must only contain digits and valid hex characters.
 */
uint64_t xtou64(const char *str)
{
	uint64_t res = 0;
	char c;

	while ((c = *str++)) {
		char v = (c & 0xF) + (c >> 6) | ((c >> 3) & 0x8);
		res = (res << 4) | (uint64_t) v;
	}

	return res;
}

int main (int argc, char **argv)
{
	int aflag = 0;
	int bflag = 0;
	char *smc_helper_path = SMC_HELPER_PATH;
	int index;
	bool use_hex = true;
        uint64_t cmd[SMC_CMD_LENGTH] = { 0x0 };
        uint64_t res[SMC_RES_LENGTH] = { 0x0 };
	int c;
	FILE *smc_f;
	size_t count;
	int ret;

        opterr = 0;

	while ((c = getopt (argc, argv, "hDd:")) != -1)
		switch (c)
		{
		case 'h':
			fprintf(stdout, "Usage %s a0 a1 a2 a3 ...\n", argv[0]);
			return 0;
			break;
		case 'D':
			use_hex = false;
			break;
		case 'd':
			smc_helper_path = optarg;
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

	if (optind == argc) {
		fprintf(stderr, "SMC command argument required\n");
		return 2;
	}

	for (index = optind; index < argc; index++) {
		if (use_hex) {
			cmd[index - optind] = xtou64(argv[index]);
		} else {
			cmd[index - optind] = atol(argv[index]);
		}
		// printf("cmd[%d]: 0x%lx\n", index - optind, cmd[index - optind]);
	}

	smc_f = fopen(smc_helper_path, "r+");
	if (!smc_f) {
		perror(smc_helper_path);
		return errno;
	}

	count = fwrite(cmd, sizeof(cmd), 1, smc_f);
	if (count < 1) {
		fprintf(stderr, "Faile to write command\n");
		fclose(smc_f);
		ret = -EIO;
		goto out;
	}

	count = fread(res, sizeof(res), 1, smc_f);
	if (count < 1) {
		fprintf(stderr, "Faile to read result\n");
		fclose(smc_f);
		ret = -EIO;
		goto out;
	}
	fprintf(stdout, "Res:");
	for (index = 0; index < sizeof(res) / sizeof(res[0]); index++) {
		if (use_hex) {
			fprintf(stdout, "0x%lx", res[index]);
		} else {
			fprintf(stdout, "0x%lu", res[index]);
		}
	}
	fprintf(stdout, "\n");

out:
	fclose(smc_f);

	return 0;
}
