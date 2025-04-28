#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  if (current->cur_brk == 0) {
    current->cur_brk = current->max_brk = new_brk;
  }
  else {
    if (new_brk > current->max_brk) {
      // TODO: map memory region [current->max_brk, new_brk)
      // into address space current->as

      // 把current->as.area.start的虚拟空间映射到物理空间
      // 计算需要映射的物理页面范围
      uint32_t p_start = PGROUNDUP(current->max_brk);
      uint32_t p_end = PGROUNDUP(new_brk);
      
      // 获取虚拟地址起始点
      // 这里假设堆区域从current->as.area.start开始
      // 偏移量等于physical_brk相对于起始物理地址的距离
      void *v_start = (void *)((uintptr_t)current->as.area.start + 
                              (p_start - (uintptr_t)_heap.start));
      
      // 为每个物理页建立映射
      for (uint32_t p_addr = p_start, v_addr = (uintptr_t)v_start; 
           p_addr < p_end; 
           p_addr += PGSIZE, v_addr += PGSIZE) {
        // 将虚拟地址v_addr映射到物理地址p_addr
        _map(&current->as, (void *)v_addr, (void *)p_addr);
      }

      current->max_brk = new_brk;
    }

    current->cur_brk = new_brk;
  }

  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
