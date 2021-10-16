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

#include "memtool.h"
static void do_load_help(FILE *output)
{
	fprintf(output, "Usage:\nmemtool load [options] <address> <input_file>\n\n");
	fprintf(output, "load memory content in output file.\n");
	fprintf(output, "Options:\n");
	fprintf(output, " -m, --mem-dev\t\t memory device to use (default is /dev/mem)\n");
	fprintf(output, "Arguments:\n");
	fprintf(output, " <address> can be given in decimal, hexedecimal or octal format\n");
	fprintf(output, " depending of the prefix (no-prefix, 0x, and 0).\n");
}

int do_load(int argc, char **argv)
{
	int c;
	unsigned page_size, mapped_size, offset_in_page;
	void *map_base, *virt_addr;
	int fd;
	int page_count;
	off_t target;
	off_t size;
	int in_fd;
	char *memdev = "/dev/mem";

	while (1) {
		static struct option long_options[] = {
			{"mem-dev", required_argument, 0, 'm'},
			{0, 0, 0, 0}
		};
		int option_index = 0;

		c = getopt_long(argc, argv, "m:", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
		case 'm':
			memdev = optarg;
			break;

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

	page_size = getpagesize();

	fd = open(memdev, O_RDWR | O_SYNC);
	if (fd == -1) {
		perror("Can't open memory device");
		exit(EXIT_FAILURE);
	}

	page_count = size / page_size;
	if (size % page_size)
		page_count++;

	mapped_size = page_count * page_size;
	offset_in_page = (unsigned)target & (page_size - 1);
	if (offset_in_page + size > page_size) {
		/* This access spans pages.
		 * Must map two pages to make it possible: */
		mapped_size += getpagesize();
	}
	map_base = mmap(NULL, mapped_size, PROT_WRITE, MAP_SHARED, fd, target & ~(off_t)(page_size - 1));
	if (map_base == MAP_FAILED) {
		perror("Failed to map memory edvice to memory\n");
		exit(EXIT_FAILURE);
	}

	virt_addr = (char *)map_base + offset_in_page;

	if (read(in_fd, virt_addr, size) != size) {
		perror("Failed reading file content to memory");
		return EXIT_FAILURE;
	}

	if (munmap(map_base, mapped_size) == -1) {
		perror("Can't unmap memory");
		exit(EXIT_FAILURE);
	}
	close(fd);
	close(in_fd);

	return EXIT_SUCCESS;
}
