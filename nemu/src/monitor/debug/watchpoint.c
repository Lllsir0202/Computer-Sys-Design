#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
// 用来实现wp的添加
WP* new_up(){
  // 首先我们需要在free中找一个，然后返回其的WP*即可
  if(free_ == NULL){
    // 说明没有了
    panic("There is no free watchpoint");
    assert(0);
  }
  // 这里说明free_是有空闲的
  WP* new_wp = free_;
  free_ = free_->next;
  if(head == NULL){
    head = new_wp;
  }else{
    WP* p = head;
    while(p->next != NULL){
      p = p->next;
    }
    // 这时p的next是NULL
    p->next = new_wp;
    new_wp->next = NULL;
  }
  return new_wp;
}

void free_up(WP* wp){
  assert(wp != NULL);
  WP* p = head;
  assert(p != NULL);
  if(head->NO == wp->NO){
    // 说明头部就是
    if(free_ == NULL){
      free_ = wp;
    }else{
      WP* f_p = free_;
      while(f_p->next != NULL){
        f_p = f_p->next;
      }
      f_p->next = wp;
    }
    // 接下来就是把它从head中去除
    head = NULL;
    return ;
  }
  while(p->next != NULL){
    // 当p不是尾部时，我们去检查即可
    if(p->next->NO == wp->NO){
      if(free_ == NULL){
        free_ = wp;
      }else{
        WP* f_p = free_;
        while(f_p->next != NULL){
          f_p = f_p->next;
        }
        f_p->next = wp;
      }
      p->next = wp->next;
      wp->next = NULL;
      return ;
    }
  }
}
