#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  // Log("reach here1");
  _protect(&pcb[i].as);
  // Log("protect %p", pcb[i].as.ptr);
  // Log("here");
  uintptr_t entry = loader(&pcb[i].as, filename);
  // Log("entry = %p", entry);
  // Log("reach here2");

  // TODO: remove the following three lines after you have implemented _umake()
  // _switch(&pcb[i].as);
  // Log("switch to new process %p", pcb[i].as);
  // current = &pcb[i];
  // Log("switch to new process %p", current->as.ptr);
  // ((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);
  // Log("reach here3");

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
  Log("pcb[i].tf = %p", pcb[i].tf);
}

// Add in pa4-3
// 默认为pal
static PCB *current_game = NULL;
_RegSet* schedule(_RegSet *prev, bool keyboard) {
  // save the context pointer
  current->tf = prev;
  // Log("before switch to new process %p", current->as.ptr);

  // always select pcb[0] as the new process
  // current = &pcb[0];
  // Change in pa4-3
  // current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  if(current_game == NULL) {
    current_game = &pcb[0];
  }
  if(keyboard) {
    current_game = (current_game == &pcb[0] ? &pcb[2] : &pcb[0]);
  }
  // 优先级调度
  static uint32_t cnt = 0;
  if(cnt == 0) {
    current = current_game;
  }
  cnt++;
  if(cnt == 1000) {
    current = &pcb[1];
    cnt = 0;
  }

  // TODO: switch to the new address space,
  // then return the new context
  _switch(&current->as);
  // Log("switch to new process %p", current->as.ptr);

  // Log("current->tf is %p", current->tf);
  return current->tf;
}
