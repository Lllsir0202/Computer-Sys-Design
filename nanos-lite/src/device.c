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
}

void fb_write(const void *buf, off_t offset, size_t len) {
  // 用于把buf中的len字节写到屏幕上offset处. 你需要先从offset计算出屏幕上的坐标, 然后调用IOE的_draw_rect()接口.
  Log("Screen width is %d", _screen.width);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
}
