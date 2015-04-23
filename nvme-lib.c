#define _GNU_SOURCE
#include "nvme-lib.h"

static uint64_t rw_max_size = 256;
static uint64_t lba_size = 512;

int lib_nvme_write_core(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba){
	if (fd < 0)
		return -1;

	struct nvme_user_io io;
	memset(&io, 0, sizeof(io));

	io.opcode = nvme_cmd_write;
	io.addr = (unsigned long)base;
	io.slba = start_lba;
	io.nblocks = len;
	int err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);

	if (err < 0){
		printf("lib_nvme_write_core:%d\n", errno);
	}
	return err;
}

int lib_nvme_write(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba){
	if (len < rw_max_size){
		return lib_nvme_write_core(fd, nsid, base, len, start_lba);
	}
	else{
		int num = len / rw_max_size;
		int tail = len % rw_max_size;
		int i, err;
		for (i = 0; i < num; ++i){
			err = lib_nvme_write_core(fd, nsid, base+i*rw_max_size*lba_size, rw_max_size-1, start_lba+i*rw_max_size);
			if(err < 0)
				return err;
		}
		err = lib_nvme_write_core(fd, nsid, base+num*rw_max_size, tail, start_lba+i*rw_max_size);
	}
	return 0;
}

int lib_nvme_read_core(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba){
	if (fd < 0)
		return -1;

	struct nvme_user_io io;
	memset(&io, 0, sizeof(io));

	io.opcode = nvme_cmd_read;
	io.addr = (unsigned long)base;
	io.slba = start_lba;
	io.nblocks = len;

	int err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);

	if (err < 0){
		printf("lib_nvme_read_core:%d\n", errno);
	}
	return err;
}

int lib_nvme_read(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba){
	if (len < rw_max_size){
		return lib_nvme_read_core(fd, nsid, base, len, start_lba);
	}
	else{
		int num = len / rw_max_size;
		int tail = len % rw_max_size;
		int i, err;
		for (i = 0; i < num; ++i){
			err = lib_nvme_read_core(fd, nsid, base+i*rw_max_size*lba_size, rw_max_size-1, start_lba+i*rw_max_size);
			if(err < 0)
				return err;
		}
		err = lib_nvme_read_core(fd, nsid, base+num*rw_max_size, tail, start_lba+i*rw_max_size);
	}
	return 0;
}

inline int set_cpu(int i){
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(i,&mask);
	printf("thread %u, i= %d\n", pthread_self(), i);
	return pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
}

void* lib_nvme_single_cmd(void* args){
	arg_struct_t* pargs = (arg_struct_t*)args;
	if(set_cpu(pargs->cpu) < 0)
		printf("Set CPU error.\n");
	switch(pargs->opcode){
		case NVME_IOV_WRITE:
			lib_nvme_write(pargs->fd, pargs->nsid, pargs->data, pargs->len, pargs->start_lba);
			break;
		case NVME_IOV_READ:
			lib_nvme_read(pargs->fd, pargs->nsid, pargs->data, pargs->len, pargs->start_lba);
			break;
		case NVME_IOV_TRIM:
			lib_nvme_unmap(pargs->fd, pargs->nsid, pargs->len, pargs->start_lba);
			break;
		default:
			printf("Invalid opcode!\n");
	}
	return NULL;
}

int lib_nvme_batch_cmd(int fd, int nsid, const nvme_iovec_t *iov, uint32_t iovcnt){
	int i;
	int err = -1;
	int cpu = 0;
	arg_struct_t* args = malloc(iovcnt*sizeof(arg_struct_t));
	pthread_t* tid = malloc(iovcnt*sizeof(pthread_t));
	for (i = 0; i < iovcnt; ++i){
		args[i].cpu = cpu;
		args[i].fd = fd;
		args[i].nsid = nsid;
		args[i].data = (char*)iov[i].iov_base;
		args[i].len = iov[i].iov_len;
		args[i].opcode = iov[i].iov_opcode;
		args[i].start_lba = iov[i].iov_lba;
		cpu = (cpu + 1) % MY_CPU_NUM;
	}
	for (i = 0; i < iovcnt; ++i){
		if (pthread_create(&tid[i], NULL, &lib_nvme_single_cmd, (void*)&args[i])){
			printf("ERROR creating thread!\n");
			goto exit;
		}
	}
	for (i = 0; i < iovcnt; ++i){
		if (pthread_join(tid[i],NULL)){
			printf("ERROR joining thread!\n");
			goto exit;
		}
	}
exit:
//	pthread_exit(NULL);
	free(tid);
	free(args);
	return err;
}

int lib_nvme_flush(int fd, int nsid){
	if (fd < 0)
		return -1;

	struct nvme_common_command cmd;
	memset(&cmd, 0, sizeof(cmd));

	cmd.opcode = nvme_cmd_flush;
	cmd.nsid = 1;

	int err = ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);
	if (err < 0){
		printf("lib_nvme_flush:%d\n", errno);
	}
	return err;
}

