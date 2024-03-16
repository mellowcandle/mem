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

static void do_devmem_help(FILE *output)
{
	fprintf(output, "Usage:\nmem devmem [options] <address> [type [data]]\n\n");
	fprintf(output, "devmem memory content in output file.\n");
	fprintf(output, "Options:\n");
	fprintf(output, " -m, --mem-dev\t\t memory device to use (default is /dev/mem)\n");
	fprintf(output, " -r, --read-back\t\t Read back data after write\n");
	fprintf(output, " -f, --force-strict-alignment\t\t If address is not aligned, go back until it aligned (default behaviour in devmem2)\n");
	fprintf(output, " -v, --verbose\t\t Output addresses and written values\n");
	fprintf(output, " -h, --help\t\t Display this help screen\n");
	fprintf(output, "Arguments:\n");
	fprintf(output, " <address> can be given in decimal, hexedecimal or octal format\n");
	fprintf(output, " [type] access operation type: [b]yte, [h]alfword, [w]ord, [l]ong\n");
	fprintf(output, " [data] data to be written\n\n");
	fprintf(output, "Note: base is detect according to the prefix (no-prefix, 0x, and 0).\n");
}

#define READ_OP 0
#define WRITE_OP 1

static inline off_t apply_alignment(off_t addr, size_t size)
{
	return addr & ~(size - 1);
}

int do_devmem(int argc, char **argv)
{
	int c;
	off_t target;
	off_t size = sizeof(uint32_t);
	char *memdev = "/dev/mem";
	struct mapped_mem mem;
	bool read_back = false;
	char access_type;
	bool force_align = false;
	int op = READ_OP;
	uint64_t write_val;
	uint64_t read_val;
	bool verbose = false;
	while (1) {
		// clang-format off
		static struct option long_options[] = {
			{"mem-dev", required_argument, 0, 'm'},
			{"read-back", no_argument, 0, 'r'},
			{"force-strict-alignment", no_argument, 0, 'f'},
			{"verbose", no_argument, 0, 'v'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};
		// clang-format on
		int option_index = 0;

		c = getopt_long(argc, argv, "mrf:vh", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
		case 'm':
			memdev = optarg;
			break;
		case 'h':
			do_devmem_help(stdout);
			return EXIT_SUCCESS;
		case 'r':
			read_back = true;
			break;
		case 'f':
			force_align = true;
			break;
		case 'v':
			verbose = true;
			break;
		case '?':
			/* getopt_long already printed an error message. */
			return EXIT_FAILURE;
		default:
			fprintf(stderr, "Unsupported option\n");
			do_devmem_help(stderr);
			return EXIT_FAILURE;
			break;
		}
	};

	if ((argc - optind < 1) || (argc - optind > 3)) {
		fprintf(stderr, "devmem: Unsupported arguments\n");
		do_devmem_help(stderr);
		return EXIT_FAILURE;
	}

	if (parse_input(argv[optind], &target)) {
		do_devmem_help(stderr);
		exit(EXIT_FAILURE);
	}

	if (argc - optind > 1) {
		access_type = tolower(*argv[optind + 1]);
		switch (access_type) {
		case 'b':
			size = sizeof(uint8_t);
			break;
		case 'h':
			size = sizeof(uint16_t);
			break;
		case 'w':
			size = sizeof(uint32_t);
			break;
		case 'l':
			size = sizeof(uint64_t);
			break;
		default:
			fprintf(stderr, "devmem: Unsupported data type %c.\n", access_type);
			exit(EXIT_FAILURE);
		}
	}

	if (argc - optind == 3)
		op = WRITE_OP;

	if (force_align)
		target = apply_alignment(target, size);

	if (map_memory(memdev, size, (op == WRITE_OP) ? PROT_WRITE : PROT_READ, target, &mem))
		exit(EXIT_FAILURE);

	if (op == WRITE_OP) {
		write_val = strtoul(argv[optind + 2], 0, 0);

		switch (access_type) {
		case 'b':
			write_val &= 0xff;
			*(uint8_t *)mem.v_ptr = write_val;
			break;
		case 'h':
			write_val &= 0xffff;
			*(uint16_t *)mem.v_ptr = write_val;
			break;
		case 'w':
			write_val &= 0xffffffff;
			*(uint32_t *)mem.v_ptr = write_val;
			break;
		case 'l':
			*(uint64_t *)mem.v_ptr = write_val;
			break;
		default:
			fprintf(stderr, "devmem: Unsupported data type %c.\n", access_type);
			exit(EXIT_FAILURE);
		}
		if (verbose)
				printf("Write at address 0x%8lx (%p): 0x%8lx\n", target, mem.v_ptr, write_val);
	}

	if ((op == READ_OP) || read_back) {
		switch (access_type) {
		case 'b':
			read_val = *(uint8_t *)mem.v_ptr;
			read_val &= 0xff;
			break;
		case 'h':
			read_val = *(uint16_t *)mem.v_ptr;
			read_val &= 0xffff;
			break;
		case 'w':
			read_val = *(uint32_t *)mem.v_ptr;
			read_val &= 0xffffffff;
			break;
		case 'l':
			read_val = *(uint64_t *)mem.v_ptr;
			break;
		default:
			fprintf(stderr, "devmem: Unsupported data type %c.\n", access_type);
			exit(EXIT_FAILURE);
		}

		printf("Read at address 0x%8lx (%p): 0x%8lx\n", target, mem.v_ptr, read_val);

	}



	unmap_memory(&mem);

	return EXIT_SUCCESS;
}
