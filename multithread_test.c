
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <omp.h>
#include "nvme-lib.h"

char data[26][4096];
char rdata[4096];

void read_result(){
	int i;
	for (i = 20; i < 30; ++i){
		printf("%c", rdata[i]);
	}
	printf("\n");
}

int main(){

	int i, fd;
	char c;
	fd = open("/dev/nvme0n1",O_RDWR);
	if (fd < 0)
	{
		printf("block device open failed, errno = %d\n", errno);
		goto exit;
	}
	printf(" open: success, fd = %d\n", fd);

	// get namespace id
	int nsid = ioctl(fd, NVME_IOCTL_ID);
	printf(" namespace id: %d\n", nsid);

	for (c = 'a'; c <= 'z'; c ++){
		for(i = 0; i < 4096; i ++)
			data[c-'a'][i] = c;
	}

	#pragma omp parallel for
	for (c = 'a'; c <= 'z'; c ++){
		printf("Write %c cmd return: %d\n", c, lib_nvme_write(fd, nsid, data[c-'a'], 0, 0x500));
		printf("Read %c cmd return: %d\n", c, lib_nvme_read(fd, nsid, rdata, 0, 0x500));
		read_result();
	}

	close(fd);
	return 0;

	exit:
	close(fd);
	return -1;
}
