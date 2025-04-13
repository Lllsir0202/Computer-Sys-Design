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

void dispinfo_read(void *buf, off_t offset, size_t len) {
  // 用于把字符串dispinfo中offset开始的len字节写到buf中.
  if(offset + len > 128) {
    len = 128 - offset;
  }
  memcpy(buf, dispinfo + offset, len);
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
  // 不能一行画完的话
  _draw_rect((uint32_t*)buf, x, y, _screen.width-x, 1);

  // 完整的行
  int full_rows = (pixels - (_screen.width-x)) / _screen.width;
  int last_one = (pixels - (_screen.width-x)) - full_rows * _screen.width;

  // 绘制整行
  if(full_rows > 0){
    _draw_rect((uint32_t*)buf, 0, y+full_rows+1, _screen.width, full_rows);
  }
  _draw_rect((uint32_t*)buf, 0, y+full_rows+2,last_one, 1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH : %d\nHEIGHT : %d", _screen.width, _screen.height);
  printf("%s\n",dispinfo);
}
