#!/bin/sh

gcc -fPIC -g -c nvme-lib.c 
gcc -shared -o libnvme.so nvme-lib.o

sudo mv libnvme.so /usr/lib

#LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH="/usr/local/lib"

gcc scsi_test.c -L. -lnvme -lpthread
