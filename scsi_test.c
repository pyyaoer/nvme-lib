#define _GNU_SOURCE

#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <sys/timeb.h>
#include "nvme-lib.h"

//#define IOVN 8
#define BUFSZ (1024 * 32)

int main(int argc, char* argv[]){
	int fd;
	int i, j;
	srand(time(NULL));
	fd = open("/dev/nvme0n1", O_RDWR | O_DIRECT);
	if (fd < 0){
		printf("block device open failed, errno = %d\n", errno);
		goto exit;
	}
	printf("Open success, fd = %d\n", fd);


	/************************************/
	// write data: 0~255 logical blocks
	char data1[4096 * 64] = "";
	for(i = 0; i < sizeof(data1); ++i)
		data1[i] = 'w';
	printf("\nWriting LBA 0~255.\n");
	printf("SCSI write return: %d\n", lib_nvme_write_scsi(fd, data1, 256, 0x0));

	// read data
	char data2[4096 * 64] = {0};
	printf("SCSI read return: %d\n", lib_nvme_read_scsi(fd, data2, 256, 0x0));
	printf("read length: %d\n", strlen(data2));
	printf("a[0]:%c, a[131071]:%c, a[131072]:%c\n", data2[0], data2[131071], data2[131072]);

	int IOVN = atoi(argv[1]);
	uint64_t base[1024*32];
	uint64_t len[1024*32];
	uint64_t start_pos[1024*32];

	/************************************/
	for (i = 0; i < IOVN; ++i){
		len[i] = BUFSZ;
		start_pos[i] = i*BUFSZ;
		if (i < 1024)
			base[i] = (uint64_t)malloc(BUFSZ * 512);
		else
			base[i] = base[i-1024];
	}
	struct timeb start_t, end_t;
	uint64_t t_sec, t_ms;
	ftime(&start_t);
	for (i = 0; i < IOVN; ++i){
		lib_nvme_write_scsi(fd, (char*)base[i], len[i], start_pos[i]);
	}
	fsync(fd);
	ftime(&end_t);
	t_sec = end_t.time - start_t.time;
	t_ms = end_t.millitm - start_t.millitm + 1000 * t_sec;
	printf("\nWrite time:%d ms\n", t_ms);
	printf("Write Speed:%lf GB/s\n", IOVN*1000.0/64.0/t_ms);

	ftime(&start_t);
	for (i = 0; i < IOVN; ++i){
		lib_nvme_read_scsi(fd, (char*)base[i], len[i], start_pos[i]);
	}
	ftime(&end_t);
	t_sec = end_t.time - start_t.time;
	t_ms = end_t.millitm - start_t.millitm + 1000 * t_sec;
	printf("\nRead time:%d ms\n", t_ms);
	printf("Read Speed:%lf GB/s\n", IOVN*1000.0/64.0/t_ms);

	for(i = 0; i < 1024; ++i){
		free((char*)base[i]);
	}

exit:
	close(fd);
	return 0;
}
