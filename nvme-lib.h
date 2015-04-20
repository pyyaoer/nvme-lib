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
#include "sg.h"


int lib_nvme_write(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba);

int lib_nvme_read(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba);

int lib_nvme_write_core(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba);

int lib_nvme_read_core(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba);

int lib_nvme_flush(int fd, int nsid);

int lib_nvme_unmap(int fd, int nsid, uint64_t start_lba, unsigned nlb);

int lib_nvme_features(int fd, int set_get, int feature, int cdw11, int* res);

int lib_nvme_set_awun(int fd, int block_awun);

int lib_nvme_get_awun(int fd, int block_awun);

int lib_nvme_identify(int fd, int nsid, void *ptr, int cns);

int lib_nvme_nsze(int fd, int nsid);

int lib_nvme_lba_size(int fd, int nsid);

int lib_nvme_metadata_size(int fd, int nsid);

int lib_nvme_awun_size(int fd, int nsid);

int lib_nvme_awupf_size(int fd, int nsid);

int lib_nvme_mdts(int fd, int nsid);
