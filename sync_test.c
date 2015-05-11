#define _GNU_SOURCE

#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <sys/timeb.h>
#include "nvme-lib.h"

//#define IOVN 8
#define BUFSZ (4096 * 4096 * 64)

int main(int argc, char* argv[]){
	int IOVN = atoi(argv[1]);
	int fd[128];
	uint64_t base[128];
	uint64_t len[128];
	uint64_t start_pos[128];
	int i, j;
	srand(time(NULL));
	for (i = 0; i < IOVN; ++i){
		fd[i] = open("/dev/nvme0n1", O_WRONLY);
		if (fd[i] < 0){
			printf("block device open failed, errno = %d\n", errno);
			goto exit;
		}
		printf("Open success, fd = %d\n", fd[i]);
		len[i] = BUFSZ;
		start_pos[i] = 1024*1024*1024;
		start_pos[i] *= i;
//		uint64_t high = rand();
//		start_pos[i] |= (high << 32);
//		printf("%d %ld\n", len[i], start_pos[i]);
		base[i] = (uint64_t)malloc(BUFSZ);
	}

	struct timeb start_t, end_t;
	uint64_t t_sec, t_ms;
	ftime(&start_t);
	for (i = 1; i < IOVN; ++i){
		lib_nvme_batch_sync(&fd[i], &base[i], &len[i], &start_pos[i], 1);
	}
	for (i = 0; i < IOVN; ++i){
		fsync(fd[i]);
//		close(fd[i]);
	}
	ftime(&end_t);
	t_sec = end_t.time - start_t.time;
	t_ms = end_t.millitm - start_t.millitm;
	printf("Single time:%d ms\n", t_sec*1000+t_ms);
/*
	for (i = 0; i < IOVN; ++i){
		fd[i] = open("/dev/nvme0n1", O_WRONLY);
		if (fd[i] < 0){
			printf("block device open failed, errno = %d\n", errno);
			goto exit;
		}
	}

	ftime(&start_t);
	lib_nvme_batch_sync(fd, base, len, start_pos, IOVN);
	for (i = 0; i < IOVN; ++i){
		fsync(fd[i]);
//		close(fd[i]);
	}
	ftime(&end_t);
	t_sec = end_t.time - start_t.time;
	t_ms = end_t.millitm - start_t.millitm;
	printf("Batch time:%d ms\n", t_sec*1000+t_ms);
*/
//	close(fd);
exit:
	for(i = 0; i < IOVN; ++i){
		free((char*)base[i]);
		close(fd[i]);
	}
	return 0;
}