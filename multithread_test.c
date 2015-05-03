
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "nvme-lib.h"

char data[26][4096];
char rdata[4096];

struct arg_struct {
	char c;
	int fd;
	int nsid;
	char* data;
};

void read_result(){
	int i;
	for (i = 20; i < 30; ++i){
		printf("%c", rdata[i]);
	}
	printf("\n");
}

void* write_test(void* a){
	struct arg_struct* pargs = (struct arg_struct*)a;
	printf("Write %c cmd return: %d\n", pargs->c, lib_nvme_write_ioctl(pargs->fd, pargs->nsid, pargs->data, 0, 0x500));
	return NULL;
}

void* read_test(void* a){
	struct arg_struct* pargs = (struct arg_struct*)a;
	printf("Read %c cmd return: %d\n", pargs->c, lib_nvme_read_ioctl(pargs->fd, pargs->nsid, pargs->data, 0, 0x500));
	return NULL;
}

int main(){

	int i, fd;
	pthread_t tid[2 * 26];
	struct arg_struct args[2 * 26];
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
		args[c-'a'].c = c;
		args[c-'a'].fd = fd;
		args[c-'a'].nsid = nsid;
		args[c-'a'].data = data[c-'a'];
		args[26+c-'a'].c = c;
		args[26+c-'a'].fd = fd;
		args[26+c-'a'].nsid = nsid;
		args[26+c-'a'].data = rdata;
	}

	i = 0;
	for (c = 'a'; c <= 'z'; c ++){
		if (pthread_create(&tid[i], NULL, &write_test, (void*)&args[c-'a'])){
			printf("ERROR creating thread!\n");
			goto exit;
		}
		i++;
		if (pthread_create(&tid[i], NULL, &read_test, (void*)&args[26+c-'a'])){
			printf("ERROR creating thread!\n");
			goto exit;
		}
		i++;
	}

	for (i = 0; i < 2*26; ++i){
		if (pthread_join(tid[i],NULL)){
			printf("ERROR joining thread!\n");
			goto exit;
		}
	}

	read_result();

	pthread_exit(NULL);

	close(fd);
	return 0;

	exit:
	close(fd);
	return -1;
}
