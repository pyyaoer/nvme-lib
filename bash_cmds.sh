#!/bin/sh

gcc -fPIC -g -c nvme-lib.c 
gcc -shared -o libnvme.so nvme-lib.o

sudo mv libnvme.so /usr/lib

LD_LIBRARY_PATH=.

gcc test.c -L. -lnvme

