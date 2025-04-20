#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

// pmem是数组，表示128MB的大内存RAM
uint8_t pmem[PMEM_SIZE];

// ADD in pa4
static inline paddr_t page_translate(vaddr_t addr) {
  // 现在不能够直接使用addr作为物理地址，因为需要进行页表的转换
  uint32_t PTD_index = (addr >> 22) & 0x3FF;
  uint32_t PTE_index = (addr >> 12) & 0x3FF;
  uint32_t offset = addr & 0xFFF;
  // 接下来就是通过这里的PTD_addr和PTE_addr来进行页表的转换
  // 首先读取cr3
  // 找到页表的基地址
  uint32_t directory_base = cpu.cr3;
  // 读取base + PTD_index * sizeof(PTD)
  // 即取到了页表目录项的PTD-> 低12位是乱七八糟的标志位，然后20位>>12加上PTE_index*4即为页表项的地址
  uint32_t data = paddr_read(directory_base + PTD_index * 4, 4);
  PDE PDE_descriptor;
  memcpy(&PDE_descriptor, &data, sizeof(PDE));
  if(PDE_descriptor.present == 0){
    // 页表目录项没有present，说明没有映射
    // 这里的处理方式是直接panic
    panic("Page table descriptor not present");
  }
  PTE PTE_descripor;
  data = paddr_read(PDE_descriptor.page_frame * PAGE_SIZE + PTE_index * 4, 4);
  memcpy(&PTE_descripor, &data, sizeof(PTE));
  if(PTE_descripor.present == 0){
    // 页表项没有present，说明没有映射
    // 这里的处理方式是直接panic
    panic("Page table descriptor not present");
  }
  // 接下来即可拼接地址
  return (PTE_descripor.page_frame << 12) | offset;
  // 这里的offset是低12位，PTE_descripor.page_frame_number是高20位
}

/* Memory accessing interfaces */
// 在调试器的x中非常有用...
uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_num = is_mmio(addr);
  if(mmio_num != -1){
    return mmio_read(mmio_num, addr, len);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio_num = is_mmio(addr);
  if(mmio_num != -1){
    mmio_write(addr, len, data, mmio_num);
    return;
  }
  memcpy(guest_to_host(addr), &data, len);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  // ADD in pa4
  // 首先需要考虑是否出现跨页的情况，其实就是offset+len又没有>PG_SIZE的情况
  uintptr_t offset = addr & (PAGE_MASK);
  if(offset + len > PAGE_SIZE) {
    // 出现跨页，但是在指导书中的说法是只有跨页，但是不一定(?)，可能会有更多页？
    panic("Corss page");
  }
  else{
    paddr_t paddr = page_translate(addr);
    // 这里的addr是虚拟地址，paddr是物理地址
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  paddr_write(addr, len, data);
}
