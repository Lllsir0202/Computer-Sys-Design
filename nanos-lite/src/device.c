#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));

// 修改返回值，保证其能够记录下len
int dispinfo_read(void *buf, off_t offset, size_t len) {
  // 用于把字符串dispinfo中offset开始的len字节写到buf中.
  if(dispinfo[offset] == '\0') {
    return 0;
  }
  int i;
  for(i = offset; i < len; i++){
    if(dispinfo[i] == '\n' || dispinfo[i] == '\0'){
      break;
    }
  }
  Log("len is %d", i+1);
  memcpy(buf, dispinfo + offset, i+1);
  return i+1;
}

void fb_write(const void *buf, off_t offset, size_t len) {
  // 用于把buf中的len字节写到屏幕上offset处. 你需要先从offset计算出屏幕上的坐标, 然后调用IOE的_draw_rect()接口.
  int x = (offset / 4) % _screen.width;
  int y = (offset / 4) / _screen.width;
  // 计算出坐标

  // 由于len不一定能一行结束，所以需要考虑多行的情况
  // 像素数目
  int pixels = len / 4;

  if(pixels <= _screen.width - x) {
    _draw_rect((uint32_t*)buf, x, y, pixels, 1);
    return;
  }
  // 第一行不完整部分的像素数
  int first_row_pixels = _screen.width - x;
  
  // 绘制第一行不完整部分
  _draw_rect((uint32_t*)buf, x, y, first_row_pixels, 1);
  
  // 剩余需要绘制的像素
  int remaining_pixels = pixels - first_row_pixels;
  
  // 绘制中间完整行
  int full_rows = remaining_pixels / _screen.width;
  if (full_rows > 0) {
    _draw_rect((uint32_t*)buf + first_row_pixels, 0, y + 1, _screen.width, full_rows);
  }
  
  // 绘制最后一行不完整部分
  int last_row_pixels = remaining_pixels % _screen.width;
  if (last_row_pixels > 0) {
    _draw_rect((uint32_t*)buf + first_row_pixels + full_rows * _screen.width, 
               0, y + 1 + full_rows, last_row_pixels, 1);
  }
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH : %d\nHEIGHT : %d\0", _screen.width, _screen.height);
  printf("%s\n",dispinfo);
}
