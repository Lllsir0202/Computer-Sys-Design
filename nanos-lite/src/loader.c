#include "common.h"
extern void ramdisk_read(void *buf, size_t offset, size_t len);
extern void ramdisk_write(void *buf, size_t offset, size_t len);
extern size_t get_ramdisk_size();
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);
extern size_t fs_filesz(int fd);

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  // size_t len = get_ramdisk_size();
  // char ramdisk_buf[len];
  // if (ramdisk_buf == NULL) {
  //   panic("Failed to allocate memory for ramdisk buffer");
  // }
  // ramdisk_read(ramdisk_buf, 0, len);
  // memcpy(DEFAULT_ENTRY, ramdisk_buf, len);
  // return (uintptr_t)DEFAULT_ENTRY;

  // -------Change in pa3-2--------
  Log("In load file name is %s",filename);
  int fd = fs_open(filename, 0, 0);
  size_t len = fs_filesz(fd);
  if(len == 0) {
    panic("special file is read");
  }
  char buf[len];
  fs_read(fd, (void *)buf, len);
  fs_close(fd);
  memcpy(DEFAULT_ENTRY, buf, len);
  Log("file succeed to be load");
  return (uintptr_t)DEFAULT_ENTRY;
}
