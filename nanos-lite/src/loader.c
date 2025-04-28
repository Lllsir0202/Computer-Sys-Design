#include "common.h"
extern void ramdisk_read(void *buf, size_t offset, size_t len);
extern void ramdisk_write(void *buf, size_t offset, size_t len);
extern size_t get_ramdisk_size();
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);
extern size_t fs_filesz(int fd);
extern void* new_page(void);

#define DEFAULT_ENTRY ((void *)0x8048000)

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
  // Log("here1");
  size_t len = fs_filesz(fd);
  // Log("len is %d", len);
  if(len == 0) {
    panic("special file is read");
  }
  // Log("here2");
  int page_num = len/PGSIZE;
  for(int i = 0; i < page_num; i++) {
    void *page = new_page();
    if(page == NULL) {
      panic("Failed to allocate memory for page");
    }
    Log("page is %p", page);
    fs_read(fd, page, PGSIZE);
    // Log("page is %p", page);
    _map(as, DEFAULT_ENTRY + i * PGSIZE, page);

  }
  if(len % PGSIZE != 0) {
    void *page = new_page();
    if(page == NULL) {
      panic("Failed to allocate memory for page");
    }
    fs_read(fd, page, len % PGSIZE);
    _map(as, DEFAULT_ENTRY + page_num * PGSIZE, page);
  }
  // // 首先获取一张空闲物理页
  // void *page = new_page();
  // _map(as, DEFAULT_ENTRY, page);
  // fs_read(fd, DEFAULT_ENTRY, len);
  fs_close(fd);
  Log("file succeed to be load");
  return (uintptr_t)DEFAULT_ENTRY;
}
