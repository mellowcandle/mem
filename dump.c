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

static void do_dump_help(FILE *output)
{
	fprintf(output, "Usage:\nmem dump [options] <address> <length>\n\n");
	fprintf(output, "Display memory content in hexadecimal format.\n");
	fprintf(output, "Options:\n");
	fprintf(output, " -m, --mem-dev\t\t memory device to use (default is /dev/mem)\n");
	fprintf(output, " -C, --canonical\t canonical hex+ASCII display\n");
	fprintf(output, " -a, --ascii\t\t ASCII display\n");
	fprintf(output, " -v, --no-squeezing\t output identical lines\n\n");
	fprintf(output, " -h, --help\t\t Display this help screen\n");
	fprintf(output, "Arguments:\n");
	fprintf(output, " <address> and <length> can be given in decimal, hexedecimal or octal format\n");
	fprintf(output, " depending of the prefix (no-prefix, 0x, and 0).\n");
}

int do_dump(int argc, char **argv)
{
	int c;
	int canonical = 0;
	void *virt_addr;
	int ascii = 0;
	int squeeze = 1;
	off_t target;
	off_t size;
	bool first = true;
	bool in_squeeze = false;
	char *memdev = "/dev/mem";
	struct mapped_mem mem;
	while (1) {
		// clang-format off
		static struct option long_options[] = {
			{"mem-dev", required_argument, 0, 'm'},
		    {"canonical", no_argument, 0, 'C'},
		    {"no-squeezing", no_argument, 0, 'v'},
		    {"ascii", no_argument, 0, 'a'},
		    {"help", no_argument, 0, 'h'},
		    {0, 0, 0, 0}
		};
		// clang-format on
		int option_index = 0;

		c = getopt_long(argc, argv, "m:Cvha", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
		case 'm':
			memdev = optarg;
			break;
		case 'C':
			canonical = 1;
			break;
		case 'a':
			ascii = 1;
			break;
		case 'v':
			squeeze = 0;
			break;
		case 'h':
			do_dump_help(stdout);
			return EXIT_SUCCESS;
		case '?':
			/* getopt_long already printed an error message. */
			return EXIT_FAILURE;
		default:
			fprintf(stderr, "Unsupported option\n");
			do_dump_help(stderr);
			return EXIT_FAILURE;
			break;
		}
	};
	if (argc - optind != 2) {
		fprintf(stderr, "Missing address or length\n");
		do_dump_help(stderr);
		return EXIT_FAILURE;
	}

	if (parse_input(argv[optind], &target)) {
		do_dump_help(stderr);
		exit(EXIT_FAILURE);
	}

	if (parse_input(argv[optind + 1], &size)) {
		do_dump_help(stderr);
		exit(EXIT_FAILURE);
	}

	if (canonical && ascii) {
		fprintf(stderr, "Ascii & Canonical are mutual exclusive options\n");
		return EXIT_FAILURE;
	}
	/* Get address */

	if (map_memory(memdev, size, PROT_READ, target, &mem))
		exit(EXIT_FAILURE);

	virt_addr = mem.v_ptr;

	// Do the magic here
	while (size > 0) {
		int i;

		if (!ascii) {
				if (squeeze && (!first) && ((size - 16) >= 16)) {
					if (memcmp(virt_addr, virt_addr + 16, 16) == 0) {
						if (!in_squeeze) {
							printf("*\n");
							in_squeeze = true;
						}
						goto proceed;
					} else {
						in_squeeze = false;
					}
				}

				if (target >= ULONG_MAX)
					printf("0x%.16" PRIx64 "  ", target);
				else
					printf("0x%.8" PRIx64 "  ", target);
				for (i = 0; i < 16; i++) {
					if (i == 8)
						putchar(' ');
					printf("%02x ", *(volatile uint8_t *)(virt_addr + i));
				}
				if (canonical) {
					printf(" |");
					for (i = 0; i < 16 && (size - i) > 0; i++) {
						char c = *(volatile uint8_t *)(virt_addr + i);
						printf("%c", isgraph(c) ? c : '.');
					}
					printf("|");
				}
		} else {
				for (i = 0; i < size; i++) {
					char c = *(volatile uint8_t *)(virt_addr + i);
					if (c == '\0')
							break;
					if (isascii(c))
							putchar(c);
					else
							putchar('.');
				}
				putchar('\n');
				goto exit1;
		}
		putchar('\n');
proceed:
		size -= 16;
		virt_addr += 16;
		target += 16;
		first = false;
	}
exit1:

	unmap_memory(&mem);

	return EXIT_SUCCESS;
}
