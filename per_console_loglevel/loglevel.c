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

static int parse_console_cmdline_options(struct console_cmdline *c,
					 char *options)
{
	char *item;
	bool seen_serial_opts = false;

	while ((item = strsep(&options, ",")) != NULL) {
		char *key = item, *value;

		value = strchr(item, ':');
		if (value)
			*(value++) = '\0';

		if (strcmp(key, "loglevel") == 0 && value &&
		    isdigit(value[0]) && strlen(value) == 1) {
			c->level = clamp(value[0] - '0', LOGLEVEL_EMERG,
					 LOGLEVEL_DEBUG + 1);
			c->flags |= CON_LOGLEVEL;
			continue;
		}

		if (!seen_serial_opts && isdigit(key[0]) && !value) {
			seen_serial_opts = true;
			c->options = key;
			continue;
		}

		printf("ignoring invalid console option: '%s%s%s'\n", key,
		       value ? ":" : "", value ?: "");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct console_cmdline con = { 0 };
	FILE *fp;
	long size;
	char *buffer;

	fp = fopen("afl", "rb");
	if (!fp) {
		perror("afl");
		exit(1);
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	rewind(fp);

	buffer = calloc(1, size + 1);
	if (!buffer) {
		fclose(fp);
		perror("calloc");
		exit(1);
	}

	if (fread(buffer, size, 1, fp) != 1) {
		fclose(fp);
		free(buffer);
		perror("fread");
		exit(1);
	}

	parse_console_cmdline_options(&con, buffer);
	printf("options: %s\n", con.options);
	printf("level: %d\n", con.level);

	fclose(fp);
	free(buffer);
}
