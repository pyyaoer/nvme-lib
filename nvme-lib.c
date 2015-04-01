#include "nvme-lib.h"

int lib_nvme_write(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba){
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
    printf("lib_nvme_write:%d\n", errno);
  }
  return err;
}

int lib_nvme_read(int fd, int nsid, char* base, uint64_t len, uint64_t start_lba){
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
    printf("lib_nvme_read:%d\n", errno);
  }
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

int lib_nvme_unmap(int fd, int nsid, uint64_t start_lba, unsigned nlb){
  if (fd < 0)
    return -1;

  struct nvme_dsm_range range;
  memset(&range, 0, sizeof(range));
  range.nlb = nlb;
  range.slba = start_lba;

  struct nvme_dsm_cmd cmd;
  memset(&cmd, 0, sizeof(cmd));

  cmd.opcode = nvme_cmd_dsm;
  cmd.nsid = nsid;
  cmd.prp1 = (unsigned long)&range;
  cmd.nr = 0;
  cmd.attributes = NVME_DSMGMT_AD;

  int err = ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);

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
