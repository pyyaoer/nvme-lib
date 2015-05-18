#define _GNU_SOURCE

#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <sys/timeb.h>
#include "nvme-lib.h"

// based on lba
#define RANDOM_BLOCK 16
#define LBA_SIZE 512

// argv[1]: thread num
int main(int argc, char* argv[]){
	int fd;
	int i, j;
	struct timeb start_t, end_t;
	uint64_t t_sec, t_ms;
	srand(time(NULL));
	fd = open("/dev/nvme0n1", O_RDWR | O_DIRECT);
	if (fd < 0){
		printf("block device open failed, errno = %d\n", errno);
		goto exit;
	}
	printf("Open success, fd = %d\n", fd);

	char data[LBA_SIZE * RANDOM_BLOCK];
	uint64_t blk_num = 1024 * 256 * 256;
	struct nvme_iovec* iovec;
	iovec = malloc(blk_num * sizeof(nvme_iovec_t));

	int nsid = ioctl(fd, NVME_IOCTL_ID);
	printf("Namespace id: %d\n", nsid);
	int nsze = lib_nvme_nsze(fd, nsid);

	srand(time(NULL));
	for (i = 0; i < blk_num; ++i){
		iovec[i].iov_base = (uint64_t)data;
		iovec[i].iov_len = RANDOM_BLOCK;
		iovec[i].iov_lba = (rand() % nsze) / RANDOM_BLOCK * RANDOM_BLOCK;
		iovec[i].iov_opcode = NVME_IOV_READ;
	}

	ftime(&start_t);
	lib_nvme_batch_scsi(fd, iovec, blk_num, atoi(argv[1]));
	ftime(&end_t);
	t_sec = end_t.time - start_t.time;
	t_ms = end_t.millitm - start_t.millitm + 1000 * t_sec;
	printf("%d Batch time:%d ms\n", atoi(argv[1]), t_ms);
	printf("IOPS:%ld\n", blk_num*1000/t_ms);

	free(iovec);
exit:
	close(fd);
	return 0;
}
