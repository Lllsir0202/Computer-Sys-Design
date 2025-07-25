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

// Bytes as unit
size_t get_screen_size() {
  return _screen.width * _screen.height * sizeof(uint32_t);
}

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  // int i;
  // for (i = 0; i < _screen.width * _screen.height; i++) {
  //   fb[i] = pixels[i];
  // }
  for(int i = 0; i < h; i++) {
    for(int j = 0; j < w; j++) {
      if(x+j < _screen.width && y+i < _screen.height){
        fb[(y + i) * _screen.width + (x + j)] = pixels[i * w + j];
      }
    }
  }
}

void _draw_sync() {
}

int _read_key() {
  // TODO()
  // 读取键盘输入
  // 读取输入截码
  uint8_t _status = inb(I8042_STATUS_PORT);
  if(_status & 1) {
    // 如果状态位被置位，那么取数据
    uint32_t data = inl(I8042_DATA_PORT);
    return data;
  } else {
    return _KEY_NONE;
  }
}
