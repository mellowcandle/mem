#ifndef MEMTOOL_H
#define MEMTOOL_H

#include <sys/types.h>

struct mapped_mem {
	char *v_ptr;
	void *base;
	off_t mapped_size;
};

int do_dump(int argc, char **argv);
int do_copy(int argc, char **argv);
int do_compare(int argc, char **argv);
int do_load(int argc, char **argv);
int do_store(int argc, char **argv);
int do_devmem(int argc, char **argv);
int parse_input(const char *input, off_t *val);

int map_memory(char *memdev, off_t size, int props, off_t target, struct mapped_mem *mem);
void unmap_memory(struct mapped_mem *mem);

#define TRACE() fprintf(stderr, "%s:%u\n", __FILE__, __LINE__)
#endif
