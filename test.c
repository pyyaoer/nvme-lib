
#include <stdio.h>
#include <errno.h>
#include <string.h>
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
	printf("namespace size:%d\n",lib_nvme_nsze(fd,nsid));
	printf("LBA data size:%d\n",lib_nvme_lba_size(fd,nsid));
	printf("metadata size:%d\n",lib_nvme_metadata_size(fd,nsid));
	printf("awun size:%d\n",lib_nvme_awun_size(fd,nsid));
	printf("awupf size:%d\n",lib_nvme_awupf_size(fd,nsid));
	printf("maximum data transfer size:%d\n", lib_nvme_mdts(fd,nsid));

	// init read
	char data0[4096 * 64] = "";
	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data0, 3, 0x0));
	printf("---read result: %d\n", strlen(data0));
	
	// write data
	char data1[4096 * 64] = "";
	int i = 0;
	for(i = 0; i < sizeof(data1); i ++)
		data1[i] = 'b';
	printf("write cmd return: %d\n",lib_nvme_write(fd, nsid, data1, 32*8-1, 0x0));

	// read data
	char data2[4096 * 64] = {0};
	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data2, 32*8-1, 0x0));
	printf("---read result: %d\n", strlen(data2));
	printf("%c %c %c\n", data2[0], data2[131071], data2[131072]);

	// unmap
	printf("unmap cmd return: %d\n",lib_nvme_unmap_scsi(fd, 32*8, 0x0));

	// read again
	char data3[4096 * 64] = "";
	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data3, 3, 0x0));
	printf("---read result: %d\n", strlen(data3));

	close(fd);
	return 0;

	exit:
	close(fd);
	return -1;
}
