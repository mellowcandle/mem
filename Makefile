CFLAGS := -Wall -std=c99

memtool: memtool.o dump.o
	$(CC) -o memtool memtool.o dump.o

clean:
	rm -rf memtool *.o
