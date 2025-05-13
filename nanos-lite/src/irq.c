#include "common.h"

extern _RegSet* do_syscall(_RegSet* r);
extern _RegSet* schedule(_RegSet* r, bool keyboard);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:{
      // return do_syscall(r);
      // change in pa4-3
      return do_syscall(r);
      // return schedule(r);
    }
    case _EVENT_TRAP: {
      // Log("[important]: reach here");
      // _RegSet *ret = schedule(r);
      // Log("schedule return %p", ret);
      return schedule(r, false);
    }
    case _EVENT_IRQ_TIME: {
      // Log("[important]:::timer interrupt");
      return schedule(r, false);
    }
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
