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
    printf("%d\n", errno);
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
    printf("%d\n", errno);
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
    printf("%d\n", errno);
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
    printf("%d\n", errno);
  }
  return err;
}
