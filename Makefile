CFLAGS := -Wall -std=c99

memtool: memtool.o dump.o store.o
	$(CC) -o memtool memtool.o dump.o store.o

clean:
	rm -rf memtool *.o
