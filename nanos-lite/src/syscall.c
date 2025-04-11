#include "common.h"
#include "syscall.h"

static int sys_non(_RegSet *r) {
  return 1;
}

static void sys_exit(_RegSet *r) {
  // 其实本质就是将push 1为例，传入eax，然后将其作为_halt的参数
  Log("r->eax is %x, r->ebx is %x", r->eax, r->ebx);
  _halt(0);
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
      sys_exit(r);
      return r;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
