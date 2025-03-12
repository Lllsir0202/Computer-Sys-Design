#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  // 我们需要保存下来表达式，但是由于我们的实现expr时，解析和计算是同步进行的，所以我们只能存下char*
  char* expression;

} WP;

#endif
