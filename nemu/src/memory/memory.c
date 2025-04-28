#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

// ADD in pa4
#define CR0_PG    0x80000000  // Paging
#define PAGE_SHIFT 12 // shift 2^12

// pmem是数组，表示128MB的大内存RAM
uint8_t pmem[PMEM_SIZE];

// ADD in pa4
static inline paddr_t page_translate(vaddr_t addr, bool write) {
  // ++cnt;
  // 现在不能够直接使用addr作为物理地址，因为需要进行页表的转换
  uint32_t PDE_index = (addr >> 22) & 0x3FF;
  uint32_t PTE_index = (addr >> 12) & 0x3FF;
  uint32_t offset = addr & 0xFFF;
  // 接下来就是通过这里的PTD_addr和PTE_addr来进行页表的转换
  // 首先读取cr3
  // 找到页表的基地址
  uint32_t directory_base = cpu.cr3;
  // 读取base + PTD_index * sizeof(PTD)
  // 即取到了页表目录项的PTD-> 低12位是乱七八糟的标志位，然后20位>>12加上PTE_index*4即为页表项的地址
  uint32_t data = paddr_read(directory_base + PDE_index * sizeof(PDE), sizeof(PDE));
  PDE PDE_descriptor;
  memcpy(&PDE_descriptor, &data, sizeof(PDE));
  if(!PDE_descriptor.present && !write){
    // 页表目录项没有present，说明没有映射
    // 这里的处理方式是直接panic
    // Log("cnt is %d", cnt);
    panic("Page entry descriptor not present");
  }else if(!PDE_descriptor.present && write){
    // Log("cnt is %d", cnt);
    panic("error in write(PDE)");
  }
  PTE PTE_descriptor;
  data = paddr_read((PDE_descriptor.page_frame << PAGE_SHIFT) + PTE_index * sizeof(PTE), sizeof(PTE));
  memcpy(&PTE_descriptor, &data, sizeof(PTE));
  if(!PTE_descriptor.present && !write){
    // 页表项没有present，说明没有映射
    // 这里的处理方式是直接panic
    // Log("cnt is %d", cnt);
    panic("Page table descriptor not present");
  } else if(!PTE_descriptor.present && write){
    // Log("cnt is %d", cnt);
    panic("error in write(PTE)");
  }
  // 接下来即可拼接地址
  // Log("paddr is %08x", (PTE_descriptor.page_frame << 12) | offset);
  return (PTE_descriptor.page_frame << 12) | offset;
  // 这里的offset是低12位，PTE_descripor.page_frame_number是高20位
}

/* Memory accessing interfaces */
// 在调试器的x中非常有用...
uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_num = is_mmio(addr);
  if(mmio_num != -1){
    // Log("here");
    return mmio_read(mmio_num, addr, len);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio_num = is_mmio(addr);
  if(mmio_num != -1){
    // Log("here");
    mmio_write(addr, len, data, mmio_num);
    return;
  }
  memcpy(guest_to_host(addr), &data, len);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  // ADD in pa4
  // 其实需要考虑下是否开启了保护机制
  if(cpu.cr0 & CR0_PG) {
    // 首先需要考虑是否出现跨页的情况，其实就是offset+len又没有>PG_SIZE的情况
    if(addr == 0x8048a1c){
      Log("here");
    }
    uintptr_t offset = addr & (PAGE_MASK);
    if(offset + len > PAGE_SIZE) {
      // 出现跨页，但是在指导书中的说法是只有跨页，但是不一定(?)，可能会有更多页？-> 不会有很多页的
      // 第一页先读取
      if(addr == 0x8048a1c){
        Log("here2");
      }
      int first_page = PAGE_SIZE - offset;
      // Log("first_page is %d", first_page);
      paddr_t paddr = page_translate(addr, false);
      uint32_t data = paddr_read(paddr, first_page);
      // 读取第二页
      int second_page = len - first_page;
      // Log("second_page is %d", second_page);
      paddr = page_translate(addr + first_page, false);
      uint32_t data2 = paddr_read(paddr, second_page);
      // 这里的data是第一页的数据，data2是第二页的数据
      // NOTE!!!->小端序
      return data2 << (8 * first_page) | data;
    }else {
      paddr_t paddr = page_translate(addr, false);
      // 这里的addr是虚拟地址，paddr是物理地址
      return paddr_read(paddr, len);
    }
  } else {
    return paddr_read(addr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(cpu.cr0 & CR0_PG) {
    uintptr_t offset = addr & (PAGE_MASK);
    if(offset + len > PAGE_SIZE) {
      // 出现跨页，但是在指导书中的说法是只有跨页，但是不一定(?)，可能会有更多页？-> 不会有很多页的
      // 第一页先找到
      int first_page = PAGE_SIZE - offset;
      uint32_t data1 = data & (~0u >> (first_page << 3));
      // Log("data1 is %x", data1);
      // Log("first_page is %d", first_page);
      paddr_t paddr = page_translate(addr, false);
      // 写入第一页
      paddr_write(paddr, first_page, data1);
      // 读取第二页
      int second_page = len - first_page;
      uint32_t data2 = data >> (first_page << 3);
      // Log("data2 is %x", data2);
      // Log("second_page is %d", second_page);
      paddr = page_translate(addr + first_page, false);
      paddr_write(paddr, second_page, data2);
    }else {
      paddr_t paddr = page_translate(addr, true);
      // 这里的addr是虚拟地址，paddr是物理地址
      paddr_write(paddr, len, data);
    }
  }else {
    paddr_write(addr, len, data);
  }
}
