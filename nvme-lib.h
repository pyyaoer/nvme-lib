#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include "nvme.h"


int lib_nvme_write(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba);

int lib_nvme_read(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba);

int lib_nvme_flush(int fd, int nsid);

int lib_nvme_unmap(int fd, int nsid, uint64_t start_lba, unsigned nlb);
