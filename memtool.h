#ifndef MEMTOOL_H
#define MEMTOOL_H

int do_dump(int argc, char **argv);
int do_load(int argc, char **argv);
int do_store(int argc, char **argv);
int parse_input(const char *input, off_t *val);

#define TRACE() fprintf(stderr, "%s:%u\n", __FILE__, __LINE__)
#endif
