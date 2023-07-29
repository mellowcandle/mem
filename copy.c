#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mem.h"

static void do_copy_help(FILE *output)
{
	fprintf(output, "Usage:\nmem copy [options] <source address> <target address> <size>\n\n");
	fprintf(output, "copy <size> bytes from <source address> to <target address>.\n");
	fprintf(output, "Options:\n");
	fprintf(output, " -m, --mem-dev\t\t memory device to use (default is /dev/mem)\n");
	fprintf(output, " -h, --help\t\t Display this help screen\n");
	fprintf(output, "Arguments:\n");
	fprintf(output, " <source address> can be given in decimal, hexedecimal or octal format\n");
	fprintf(output, " <target address> can be given in decimal, hexedecimal or octal format\n");
	fprintf(output, " <size> can be given in decimal, hexedecimal or octal format\n");
	fprintf(output, "Note: base is detect according to the prefix (no-prefix, 0x, and 0).\n");
}

int do_copy(int argc, char **argv)
{
	int c;
	off_t source;
	off_t target;
	off_t size;
	int rc;
	char *memdev = "/dev/mem";
	struct mapped_mem src_mem;
	struct mapped_mem dst_mem;

	while (1) {
		// clang-format off
		static struct option long_options[] = {
			{"mem-dev", required_argument, 0, 'm'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};
		// clang-format on
		int option_index = 0;

		c = getopt_long(argc, argv, "m:h", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
		case 'm':
			memdev = optarg;
			break;
		case 'h':
			do_copy_help(stdout);
			return EXIT_SUCCESS;
		case '?':
			/* getopt_long already printed an error message. */
			return EXIT_FAILURE;
		default:
			fprintf(stderr, "Unsupported option\n");
			do_copy_help(stderr);
			return EXIT_FAILURE;
			break;
		}
	};

	if (argc - optind != 3) {
		fprintf(stderr, "Missing address or size\n");
		do_copy_help(stderr);
		return EXIT_FAILURE;
	}

	if (parse_input(argv[optind], &source)) {
		do_copy_help(stderr);
		exit(EXIT_FAILURE);
	}

	if (parse_input(argv[optind + 1], &target)) {
		do_copy_help(stderr);
		exit(EXIT_FAILURE);
	}

	if (parse_input(argv[optind + 2], &size)) {
		do_copy_help(stderr);
		exit(EXIT_FAILURE);
	}


	if (map_memory(memdev, size, PROT_READ, source, &src_mem))
		exit(EXIT_FAILURE);

	if (map_memory(memdev, size, PROT_WRITE, target, &dst_mem))
		exit(EXIT_FAILURE);

	memcpy(dst_mem.v_ptr, src_mem.v_ptr, size);

	unmap_memory(&src_mem);
	unmap_memory(&dst_mem);

	return EXIT_SUCCESS;
}


