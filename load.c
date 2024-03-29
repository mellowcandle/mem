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

static void do_load_help(FILE *output)
{
	fprintf(output, "Usage:\nmem load [options] <address> <input_file>\n\n");
	fprintf(output, "load memory content in output file.\n");
	fprintf(output, "Options:\n");
	fprintf(output, " -m, --mem-dev\t\t memory device to use (default is /dev/mem)\n");
	fprintf(output, " -h, --help\t\t Display this help screen\n");
	fprintf(output, "Arguments:\n");
	fprintf(output, " <address> can be given in decimal, hexedecimal or octal format\n");
	fprintf(output, "Note: base is detect according to the prefix (no-prefix, 0x, and 0).\n");
}

int do_load(int argc, char **argv)
{
	int c;
	off_t target;
	off_t size;
	int in_fd;
	char *memdev = "/dev/mem";
	struct mapped_mem mem;

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
			do_load_help(stdout);
			return EXIT_SUCCESS;
		case '?':
			/* getopt_long already printed an error message. */
			return EXIT_FAILURE;
		default:
			fprintf(stderr, "Unsupported option\n");
			do_load_help(stderr);
			return EXIT_FAILURE;
			break;
		}
	};

	if (argc - optind != 2) {
		fprintf(stderr, "Missing address or input file\n");
		do_load_help(stderr);
		return EXIT_FAILURE;
	}

	if (parse_input(argv[optind], &target)) {
		do_load_help(stderr);
		exit(EXIT_FAILURE);
	}

	in_fd = open(argv[optind + 1], O_RDONLY);
	if (in_fd == -1) {
		perror("Can't open file for output");
		exit(EXIT_FAILURE);
	}

	struct stat buf;
	fstat(in_fd, &buf);
	size = buf.st_size;

	if (map_memory(memdev, size, PROT_WRITE, target, &mem))
		exit(EXIT_FAILURE);

	if (read(in_fd, mem.v_ptr, size) != size) {
		perror("Failed reading file content to memory");
		return EXIT_FAILURE;
	}

	unmap_memory(&mem);
	close(in_fd);

	return EXIT_SUCCESS;
}
