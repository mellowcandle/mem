#include "memtool.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
int base_scanf(const char *buf, int base, off_t *value)
{
	int ret = 0;

	switch (base) {
	case 10:
		ret = sscanf(buf, "%" PRIu64, value);
		break;
	case 16:
		ret = sscanf(buf, "%" PRIx64, value);
		break;
	case 8:
		ret = sscanf(buf, "%" PRIo64, value);
		break;
	default:
		fprintf(stderr, "Unknown base\n");
		break;
	}

	if (ret == EOF || !ret) {
		fprintf(stderr, "Couldn't parse number: %s\n", buf);
		return 1;
	}

	return 0;
}

int parse_input(const char *input, off_t *val)
{
	int base;

	if (input[0] == '0')
		if (input[1] == 'x' || input[1] == 'X')
			base = 16;
		else
			base = 8;
	else
		base = 10;

	return base_scanf(input, base, val);
}

static int do_help(int argc, char **argv)
{
	return 0;
}

static int do_compare(int argc, char **argv)
{
	return 0;
}
static int do_copy(int argc, char **argv)
{
	return 0;
}

static int do_set(int argc, char **argv)
{
	return 0;
}

static int do_checksum(int argc, char **argv)
{
	return 0;
}

static const struct cmd {
	const char *cmd;
	int (*func)(int argc, char **argv); 
	} cmds[] = {
		{"dump", do_dump},
		{"load", do_load},
		{"store", do_store},
		{"compare", do_compare},
		{"copy", do_copy},
		{"set", do_set},
		{"checksum", do_checksum},
		{"help", do_help}, {0}};

static int do_cmd(int argc, char **argv)
{
	const struct cmd *c;

	for (c = cmds; c->cmd; ++c) {
		if (strncmp(argv[0], c->cmd, strlen(c->cmd)) == 0)
			return (c->func(argc, argv));
	}

	fprintf(stderr, "Subcommand \"%s\" is unknown, try \"memtool help\".\n", argv[0]);
	return EXIT_FAILURE;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		do_help(argc, argv);
		return EXIT_FAILURE;
	}

	return do_cmd(argc - 1, argv + 1);
}
