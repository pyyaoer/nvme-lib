#include <stdlib.h>
#include <time.h>
#include "nvme-lib.h"

#define IOVN 16

char data5[4096 * 4096 * 16] = "";

int main(){
	nvme_iovec_t* iovec = malloc(IOVN * sizeof(nvme_iovec_t));

	int i, fd;
	fd = open("/dev/nvme0n1",O_RDWR);
	if (fd < 0)
	{
		printf("block device open failed, errno = %d\n", errno);
		goto exit;
	}
	printf("Open success, fd = %d\n", fd);

	// get namespace id
	int nsid = ioctl(fd, NVME_IOCTL_ID);
	printf("Namespace id: %d\n", nsid);

	for (i = 0; i < IOVN; ++i){
		iovec[i].iov_base = (uint64_t)data5;
		iovec[i].iov_len = sizeof(data5)/ 512;
		iovec[i].iov_lba = rand() % 4096;
		iovec[i].iov_opcode = NVME_IOV_WRITE;
	}

	time_t start, end;
	start = time(NULL);
	lib_nvme_batch_cmd(fd, nsid, iovec, IOVN);
	end = time(NULL);
	printf("Batch time:%d\n", end-start);

	start = time(NULL);
	for (i = 0; i < IOVN; ++i){
		lib_nvme_write(fd, nsid, data5, iovec[i].iov_len, iovec[i].iov_lba);
	}
	end = time(NULL);
	printf("Seq time:%d\n", end-start);


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

	close(fd);
exit:
	free(iovec);
}