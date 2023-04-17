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

struct console_option {
	char *key;
	char *value;
};

static struct console_option find_and_remove_console_option(char *wanted,
							    char *options)
{
	struct console_option ret = { NULL, NULL };
	bool found = false, first = true;
	char *item, *opt = options;

	while ((item = strsep(&opt, ",")) != NULL) {
		char *key = item, *value;

		value = strchr(item, ':');
		if (value)
			*(value++) = '\0';

		if (strcmp(key, wanted) == 0) {
			found = true;
			ret.key = strdup(key);
			if (value)
				ret.value = strdup(value);
		}

		if (opt)
			*(opt - 1) = ',';
		if (!first)
			*(item - 1) = ',';
		if (value)
			*(value - 1) = ':';

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

	return ret;
}

int main(int argc, char *argv[])
{
	struct console_cmdline con = { 0 };
	FILE *fp = NULL;
	long size;
	char *cmdline;
	struct console_option level = { NULL, NULL };

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

	printf("After removing loglevel:\n");
	level = find_and_remove_console_option("loglevel", cmdline);
	con.options = cmdline;
	printf("options passed to driver: %s\n", con.options);
	printf("command line options: %s\n", cmdline);
	if (level.key)
		printf("level: key: %s, value: %s\n", level.key, level.value);

	if (level.key)
		free(level.key);
	if (level.value)
		free(level.value);

	if (fp) {
		fclose(fp);
		free(cmdline);
	}
}
