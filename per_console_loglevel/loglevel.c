#define _DEFAULT_SOURCE

#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

#define clamp(a, b, c) (a)
#define CON_LOGLEVEL 128
#define pr_warn printf
#define CONSOLE_LOGLEVEL_MIN 0
#define CONSOLE_MOTORMOUTH 15

struct console_cmdline {
	char *options;
	int level;
	short flags;
};

ptrdiff_t strscpy(char dest[restrict /*size*/],
		  const char src[restrict /*size*/], ptrdiff_t size)
{
	ptrdiff_t len;

	if (size <= 0)
		return -E2BIG;

	len = strnlen(src, size - 1);
	memcpy(dest, src, len);
	dest[len] = '\0';

	return len;
}

/**
  * find_and_remove_console_option() - find and remove particular option from
  *     console= parameter
  *
  * @options: pointer to the buffer with original "options"
  * @key: option name to be found
  * @buf: buffer for the found value, might be NULL
  *
  * Try to find "key[:value]" in the given @options string. Copy the value
  * into the given @buf when any.
  *
  * Warning: The function is designed only to handle new generic console
  *     options. The found "key[:value]" is removed from the given
  *     @options string. The aim is to pass only the driver specific
  *     options to the legacy driver code.
  *
  *     The new options might stay when all console drivers are able
  *     to handle it.
  *
  *     The given @options buffer is modified to avoid allocations. It
  *     makes the life easier during early boot.
  */
static bool find_and_remove_console_option(char *options, const char *key,
					   char *buf, int size)
{
	bool found = false, copy_failed = false, trailing_comma = false;
	char *opt, *val;

	/* Find the option: key[:value] */
	while (options && *options) {
		opt = options;

		options = strchr(options, ',');
		if (options)
			*options++ = '\0';

		val = strchr(opt, ':');
		if (val)
			*val++ = '\0';

		if (strcmp(opt, key) == 0) {
			found = true;
			break;
		}

		/* Not our key. Keep it in options. */
		if (options) {
			*(options - 1) = ',';
			trailing_comma = 1;
		}
		if (val)
			*(val - 1) = ':';
	}

	/* Copy the value if found. */
	if (found && val) {
		if (buf && size > strlen(val)) {
			strscpy(buf, val, size);
		} else {
			pr_warn("Can't copy console= option value. Ignoring %s:%s\n",
				opt, val);
			copy_failed = true;
		}
	}

	/* Remove the found option. */
	if (found) {
		if (options) {
			strcpy(opt, options);
			options = opt;
		} else if (trailing_comma) {
			/* Removing last option */
			*(opt - 1) = '\0';
		} else
			*(opt) = '\0';
	}

	return found && !copy_failed;
}

void handle_per_console_loglevel_option(struct console_cmdline *c,
					char *options)
{
	char buf[10];
	int loglevel, ret;

	if (!find_and_remove_console_option(options, "loglevel", buf,
					    sizeof(buf)))
		return;

	loglevel = buf[0] - '0';

	if (loglevel < CONSOLE_LOGLEVEL_MIN || loglevel > CONSOLE_MOTORMOUTH) {
		pr_warn("Console loglevel out of range: %d\n", loglevel);
		return;
	}

	c->level = loglevel;
	c->flags |= CON_LOGLEVEL;
};

int main(int argc, char *argv[])
{
	struct console_cmdline con = { 0 };
	FILE *fp = NULL;
	long size;
	char *buffer;

	if (argc == 2)
		buffer = argv[1];
	else {
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
	}

	handle_per_console_loglevel_option(&con, buffer);
	printf("options: %s\n", con.options);
	printf("level: %d\n", con.level);

	if (fp) {
		fclose(fp);
		free(buffer);
	}
}
