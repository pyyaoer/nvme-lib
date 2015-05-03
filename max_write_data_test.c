#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "nvme-lib.h"

int main(){

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

	// get nvme info
	printf("LBA data size is %d bytes.\n", lib_nvme_lba_size(fd, nsid));
	printf("Maximum data transfer size is %d MPSMINS.\n", lib_nvme_mdts(fd, nsid));

	// write data: 0~255 logical blocks
	char data1[4096 * 64] = "";
	for(i = 0; i < sizeof(data1); i ++)
		data1[i] = 'w';
	printf("\nWriting LBA 0~255.\n");
	printf("Write cmd return: %d\n",lib_nvme_write_ioctl(fd, nsid, data1, 255, 0x0));

	// read data
	char data2[4096 * 64] = {0};
	printf("\nReading LBA 0~255.\n");
	printf("read cmd return: %d\n",lib_nvme_read_ioctl(fd, nsid, data2, 255, 0x0));
	printf("read length: %d\n", strlen(data2));
	printf("a[0]:%c, a[131071]:%c, a[131072]:%c\n", data2[0], data2[131071], data2[131072]);

	// unmap
	printf("\nUnmapping!\n");
	printf("unmap cmd return: %d\n",lib_nvme_unmap(fd, nsid, 256, 0x0));

	// write data: 0~256 logical blocks
	char data3[4096 * 64] = "";
	for(i = 0; i < sizeof(data3); i ++)
		data3[i] = 'w';
	printf("\nWriting LBA 0~256.\n");
	printf("Write cmd return: %d\n",lib_nvme_write_ioctl(fd, nsid, data3, 256, 0x0));

	// read again
	char data4[4096 * 64] = "";
	printf("\nReading LBA 0~256.\n");
	printf("read cmd return: %d\n",lib_nvme_read_ioctl(fd, nsid, data4, 256, 0x0));
	printf("read length: %d\n", strlen(data4));
	printf("a[0]:%c, a[131071]:%c, a[131072]:%c\n", data4[0], data4[131071], data4[131072]);

	// unmap
	printf("\nUnmapping!\n");
	printf("unmap cmd return: %d\n",lib_nvme_unmap(fd, nsid, 256, 0x0));

	// write data: 0~511 logical blocks
	char data5[4096 * 128] = "";
	for(i = 0; i < sizeof(data5); i ++)
		data5[i] = 'w';
	printf("\nWriting LBA 0~511.\n");
	printf("Write cmd return: %d\n",lib_nvme_write(fd, nsid, data5, 511, 0x0));

	// read again
	char data6[4096 * 128] = "";
	printf("\nReading LBA 0~511.\n");
	printf("read cmd return: %d\n",lib_nvme_read(fd, nsid, data6, 511, 0x0));
	printf("read length: %d\n", strlen(data6));
	printf("a[0]:%c, a[131071]:%c, a[131072]:%c\n", data6[0], data6[131071], data6[131072]);

	close(fd);
	return 0;

	exit:
	close(fd);
	return -1;
}