#define MAX_NUM_ADDR 128
#define SENSE_BUFF_LEN 64
#define UNMAP_CMD 0x42
#define UNMAP_CMDLEN 10
#define SG_DXFER_TO_DEV -2
int lib_nvme_unmap(int fd, int nsid, unsigned nlb, uint64_t start_lba){
	unsigned char sense_b[SENSE_BUFF_LEN];
	memset(sense_b, 0, sizeof(sense_b));

	uint64_t addr_arr[MAX_NUM_ADDR];
	uint32_t num_arr[MAX_NUM_ADDR];
	memset(addr_arr, 0, sizeof(addr_arr));
	addr_arr[0] = start_lba;
	memset(num_arr, 0, sizeof(num_arr));
	num_arr[0] = nlb;
	int addr_arr_len = 1;

	unsigned char paramp[8 + (MAX_NUM_ADDR * 16)];
	int param_len = 8 + (16 * addr_arr_len);
	memset(paramp, 0, param_len);

	int j, k = 8;
	for (j = 0; j < addr_arr_len; ++j) {
		paramp[k++] = (addr_arr[j] >> 56) & 0xff;
		paramp[k++] = (addr_arr[j] >> 48) & 0xff;
		paramp[k++] = (addr_arr[j] >> 40) & 0xff;
		paramp[k++] = (addr_arr[j] >> 32) & 0xff;
		paramp[k++] = (addr_arr[j] >> 24) & 0xff;
		paramp[k++] = (addr_arr[j] >> 16) & 0xff;
		paramp[k++] = (addr_arr[j] >> 8) & 0xff;
		paramp[k++] = addr_arr[j] & 0xff;
		paramp[k++] = (num_arr[j] >> 24) & 0xff;
		paramp[k++] = (num_arr[j] >> 16) & 0xff;
		paramp[k++] = (num_arr[j] >> 8) & 0xff;
		paramp[k++] = num_arr[j] & 0xff;
		k += 4;
	}
	k = 0;
	int num = param_len - 2;
	paramp[k++] = (num >> 8) & 0xff;
	paramp[k++] = num & 0xff;
	num = param_len - 8;
	paramp[k++] = (num >> 8) & 0xff;
	paramp[k++] = num & 0xff;

	unsigned char uCmdBlk[UNMAP_CMDLEN] =
		{UNMAP_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uCmdBlk[1] = 0; //archor
	uCmdBlk[6] = 0; //group number
	uCmdBlk[7] = (param_len >> 8) & 0xff;
	uCmdBlk[8] = param_len & 0xff;

	sg_io_hdr_t io_hdr = {0};

	io_hdr.interface_id = 'S';
	io_hdr.cmdp = (unsigned char*)uCmdBlk;
	io_hdr.cmd_len = sizeof(uCmdBlk);
	io_hdr.sbp = sense_b;
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxferp = (unsigned char*)paramp;
	io_hdr.dxfer_len = param_len;
	io_hdr.dxfer_direction = SG_DXFER_TO_DEV;
	io_hdr.timeout = 1000;

	int err = ioctl(fd, SG_IO, &io_hdr);
	if (err < 0){
		printf("lib_nvme_unmap:%d\n", errno);
	}
	return err;
}

int lib_nvme_nsid(int fd){
	int err = ioctl(fd, NVME_IOCTL_ID);
	if (err < 0){
		printf("lib_nvme_nsid:%d\n", errno);
	}
	return err;
}

//set:0 get:1
int lib_nvme_features(int fd, int set_get, int feature, int cdw11, int* res){
	struct nvme_admin_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = (set_get == 0) ? nvme_admin_set_features : nvme_admin_get_features;
	cmd.cdw10 = feature;
	cmd.cdw11 = cdw11;

	int err = ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);
	if (err < 0){
		printf("lib_nvme_features:%d\n", errno);
	}
	else{
		*res = cmd.result;
	}
	return err;
}

int lib_nvme_set_awun(int fd, int block_awun){
	int res = -1;
	lib_nvme_features(fd, 0, NVME_FEAT_WRITE_ATOMIC, block_awun, &res);
	return res;
}

int lib_nvme_get_awun(int fd, int block_awun){
	int res = -1;
	lib_nvme_features(fd, 1, NVME_FEAT_WRITE_ATOMIC, block_awun, &res);
	return res;
}

int lib_nvme_identify(int fd, int nsid, void *ptr, int cns)
{
	struct nvme_admin_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = nvme_admin_identify;
	cmd.nsid = nsid;
	cmd.addr = (unsigned long)ptr;
	cmd.data_len = 4096;
	cmd.cdw10 = cns;

	int err = ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);
	if (err < 0){
		printf("lib_nvme_identify:%d\n", errno);
	}

	return err;
}

//namespace size
int lib_nvme_nsze(int fd, int nsid){
	struct nvme_id_ns ns;
	lib_nvme_identify(fd, nsid, &ns, 0);
	return ns.nsze;
}

int lib_nvme_lba_size(int fd, int nsid){
	struct nvme_id_ns ns;
	lib_nvme_identify(fd, nsid, &ns, 0);
	return 1 << (ns.lbaf[0].ds);
}

int lib_nvme_metadata_size(int fd, int nsid){
	struct nvme_id_ns ns;
	lib_nvme_identify(fd, nsid, &ns, 0);
	return ns.lbaf[0].ms;
}

int lib_nvme_awun_size(int fd, int nsid){
	struct nvme_id_ctrl ctrl;
	lib_nvme_identify(fd, nsid, &ctrl, 1);
	return ctrl.awun;
}

int lib_nvme_awupf_size(int fd, int nsid){
	struct nvme_id_ctrl ctrl;
	lib_nvme_identify(fd, nsid, &ctrl, 1);
	return ctrl.awupf;
}

// maximum data transfer size
int lib_nvme_mdts(int fd, int nsid){
	struct nvme_id_ctrl ctrl;
	lib_nvme_identify(fd, nsid, &ctrl, 1);
	return (ctrl.mdts == 0) ? 0 : (1 << ctrl.mdts);
}

/*
int lib_nvme_setup_prps(uint64_t data, int data_len, uint64_t* prp1, uint64_t* prp2){
	int pgsize = getpagesize();
	int offset = data % pgsize;
	int nprps = data_len / pgsize;

	int length = data_len + offset - pgsize;
	if (length <= 0)
		return data_len;
}
*/
