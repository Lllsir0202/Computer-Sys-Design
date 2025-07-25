#include "common.h"

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE
#define HAS_PTE

void init_mm(void);
void init_ramdisk(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
uint32_t loader(_Protect *, const char *);
void load_prog(const char *filename);

int main() {
#ifdef HAS_PTE
  init_mm();
#endif

  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  init_ramdisk();

  init_device();

#ifdef HAS_ASYE
  Log("Initializing interrupt/exception handler...");
  init_irq();
#endif

  init_fs();

  // Log("finish fs");

  // uint32_t entry = loader(NULL, "/bin/pal");
  // ((void (*)(void))entry)();
  load_prog("/bin/pal");
  
  // Add in pa4-3
  load_prog("/bin/hello");

  // Add in pa4-3
  load_prog("/bin/videotest");

  // Add in pa4-2
  _trap();

  panic("Should not reach here");
}
