#define _GNU_SOURCE

#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include "nvme-lib.h"

#define IOVN 32
#define BUFSZ (4096 * 4096 * 8)

char data5[IOVN][BUFSZ];

int main(){
	nvme_iovec_t* iovec = malloc(IOVN * sizeof(nvme_iovec_t));

	int i, j, fd;
	fd = open("/dev/nvme0n1", O_RDWR|O_NONBLOCK|O_DIRECT);
	if (fd < 0){
		printf("block device open failed, errno = %d\n", errno);
		goto exit;
	}
	printf("Open success, fd = %d\n", fd);

	// get namespace id
	int nsid = ioctl(fd, NVME_IOCTL_ID);
	printf("Namespace id: %d\n", nsid);

	for (i = 0; i < IOVN; ++i){
		for(j = 0; j < BUFSZ; ++j){
			data5[i][j] = '0';
		}
	}

	srand(time(NULL));

	for (i = 0; i < IOVN; ++i){
		iovec[i].iov_base = (uint64_t)data5[i];
		iovec[i].iov_len = sizeof(data5[i])/512;
		iovec[i].iov_lba = rand()%4096;
		iovec[i].iov_opcode = NVME_IOV_WRITE;
	}
	close(fd);

	struct timeb start_t, end_t;
	uint64_t t_sec, t_ms;
	ftime(&start_t);
	for (i = 0; i < 32; i ++){
		fd = open("/dev/nvme0n1", O_RDWR|O_NONBLOCK|O_DIRECT);
		lib_nvme_batch_cmd(fd, nsid, iovec, 1);
		close(fd);
	}
//	lib_nvme_batch_cmd(fd, nsid, iovec, IOVN);
	ftime(&end_t);
	t_sec = end_t.time - start_t.time;
	t_ms = end_t.millitm - start_t.millitm;
	printf("Batch time:%d ms\n", t_sec*1000+t_ms);

	ftime(&start_t);
	for (i = 0; i < IOVN; ++i){
		fd = open("/dev/nvme0n1", O_RDWR|O_NONBLOCK|O_DIRECT);
		lib_nvme_write(fd, nsid, (char*)iovec[0].iov_base, iovec[0].iov_len, iovec[0].iov_lba);
//		lib_nvme_write(fd, nsid, (char*)iovec[i].iov_base, iovec[i].iov_len, iovec[i].iov_lba);
		close(fd);
	}
	ftime(&end_t);
	t_sec = end_t.time - start_t.time;
	t_ms = end_t.millitm - start_t.millitm;
	printf("Seq time:%d ms\n", t_sec*1000+t_ms);


/*
	// write data: 0~511 logical blocks
	for(i = 0; i < sizeof(data5); i ++)
		data5[i] = 'd';
	printf("\nWriting LBA 0~511.\n");
	printf("Write cmd return: %d\n",lib_nvme_write(fd, nsid, data5, 4095, 0x0));

	// read again
	char data6[4096 * 128] = "";
	printf("\nReading LBA 0~511.\n");
	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data6, 511, 0x706));
	printf("read length: %d\n", strlen(data6));
	printf("a[0]:%c, a[131071]:%c, a[131072]:%c\n", data6[0], data6[131071], data6[131072]);
*/
	printf("Wocao\n");

//	close(fd);
exit:
	free(iovec);
}