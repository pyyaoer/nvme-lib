#define _GNU_SOURCE

#include <stdlib.h>
#include <time.h>
#include "nvme-lib.h"

#define IOSUBMIT_BLK_SIZE (4096*4)
#define IOSUBMIT_BLK_NUM (1024*1024)
#define IOSUBMIT_THREAD_NUM 1

int main(){
	int fd;
	uint64_t i=1;
	time_t start, end;

	//Open device.
	fd = open("/dev/nvme0n1",O_RDWR);
	if (fd < 0){
		printf("block device open failed, errno = %d\n", errno);
		return -1;
	}
	printf("Open success, fd = %d\n", fd);

	//Read/Write buffer.
	char* write_buffer = malloc(IOSUBMIT_BLK_SIZE);
	char* read_buffer = malloc(IOSUBMIT_BLK_SIZE);
	for(i = 0; i < sizeof(write_buffer); i ++)
		write_buffer[i] = 'b';

	//Write performance.
	start = time(NULL);
	for (i = 0; i < IOSUBMIT_BLK_NUM; ++i){
		lib_nvme_write_iosubmit(fd, write_buffer, IOSUBMIT_BLK_SIZE, IOSUBMIT_BLK_SIZE*i);
	}
	fsync(fd);
	end = time(NULL);
	printf("Write time:%d\n", end-start);

	//Read performance.
	start = time(NULL);
	for (i = 0; i < IOSUBMIT_BLK_NUM; ++i){
		lib_nvme_read_iosubmit(fd, read_buffer, IOSUBMIT_BLK_SIZE, IOSUBMIT_BLK_SIZE*i);
	}
	end = time(NULL);
	printf("Read time:%d\n", end-start);

	close(fd);
}
