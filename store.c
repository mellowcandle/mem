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
#include <unistd.h>

#include "mem.h"

static void do_store_help(FILE *output)
{
	fprintf(output, "Usage:\nmem store [options] <address> <length> <output_file>\n\n");
	fprintf(output, "Store memory content in output file.\n");
	fprintf(output, "Options:\n");
	fprintf(output, " -m, --mem-dev\t\t memory device to use (default is /dev/mem)\n");
	fprintf(output, " -h, --help\t\t Display this help screen\n");
	fprintf(output, "Arguments:\n");
	fprintf(output, " <address> and <length> can be given in decimal, hexedecimal or octal format\n");
	fprintf(output, " depending of the prefix (no-prefix, 0x, and 0).\n");
}

int do_store(int argc, char **argv)
{
	int c;
	off_t target;
	off_t size;
	int out_fd;
	char *memdev = "/dev/mem";
	struct mapped_mem mem;

	while (1) {
		// clang-format off
		static struct option long_options[] = {
		    {"mem-dev", required_argument, 0, 'm'},
			{"help", no_argument, 0, 'h'},
		    {0, 0, 0, 0}
		    // clang-format on
		};

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
			do_store_help(stdout);
			return EXIT_SUCCESS;
		case '?':
			/* getopt_long already printed an error message. */
			return EXIT_FAILURE;
		default:
			fprintf(stderr, "Unsupported option\n");
			do_store_help(stderr);
			return EXIT_FAILURE;
			break;
		}
	};

	if (argc - optind != 3) {
		fprintf(stderr, "Missing address or length\n");
		do_store_help(stderr);
		return EXIT_FAILURE;
	}

	if (parse_input(argv[optind], &target)) {
		do_store_help(stderr);
		exit(EXIT_FAILURE);
	}

	if (parse_input(argv[optind + 1], &size)) {
		do_store_help(stderr);
		exit(EXIT_FAILURE);
	}

	out_fd = open(argv[optind + 2], O_WRONLY | O_CREAT, 0644);
	if (out_fd == -1) {
		perror("Can't open file for output");
		exit(EXIT_FAILURE);
	}

	if (map_memory(memdev, size, PROT_READ, target, &mem))
		exit(EXIT_FAILURE);


	if (write(out_fd, mem.v_ptr, size) != size) {
		perror("Failed writing memory content to file");
		return EXIT_FAILURE;
	}

	unmap_memory(&mem);
	close(out_fd);

	return EXIT_SUCCESS;
}
