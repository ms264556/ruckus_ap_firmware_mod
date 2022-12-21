CC=gcc
CFLAGS=-Wall -g
TARGET=all

INSTALL_DIR = ../bin

OBJS = ruckus-fw.o md5.o

all: dump-header shred-package build-package
	$(MAKE) -C ./squashfs4.0-ruckus-lzma/

dump-header: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) dump-header.c $(OBJS) -o dump-header

shred-package: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) shred-package.c $(OBJS) -o shred-package

build-package: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) build-package.c $(OBJS) -o build-package

ruckus-fw.o: md5.o
	$(CC) $(CFLAGS) $(LDFLAGS) -c ruckus-fw.c

md5.o:
	$(CC) $(CFLAGS) $(LDFLAGS) -c md5.c

install: dump-header shred-package build-package
	mkdir -p $(INSTALL_DIR)
	install dump-header $(INSTALL_DIR)
	install shred-package $(INSTALL_DIR)
	install build-package $(INSTALL_DIR)
	$(MAKE) -C ./squashfs4.0-ruckus-lzma install

clean:
	rm -f *.o dump-header shred-package build-package
	$(MAKE) -C ./squashfs4.0-ruckus-lzma clean
