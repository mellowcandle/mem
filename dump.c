#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ctype.h>
#include <inttypes.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#include "memtool.h"
static void do_dump_help(FILE *output)
{
	printf("Damn\n");
}

int do_dump(int argc, char **argv)
{
	int c;
	unsigned page_size, mapped_size, offset_in_page;
	void *map_base, *virt_addr;
	int fd;
	int page_count;
	int canonical = 0;
	int squeeze = 1;
	off_t target;
	off_t size;
	bool first = true;
	bool in_squeeze = false;

/*
 -C, --canonical           canonical hex+ASCII display
 -v, --no-squeezing        output identical lines
 */
	while (1) {
		static struct option long_options[] =
		{
			{"canonical",		no_argument,       0, 'C'},
			{"no-squeezing",	no_argument,	   0, 'v'},
			{0, 0, 0, 0}
		};
		int option_index = 0;

		c = getopt_long (argc, argv, "Cv",
				 long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
		case 'C':
			canonical = 1;
			break;
		case 'v':
			squeeze = 0;
			break;
		case '?':
	  /* getopt_long already printed an error message. */
			return EXIT_FAILURE;
		default:
			fprintf(stderr, "Unsupported option\n");
			return EXIT_FAILURE;
			break;
		}
	};
	if (argc - optind != 2) {
		printf("%d\n", argc-optind);
		TRACE();
		do_dump_help(stderr);
		return EXIT_FAILURE;
	}

	if (parse_input(argv[optind], &target)) {
		printf("Herer2\n");
		do_dump_help(stderr);
		exit(EXIT_FAILURE);
	}
	printf("target: 0x%" PRIX64 "\n", target);

	if (parse_input(argv[optind+1], &size)) {
		do_dump_help(stderr);
		exit(EXIT_FAILURE);
	}
	printf("size: 0x%" PRIX64 "\n", size);

	/* Get address */


	page_size = getpagesize();

	fd = open("/dev/mem", O_RDONLY | O_SYNC);
	if (!fd) {
		perror("Can't open /dev/mem");
		exit(EXIT_FAILURE);
	}

	page_count = size / page_size;
	if (size % page_size)
		page_count++;

	printf("Mapping %d pages\n",page_count);
	mapped_size = page_count * page_size;
	offset_in_page = (unsigned)target & (page_size - 1);
	if (offset_in_page + size > page_size) {
		/* This access spans pages.
		 * Must map two pages to make it possible: */
		mapped_size += getpagesize();
	}
	printf("Mapped size: %d\n", mapped_size);
	map_base = mmap(NULL,
			mapped_size,
			PROT_READ,
			MAP_SHARED,
			fd,
			target & ~(off_t)(page_size - 1));
	if (map_base == MAP_FAILED) {
		perror("Failed to map /dev/mem to memory\n");
		exit(EXIT_FAILURE);
	}

	virt_addr = (char*)map_base + offset_in_page;

	// Do the magic here
	while (size > 0) {
		int i;

		if (squeeze && (!first) && ((size - 16) >= 16))
			if (memcmp(virt_addr, virt_addr + 16, 16) == 0) {
				if (!in_squeeze) {
					printf("*\n");
					in_squeeze = true;
				}
				goto proceed;
			} else
				in_squeeze = false;

		if (target >= ULONG_MAX)
			printf("0x%.16" PRIx64 "  ", target);
		else
			printf("0x%.8" PRIx64 "  ", target);
		for (i = 0; i < 16; i ++) {
			if (i == 8)
				putchar(' ');
			printf("%02x ", *(volatile uint8_t*)(virt_addr + i));
		}
		if (canonical) {
			printf(" |");
			for (i = 0; i < 16 && (size - i) > 0; i ++) {
				char c = *(volatile uint8_t*)(virt_addr + i);
				printf("%c", isgraph(c) ? 'c' : '.');
			}
			printf("|");
		}
		putchar('\n');
proceed:
		size -= 16;
		virt_addr += 16;
		target += 16;
		first = false;
	}

	if (munmap(map_base, mapped_size) == -1) {
		perror("Can't unmap memory");
		exit(EXIT_FAILURE);
	}
	close(fd);

	return EXIT_SUCCESS;
}

