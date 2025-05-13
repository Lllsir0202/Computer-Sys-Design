#include "common.h"
#include "syscall.h"

extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern ssize_t fs_write(int fd, const void *buf, size_t len);
extern off_t fs_lseek(int fd, off_t offset, int whence);
extern int fs_close(int fd);
extern int mm_brk(uint32_t new_brk);

// Add in pa4-4
extern _RegSet* schedule(_RegSet *r, bool keyboard);

static int sys_non(_RegSet *r) {
  return 1;
}

static void sys_exit(uintptr_t param) {
  // 其实本质就是将push 1为例，传入eax，然后将其作为_halt的参数
  _halt(param);
}

static ssize_t sys_write(int fd, const void *buf, size_t count) {
  // if(fd == 1 || fd == 2){
  //   // 1: stdout, 2: stderr
  //   int i = 0;
  //   // Log("sys_write: %d", count);
  //   for(i = 0; i < count; i++){
  //     _putc(((char *)buf)[i]);
  //   }
  //   return count;
  // } else {
  //   return fs_write(fd, buf, count);
  // }

  // In pa3-3: Rewrite this function
  return fs_write(fd, buf, count);
}

static int sys_brk(void *addr) {
  return mm_brk((uintptr_t)addr);
}

// ADD in pa3-3
static int sys_open(const char *pathname, int flags, int mode) {
  return fs_open(pathname, flags, mode);
}

static ssize_t sys_read(int fd, void *buf, size_t len) {
  return fs_read(fd, buf, len);
}

static off_t sys_lseek(int fd, off_t offset, int whence) {
  return fs_lseek(fd, offset, whence);
}

static int sys_close(int fd) {
  return fs_close(fd);
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  // Log("syscall is %d", a[0]);
  switch (a[0]) {
    case SYS_none:{
      SYSCALL_ARG1(r) = sys_non(r);
      return r;
    }
    case SYS_exit:{
      // 基于navy-apps/libs/libos/src/nanos.c中传递的参数，我们知道，
      // 其实eax->类型，ebx--edx -> 参数，而_exit传递的status就是在ebx中的
      sys_exit(SYSCALL_ARG2(r));
      return r;
    }
    case SYS_write:{
      // 基于man 2 write可以知道，三个参数分别是fd, buf, count
      SYSCALL_ARG1(r) = sys_write(SYSCALL_ARG2(r), (void *)SYSCALL_ARG3(r), SYSCALL_ARG4(r));;
      return r;
    }
    case SYS_brk: {
      // 目前的理解是：其实就是ebx->increment，计算出地址addr，然后再那里设置res，如果成功则返回0,并返回旧的program_break
      // 如果失败则返回-1
      uintptr_t addr = SYSCALL_ARG2(r);
      // Log("increment is %d", increment);
      SYSCALL_ARG1(r) = sys_brk((void *)addr);
      return r;
    }
    case SYS_open: {
      SYSCALL_ARG1(r) = sys_open((char *)SYSCALL_ARG2(r), SYSCALL_ARG3(r), SYSCALL_ARG4(r));
      return r;
    }
    case SYS_read: {
      SYSCALL_ARG1(r) = sys_read(SYSCALL_ARG2(r), (void *)SYSCALL_ARG3(r), SYSCALL_ARG4(r));
      if(SYSCALL_ARG2(r) == 4){
        char* buf = (char *)SYSCALL_ARG3(r);
        if(SYSCALL_ARG1(r) != 0 && strcmp(buf, "kd F12\n") == 0){
        // Log("buf is %s", buf);
        return schedule(r, true);
        }
    }
      return r;
    }
    case SYS_lseek: {
      SYSCALL_ARG1(r) = sys_lseek(SYSCALL_ARG2(r), SYSCALL_ARG3(r), SYSCALL_ARG4(r));
      return r;
    }
    case SYS_close: {
      SYSCALL_ARG1(r) = sys_close(SYSCALL_ARG2(r));
      return r;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
