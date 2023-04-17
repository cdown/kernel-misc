/*
 * afl-gcc loglevel.c -fno-omit-frame-pointer -Og -ggdb -o loglevel
 * afl-fuzz -i fuzz-in -o fuzz-out -f afl ./loglevel
 */
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

#define clamp(a, b, c) (a)
#define CON_LOGLEVEL 128

struct console_cmdline {
	char *options;
	int level;
	short flags;
};

#define strscpy strncpy

static bool find_and_remove_console_option(char *buf, size_t size, char *wanted,
					   char *options)
{
	bool found = false, first = true;
	char *item, *opt = options;

	while ((item = strsep(&opt, ",")) != NULL) {
		char *key = item, *value;

		value = strchr(item, ':');
		if (value)
			*(value++) = '\0';

		if (strcmp(key, wanted) == 0) {
			found = true;
			if (value)
				strscpy(buf, value, size);
			else
				*buf = '\0';
		}

		if (!found && opt)
			*(opt - 1) = ',';
		if (!found && value)
			*(value - 1) = ':';
		if (!first)
			*(item - 1) = ',';

		if (found)
			break;

		first = false;
	}

	if (found) {
		if (opt)
			memmove(item, opt, strlen(opt) + 1);
		else if (first)
			*item = '\0';
		else
			*--item = '\0';
	}

	return found;
}

int main(int argc, char *argv[])
{
	struct console_cmdline con = { 0 };
	FILE *fp = NULL;
	long size;
	char *cmdline;
	char level[16];
	bool found;

	if (argc == 2)
		cmdline = argv[1];
	else {
		fp = fopen("afl", "rb");
		if (!fp) {
			perror("afl");
			exit(1);
		}

		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);
		rewind(fp);

		cmdline = calloc(1, size + 1);
		if (!cmdline) {
			fclose(fp);
			perror("calloc");
			exit(1);
		}

		if (fread(cmdline, size, 1, fp) != 1) {
			fclose(fp);
			free(cmdline);
			perror("fread");
			exit(1);
		}
	}

	printf("cmdline: %s\n", cmdline);
	printf("After removing loglevel:\n");
	found = find_and_remove_console_option(level, sizeof(level), "loglevel",
					       cmdline);
	con.options = cmdline;
	printf("options passed to driver: %s\n", con.options);
	printf("command line options: %s\n", cmdline);
	if (found)
		printf("level: %s\n", level);
	else
		printf("level not found");

	if (fp) {
		fclose(fp);
		free(cmdline);
	}
}
