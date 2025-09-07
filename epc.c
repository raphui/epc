#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>

#include "parser.h"

static FILE *fp_out;

void emit_value(void *a, int width)
{
	fwrite(a, width, 1, fp_out);
}

int main(int argc, char **argv)
{
	int n;
	FILE *fp;
	size_t fsize;
	char *file;
	char cmd[512];
	int ret = 0;

	if (argc < 3) {
		printf("usage: %s parameter_file binary_output_file\n", argv[0]);
		goto err;
	}

	snprintf(cmd, sizeof(cmd), "cpp -P %s", argv[1]);
	fp = popen(cmd, "r");
	if (!fp) {
		printf("failed to preprocess file: %s\n", argv[1]);
		ret = -EIO;
		goto err;
	}

	fp_out = fopen(argv[2], "wb");
	if (!fp) {
		printf("failed to open file: %s\n", argv[2]);
		ret = -EIO;
		goto err_close_fp;
	}

	/* 
	 * start with an arbritary size, we can
	 * realloc in case it is too small
	 */
	fsize = 1024;
	file = malloc(fsize + 1);
	if (!file) {
		printf("failed to allocate %d bytes\n", (int)(fsize + 1));
		ret = -ENOMEM;
		goto err_close_fp_out;
	}

	n = 0;

	while (fread(file + n, sizeof(char), 1, fp)) {
		if (n >= fsize) {
			fsize = n * 2;
			file = realloc(file, fsize);
			if (!file) {
				printf("failed to realloc file memory\n");
				ret = -ENOMEM;
				goto err_close_fp_out;
			}
		}
		n++;
	}

	file[fsize] = '\0';

	ret = parser_process(file, emit_value);

err_close_fp_out:
	fclose(fp_out);
err_close_fp:
	pclose(fp);
err:
	return ret;
}
