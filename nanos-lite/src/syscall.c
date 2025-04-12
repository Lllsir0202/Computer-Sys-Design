#include "common.h"
#include "syscall.h"

extern char end;
static uintptr_t program_break = (uintptr_t)&end;

static int sys_non(_RegSet *r) {
  return 1;
}

static void sys_exit(uintptr_t param) {
  // 其实本质就是将push 1为例，传入eax，然后将其作为_halt的参数
  _halt(param);
}

static int sys_write(int fd, const void *buf, size_t count) {
  if(fd == 1 || fd == 2){
    // 1: stdout, 2: stderr
    int i = 0;
    // Log("sys_write: %d", count);
    for(i = 0; i < count; i++){
      _putc(((char *)buf)[i]);
    }
    return count;
  } else {
    return -1;
  }
}

static int sys_brk(void *addr) {
  return 0;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

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
      int ret_value = sys_write(SYSCALL_ARG2(r), (void *)SYSCALL_ARG3(r), SYSCALL_ARG4(r));
      SYSCALL_ARG1(r) = ret_value;
      return r;
    }
    case SYS_brk: {
      // 目前的理解是：其实就是ebx->increment，计算出地址addr，然后再那里设置res，如果成功则返回0,并返回旧的program_break
      // 如果失败则返回-1
      int increment = SYSCALL_ARG2(r);
      uintptr_t addr = program_break + increment;
      uintptr_t program_break_old = program_break;
      int ret = sys_brk((void *)addr);
      if(ret == 0) {
        program_break = addr;
        SYSCALL_ARG1(r) = program_break_old;
      } else {
        SYSCALL_ARG1(r) = -1;
      }
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
