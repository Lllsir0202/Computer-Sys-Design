#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  // 我们需要保存下来表达式，但是由于我们的实现expr时，解析和计算是同步进行的，所以我们只能存下char*
  // 此外，我们还需要一个值，记录下此前的值 ->  便于比较表达式的值是否变化
  // 这里先加一个init的bool类型，来记录是否初始化了，不过很可能用不上 -> 开始就去副一下即可
  char* expression;
  uint32_t old_value;
} WP;

#endif
