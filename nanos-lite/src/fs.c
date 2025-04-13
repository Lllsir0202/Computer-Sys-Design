#include "fs.h"
extern void ramdisk_read(void *buf, size_t offset, size_t len);
extern void ramdisk_write(void *buf, size_t offset, size_t len);
extern size_t get_ramdisk_size();
extern size_t get_screen_size();
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
// #define DEBUG

typedef struct {
  char *name;         // 文件名
  size_t size;        // 文件大小
  off_t disk_offset;  // 文件在ramdisk中的偏移
  off_t open_offset;  // 文件被打开之后的读写指针，我们先默认为0
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0, -1},
  {"stdout (note that this is not the actual stdout)", 0, 0, -1},
  {"stderr (note that this is not the actual stderr)", 0, 0, -1},
  [FD_FB] = {"/dev/fb", 0, 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  // 首先得到size
  size_t size = get_screen_size();
  Log("screen size is %d", size);
  file_table[FD_FB].size = size;
  // 将后面的所有文件移动，避免交叉 -> error
  // 不需要这样做，只需要记录下size就好了
  // for(int i = FD_EVENTS; i < NR_FILES; i++) {
  //   file_table[i].disk_offset += (size+start);
  //   #ifdef DEBUG
  //   Log("file_table[%d].disk_offset is %d", i, file_table[i].disk_offset);
  //   #endif
  // }
  Log("fs initialize succeeds");
}

// ADD in pa3-2:实现文件系统的有关操作
// 得到文件长度
size_t fs_filesz(int fd) {
  if(fd < 0 || fd >= NR_FILES) {
    panic("fd out of range");
    return -1;
  }
  return file_table[fd].size;
}

// open
int fs_open(const char *pathname, int flags, int mode) {
  // loop circle to find pathname
  #ifdef DEBUG
  Log("pathname is %s",pathname);
  #endif
  for(int i = 0 ; i < NR_FILES ; i++) {
    if(strcmp(pathname, file_table[i].name) == 0) {
      // if the file is found, return the index
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("file not found");
  return -1;
}

// read
ssize_t fs_read(int fd, void *buf, size_t len) {
  #ifdef DEBUG
  Log("fs_read!!! fd is %d", fd);
  #endif
  
  if(fd>= 0 && fd <= 2) {
    return 0;
  }
  off_t offset = file_table[fd].open_offset;
  // 目前看来不太可能，正常来说
  if(fd < 0 || fd >= NR_FILES) {
    panic("fd out of range");
  }
  switch (fd) {
    case FD_STDOUT: 
    case FD_STDIN:
    case FD_STDERR: {
      return 0;
    }
    case FD_DISPINFO: {
      // Log("offset is %d", offset);
      if(offset >= file_table[fd].disk_offset) {
        return 0;
      }
      dispinfo_read(buf, offset, len);
      Log("buf is %s",buf);
      // Log("len is %d", len);
      // Log("before add open_offset is %d", file_table[fd].open_offset);
      if(len > file_table[fd].size) {
        len = file_table[fd].size;
      }
      file_table[fd].open_offset += len;
      // Log("after add open_offset is %d", file_table[fd].open_offset);
    } break;
    case FD_FB: {
      panic("Cannot be read!");
      return 0;
    } break;
    default: {
      // check if the file is opened
      if(offset < 0) {
        panic("file not opened");
        return -1;
      }
      // check if the file is overflow
      // 如果当前的offset已经是文件尾了
      // 返回0,不读取
      if(offset >= fs_filesz(fd)) {
        return 0;
      }
      // 我们处理是：如果当前的len加上openoffset超过了末尾，那么读取尽可能多的。
      if(offset + len > fs_filesz(fd)) {
        // 更新读取的len
        len = fs_filesz(fd) - offset;
      }
      off_t ramdisk_offset = file_table[fd].disk_offset + offset;
      // 进行读取
      ramdisk_read((void *)buf, ramdisk_offset, len);
      file_table[fd].open_offset += len;
    } break;
  }
  return len;
}

// close
int fs_close(int fd) {
  if(fd < 0 || fd >= NR_FILES) {
    panic("fd out of range");
    return -1;
  }
  file_table[fd].open_offset = -1;
  return 0;
}

// write
ssize_t fs_write(int fd, const void *buf, size_t len) {
  // 目前看来不太可能，正常来说
  #ifdef DEBUG
  Log("fs_write!!! fd is %d", fd);
  #endif
  if(fd < 0 || fd >= NR_FILES) {
    panic("fd out of range");
  }
  off_t offset = file_table[fd].open_offset;
  Log("fd is %d", fd);
  switch (fd) {
    case FD_STDOUT:
    case FD_STDERR: {
      // 1: stdout, 2: stderr
      int i = 0;
      // Log("sys_write: %d", count);
      for(i = 0; i < len; i++){
        _putc(((char *)buf)[i]);
      }
    } break;
    case FD_FB: {
      panic("here");
      fb_write(buf, offset, len);
      file_table[fd].open_offset += len;
    } break;
    case FD_STDIN: {
      return 0;
    } break;
    default: {
      // check if the file is opened
      if(offset < 0) {
        panic("file not opened");
        return -1;
      }
      // check if the file is overflow
      // 如果当前的offset已经是文件尾了
      if(offset >= fs_filesz(fd)) {
        return 0;
      }
      // 我们处理是：如果当前的len加上openoffset超过了末尾，那么写入尽可能多的。
      if(offset + len > fs_filesz(fd)) {
        // 更新读取的len
        len = fs_filesz(fd) - offset;
      }
      off_t ramdisk_offset = file_table[fd].disk_offset + offset;
      // 进行读取
      ramdisk_write((void *)buf, ramdisk_offset, len);
      file_table[fd].open_offset += len;
    } break;
  }
  return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  if(fd < 0 || fd > NR_FILES) {
    panic("fd out of range");
  }
  off_t openoffset = file_table[fd].open_offset;
  // 没有open
  if(openoffset < 0) {
    panic("file not open");
    return -1;
  }

  switch (whence){
    case SEEK_SET: {
      // 设置
      file_table[fd].open_offset = offset;
      #ifdef DEBUG
      Log("set offset(SET) is %d", file_table[fd].open_offset);
      #endif
    } break;
    case SEEK_CUR: {
      file_table[fd].open_offset += offset;
      #ifdef DEBUG
      Log("set offset(CUR) is %d", file_table[fd].open_offset);
      #endif
    } break;
    case SEEK_END: {
      file_table[fd].open_offset = fs_filesz(fd) + offset;
      #ifdef DEBUG
      Log("set offset(END) is %d", file_table[fd].open_offset);
      #endif
    } break;
    default: {
      panic("Invalid whence!");
    }
  }
  return file_table[fd].open_offset;
}