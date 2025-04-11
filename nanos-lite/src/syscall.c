#include "common.h"
#include "syscall.h"

static int sys_non(_RegSet *r) {
  return 1;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case SYS_none:{
      SYSCALL_ARG1(r) = sys_non(r);
      return r;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
