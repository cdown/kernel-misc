#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

struct console {
	char *options;
	int level;
};

static void _parse_named_options(struct console *con, const char *key,
				 const char *value)
{
	if (strcmp(key, "loglevel") == 0) {
		if (value && isdigit(value[0]) && strlen(value) == 1) {
			/* TODO: clamping + flags */
			con->level = value[0] - '0';
			return;
		}
	}

	printf("ignoring invalid console option: '%s%s%s'\n", key,
	       value ? ":" : "", value ?: "");
}

static int parse_named_options(struct console *con, char *options)
{
	char *item;

	while ((item = strsep(&options, ",")) != NULL) {
		char *key = item, *value;

		value = strchr(item, ':');
		if (value)
			*(value++) = '\0';

		_parse_named_options(con, key, value);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct console con = { 0 };
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

	parse_named_options(&con, buffer);
	printf("%d\n", con.level);

	fclose(fp);
	free(buffer);
}
