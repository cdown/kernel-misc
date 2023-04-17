/*
 * afl-gcc loglevel.c -fno-omit-frame-pointer -Og -ggdb -o loglevel
 * afl-fuzz -i fuzz-in -o fuzz-out -f afl ./loglevel afl
 */
#define _DEFAULT_SOURCE

#include <assert.h>
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
#define pr_warn printf

static bool find_and_remove_console_option(char *buf, size_t size,
					   const char *wanted, char *options)
{
	bool found = false, first = true;
	char *item, *opt = options;

	while ((item = strsep(&opt, ","))) {
		char *key = item, *value;

		value = strchr(item, ':');
		if (value)
			*(value++) = '\0';

		if (strcmp(key, wanted) == 0) {
			found = true;
			if (value) {
				if (strlen(value) > size - 1) {
					pr_warn("Can't copy console option value for %s:%s: not enough space (%zu)\n",
						key, value, size);
					found = false;
				} else {
					strscpy(buf, value, size);
				}
			} else
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
			memcpy(item, opt, strlen(opt) + 1);
		else if (first)
			*item = '\0';
		else
			*--item = '\0';
	}

	return found;
}

void check_result(char *cmdline_in, char *key, bool should_find,
		  char *expected_val, char *expected_cmdline)
{
	char val[16];
	char cmdline[128];
	bool found;

	strcpy(cmdline, cmdline_in);
	printf("Looking for key %s in cmdline '%s'\n", key, cmdline);
	found = find_and_remove_console_option(val, sizeof(val), key, cmdline);
	printf("Found: %d\n", found);
	assert(found == should_find);
	if (found) {
		printf("Value: %s\n", val);
		assert(strcmp(val, expected_val) == 0);
	}
	printf("Mutated cmdline: %s\n", cmdline);
	assert(strcmp(cmdline, expected_cmdline) == 0);
}

void print_result(char *cmdline, char *key)
{
	char val[16];
	bool found;

	printf("Looking for key %s in cmdline '%s'\n", key, cmdline);
	found = find_and_remove_console_option(val, sizeof(val), key, cmdline);
	printf("Found: %d\n", found);
	if (found) {
		printf("Value: %s\n", val);
	}
	printf("Mutated cmdline: %s\n", cmdline);
}

int main(int argc, char *argv[])
{
	FILE *fp = NULL;
	long size;
	char *cmdline;

	if (argc == 2) {
		if (strcmp(argv[1], "afl") == 0) {
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
		} else {
			cmdline = argv[1];
		}
		print_result(cmdline, "loglevel");
	} else {
		check_result("", "foo", false, "", "");
		check_result("bar", "foo", false, "", "bar");
		check_result("bar,baz", "foo", false, "", "bar,baz");
		check_result("foo", "foo", true, "", "");
		check_result("foo:bar", "foo", true, "bar", "");
		check_result("bar,foo:bar,baz", "foo", true, "bar", "bar,baz");
		// val is too large
		check_result(
			"bar,foo:barrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr,baz",
			"foo", false, "",
			"bar,foo:barrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr,baz");
	}

	if (fp) {
		fclose(fp);
		free(cmdline);
	}
}
