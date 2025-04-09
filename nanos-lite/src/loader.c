#include "common.h"
extern void ramdisk_read(void *buf, size_t offset, size_t len);
extern void ramdisk_write(void *buf, size_t offset, size_t len);
extern size_t get_ramdisk_size();
extern int ramdisk_start;

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  size_t len = get_ramdisk_size();
  char ramdisk_buf[len];
  if (ramdisk_buf == NULL) {
    panic("Failed to allocate memory for ramdisk buffer");
  }
  ramdisk_read(ramdisk_buf, ramdisk_start, len);
  memcpy(DEFAULT_ENTRY, ramdisk_buf, len);
  return (uintptr_t)DEFAULT_ENTRY;
}
