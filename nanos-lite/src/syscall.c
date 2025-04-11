#include "common.h"
#include "syscall.h"

static int sys_non(_RegSet *r) {
  return 1;
}

static void sys_exit(uintptr_t param) {
  // 其实本质就是将push 1为例，传入eax，然后将其作为_halt的参数
  _halt(param);
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
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
