#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard

#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64
#define I8042_STATUS_HASKEY_MASK 0x1
#define KEYBOARD_IRQ 1
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  // TODO()
  // 获取时间、减去boot_time即可
  return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int i;
  for (i = 0; i < _screen.width * _screen.height; i++) {
    fb[i] = i;
  }
}

void _draw_sync() {
}

int _read_key() {
  // TODO()
  // 读取键盘输入
  // 读取输入截码
  uint32_t key = inl(I8042_DATA_PORT);
  if(key == 0) {
    return _KEY_NONE;
  }
  // 如果有按键按下，那么写入状态寄存器1s
  outb(I8042_STATUS_PORT, I8042_STATUS_HASKEY_MASK);
  return key + 0x8000;
}
