
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

	// get namespace id
	int nsid = ioctl(fd, NVME_IOCTL_ID);
	printf(" namespace id: %d\n", nsid);

	// set & get awun
	printf("set awun return:%d\n",lib_nvme_set_awun(fd, 0));
	printf("get awun return:%d\n",lib_nvme_get_awun(fd, 0));

	// get nvme info
	printf("ns size:%d\n",lib_nvme_nsze(fd,nsid));
	printf("awun size:%d\n",lib_nvme_awun_size(fd,nsid));
	printf("awupf size:%d\n",lib_nvme_awupf_size(fd,nsid));

	// init read
	char data0[4096] = "";
	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data0, 1, 0x100));
	printf("read result: %s\n", data0);
	
	// write data
	char data1[4096] = "test";
	printf("write cmd return: %d\n",lib_nvme_write(fd, nsid, data1, 1, 0x100));

	// read data
	char data2[4096] = "";
	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data2, 1, 0x100));
	printf("read result: %s\n", data2);

	// unmap
	printf("unmap cmd return: %d\n",lib_nvme_unmap(fd, nsid, 0x100, 1));

	// read again
	char data3[4096] = "";
	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data3, 1, 0x100));
	printf("read result: %s\n", data3);

	close(fd);
	return 0;

	exit:
	close(fd);
	return -1;
}
