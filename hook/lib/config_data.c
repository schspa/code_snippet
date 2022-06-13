/*
 * config_data.c --- Description
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>
#include "config_data.h"

char* read_file(const char *filename) {
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';


cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}

static cJSON *parse_file(const char *filename)
{
	cJSON *parsed = NULL;
	char *content = read_file(filename);

	parsed = cJSON_Parse(content);

	if (content != NULL)
	{
		free(content);
	}

	return parsed;
}

/*
 * { mac : [ {"ifr_name" : "eno0", "mac" : "b3:26:83:2c:73:ec"}]}
 */
int parse_config_file(struct MacHookTable *table, int len)
{
	const cJSON *sensors = NULL;
	const cJSON *sensor = NULL;
	const cJSON *tree = NULL;
	int ret = 0;
	int i;

	tree = parse_file(THERMAL_CONFIG_FILE_PATH);
	if (tree == NULL) {
	    const char *error_ptr = cJSON_GetErrorPtr();
	    if (error_ptr != NULL)
	    {
		fprintf(stderr, "Error before: %s\n", error_ptr);
	    }
	    return -EINVAL;
	}

	cJSON_Print(tree);

	sensors = cJSON_GetObjectItemCaseSensitive(tree, "mac");
	cJSON_ArrayForEach(sensor, sensors)
	{
	    cJSON *ifr_name = cJSON_GetObjectItemCaseSensitive(sensor, "ifr_name");
	    cJSON *mac = cJSON_GetObjectItemCaseSensitive(sensor, "mac");

	    if (!cJSON_IsString(ifr_name)) {
		ret = -EINVAL;
		goto end;
	    }
	    if (!cJSON_IsString(mac)) {
		ret = -EINVAL;
		goto end;
	    }

	    printf("mapping: %s -> %s\n", ifr_name->valuestring, mac->valuestring);

	    for (i = 0; i < len; i++) {
		if (strcasecmp(table[i].name, tz_type->valuestring))
		    continue;

		table[i].name =	strdup(thermal_type->valuestring);
	    }
	}

end:
	cJSON_Delete(tree);
	return ret;
}
