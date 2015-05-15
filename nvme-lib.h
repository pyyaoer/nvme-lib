#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <linux/aio_abi.h>
#include <scsi/scsi.h>
#include "nvme.h"
#include "sg.h"

#define NVME_IOV_WRITE 0X01
#define NVME_IOV_READ 0X02
#define NVME_IOV_TRIM 0x03

typedef struct nvme_iovec {
	uint64_t iov_base;
	uint64_t iov_len;
	uint64_t iov_lba;
	uint32_t iov_opcode;
} nvme_iovec_t;

typedef struct arg_struct {
	int cpu;
	int fd;
	int nsid;
	int iovcnt;
	nvme_iovec_t* iovec;
} arg_struct_t;

//just for test
typedef struct arg_write_struct {
	int cpu;
	int fd;
	char* base;
	uint64_t len;
	uint64_t start_pos;
} arg_write_struct_t;
void* lib_nvme_write_single_sync(void* args);
int lib_nvme_batch_sync(int* fd, uint64_t* base, uint64_t* len, uint64_t* start_pos, int thread_num);
//test end

int lib_nvme_write_iosubmit(int fd, char* base, uint64_t len, uint64_t start_pos, int thread_num);

int lib_nvme_read_iosubmit(int fd, char* base, uint64_t len, uint64_t start_pos, int thread_num);

int lib_nvme_write(int fd, int nsid, uint64_t base, uint64_t len, uint64_t start_lba);

int lib_nvme_read(int fd, int nsid, uint64_t base, uint64_t len, uint64_t start_lba);

int lib_nvme_write_ioctl(int fd, int nsid, uint64_t base, uint64_t len, uint64_t start_lba);

int lib_nvme_read_ioctl(int fd, int nsid, uint64_t base, uint64_t len, uint64_t start_lba);

int lib_nvme_batch_cmd(int fd, int nsid, nvme_iovec_t *iov, uint32_t iovcnt);

int lib_nvme_flush(int fd, int nsid);

int lib_nvme_read_scsi(int fd, uint64_t base, uint64_t len, uint64_t start_lba);

int lib_nvme_write_scsi(int fd, uint64_t base, uint64_t len, uint64_t start_lba);

int lib_nvme_unmap_scsi(int fd, unsigned len, uint64_t start_lba);

int lib_nvme_batch_scsi(int fd, nvme_iovec_t *iov, uint32_t iovcnt);

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
