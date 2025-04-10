#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096    // Bytes mapped by a page

// struct _RegSet {
//   uintptr_t esi, ebx, eax, eip, edx, error_code, eflags, ecx, cs, esp, edi, ebp;
//   int       irq;
// };

struct _RegSet {
  // 越早压栈的,应该越后弹出,这里模拟的就是trap frame,所以我们根据push反过来处理即可
  // 最后是通用寄存器
  uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  // 然后依次push的error_code和irq;
  int       irq;
  // 栈底是EFLAGS, CS, EIP
  uintptr_t eip, cs, eflags, error_code;
};

#define SYSCALL_ARG1(r) 0
#define SYSCALL_ARG2(r) 0
#define SYSCALL_ARG3(r) 0
#define SYSCALL_ARG4(r) 0

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
