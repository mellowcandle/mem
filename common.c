#include <ctype.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mem.h"

int map_memory(char *memdev, off_t size, int props, off_t target, struct mapped_mem *mem)
{
	unsigned page_size, mapped_size, offset_in_page;
	int page_count;
	int fd;
	page_size = sysconf(_SC_PAGESIZE);

	fd = open(memdev, O_RDONLY | O_SYNC);
	if (!fd) {
		perror("Can't open memory device");
		return -1;
	}

	page_count = size / page_size;
	if (size % page_size)
		page_count++;

	//	printf("Mapping %d pages\n",page_count);
	mapped_size = page_count * page_size;
	offset_in_page = (unsigned)target & (page_size - 1);
	if (offset_in_page + size > page_size) {
		/* This access spans pages.
		 * Must map two pages to make it possible: */
		mapped_size += page_size;
	}
	//	printf("Mapped size: %d\n", mapped_size);

	mem->base = mmap(NULL, mapped_size, props, MAP_SHARED, fd, target & ~(off_t)(page_size - 1));
	if (mem->base == MAP_FAILED) {
		perror("Failed to map memory device to memory");
		exit(EXIT_FAILURE);
	}

	mem->mapped_size = mapped_size;
	mem->v_ptr = (char *)mem->base + offset_in_page;

	close(fd);

	return EXIT_SUCCESS;
}

void unmap_memory(struct mapped_mem *mem)
{
	if (!mem)
		return;
	if (munmap(mem->base, mem->mapped_size) == -1) {
		perror("Can't unmap memory");
	}
}
