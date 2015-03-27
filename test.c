
#include <stdio.h>
#include <errno.h>
#include "nvme-lib.h"

int main(){

	int fd;
	fd = open("/dev/nvme0n1",O_RDWR);
	if (fd < 0)
	{
		printf("block device open failed, errno = %d\n", errno);
		goto exit;
	}
	printf(" open: success, fd = %d\n", fd);

	int nsid = ioctl(fd, NVME_IOCTL_ID);
	printf(" namespace id: %d\n", nsid);

	char data1[4096] = "wocao";
	printf("write cmd return: %d\n",lib_nvme_write(fd, nsid, data1, 1, 100));
	char data2[4096] = "";
//	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data2, 1, 100));
//	printf("read result: %s\n", data2);

	printf("unmap cmd return: %d\n",lib_nvme_unmap(fd, nsid, 100, 1));
	printf("flush cmd return: %d\n",lib_nvme_flush(fd, nsid));
	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data2, 1, 100));
	printf("read result: %s\n", data2);

	close(fd);
	return 0;

	exit:
	close(fd);
	return -1;
}
