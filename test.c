
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

	printf("set awun return:%d\n",lib_nvme_set_awun(fd, 0));

	printf("get awun return:%d\n",lib_nvme_get_awun(fd, 0));

	printf("ns size:%d\n",lib_nvme_nsze(fd,nsid));

	printf("awun size:%d\n",lib_nvme_awun_size(fd,nsid));

	printf("awupf size:%d\n",lib_nvme_awupf_size(fd,nsid));
	

	char data1[4096] = "test";
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
