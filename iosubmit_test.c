#define _GNU_SOURCE

#include <stdlib.h>
#include <time.h>
#include "nvme-lib.h"

uint64_t block_size = 4096*1024*4;
uint64_t block_num = 1024;
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
	char* write_buffer = malloc(block_size);
	char* read_buffer = malloc(block_size);
	for(i = 0; i < sizeof(write_buffer); i ++)
		write_buffer[i] = 'b';
/*
	//Write performance.
	start = time(NULL);
	for (i = 0; i < block_num; ++i){
		lib_nvme_write_iosubmit(fd, write_buffer, block_size, block_size*i, IOSUBMIT_THREAD_NUM);
	}
	fsync(fd);
	end = time(NULL);
	printf("Write time:%d\n", end-start);

	for(block_num = 65536*2; block_num <= 1024*1024*4; block_num *= 2){
		block_size=(4096*1024*4)/block_num;
		block_size*=1024;
		//Write performance.
		start = time(NULL);
		for (i = 0; i < block_num; ++i){
			lib_nvme_write_iosubmit_single(fd, write_buffer, block_size, block_size*i, IOSUBMIT_THREAD_NUM);
		}
		fsync(fd);
		end = time(NULL);
		printf("%ld Write time:%d\n", block_size, end-start);

		//Read performance.
		start = time(NULL);
		for (i = 0; i < block_num; ++i){
			lib_nvme_read_iosubmit(fd, read_buffer, block_size, block_size*i, IOSUBMIT_THREAD_NUM);
		}
		end = time(NULL);
		printf("%ld Read time:%d\n", block_size, end-start);
	}
*/
	for (i = 0; i < 1024*1024; ++i){
		test_range[i] = rand();
		uint64_t high = rand();
		test_range[i] |= (high << 32);
	}
	printf("%d\n", RAND_MAX);
	close(fd);
}
