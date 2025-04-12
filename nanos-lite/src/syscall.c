#include "common.h"
#include "syscall.h"

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
    for(i = 0; i < count; i++){
      _putc(((char *)buf)[i]);
    }
    return count;
  } else {
    return -1;
  }
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
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
