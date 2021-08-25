CFLAGS := -Wall -std=c99

memtool: memtool.o dump.o store.o load.o
	$(CC) -o memtool memtool.o dump.o store.o load.o

clean:
	rm -rf memtool *.o
