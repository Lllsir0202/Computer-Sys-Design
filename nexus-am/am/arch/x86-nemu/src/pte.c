#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
// 这里的kptabs是一个物理页表，实际上是一个物理页表的数组
// 由于一个PDE可能需要1024个PTE，所以其实需要1024x1024个PTE，所以这里是PMEM_SIZE/PGSIZE
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);


_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  // 这里其实就是因为，我们并没有实现分段机制，但是框架还是保留了分段的部分
  // 所以这里我们只有一个段，故需要使用NR_KSEG_MAP进行记录
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }
  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }
  
  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  uintptr_t vaddr = (uintptr_t)va;
  uintptr_t paddr = (uintptr_t)pa;
  // 这里的va是虚拟地址，pa是物理地址
  uint32_t PDE_index = PDX(vaddr);
  uint32_t PTE_index = PTX(vaddr);
  
  // 这个数组是物理页表的基址，然后我们可以通过PDE_index来找到对应的PTE
  PDE *updir = p->ptr;
  PTE data = updir[PDE_index];
  PTE *upte = NULL;
  if(!(data & PTE_P)) {
    // 如果没有这个物理页表
    // 获取一个页表项
    upte = (PTE*)palloc_f();
    updir[PDE_index] = (uintptr_t)upte | PTE_P;
    for (int i = 0; i < NR_PTE; i ++) {
      upte[i] = 0;
    }
  }
  else {
    upte = (PTE*)PTE_ADDR(data);
  }
  // 这里的upte是一个物理页表的基址，然后我们可以通过PTE_index来找到对应的PTE
  PTE *pte = upte + PTE_index;
  *pte = paddr | PTE_P;
}

void _unmap(_Protect *p, void *va) {
  uintptr_t vaddr = (uintptr_t)va;
  uint32_t PDE_index = PDX(vaddr);
  uint32_t PTE_index = PTX(vaddr);
  PDE *updir = p->ptr;
  PTE data = updir[PDE_index];
  if(!(data & PTE_P)) {
    // 如果没有这个物理页表
    // 就不需要释放了
    return;
  }
  PTE *upte = (PTE*)PTE_ADDR(updir[PDE_index]);
  // 这里的upte是一个物理页表的基址，然后我们可以通过PTE_index来找到对应的PTE
  PTE *pte = upte + PTE_index;
  // 这里的pte是一个物理页表的基址，然后我们可以通过PTE_index来找到对应的PTE
  *pte = 0;
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  return NULL;
}
