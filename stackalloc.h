#include <stdlib.h>

typedef struct FObject {
  struct FObject *prev, *next;
} FObject;
typedef struct Frame {
  struct Frame *prevframe;
  void *top;
  size_t index;
  FObject list;
} Frame;
typedef struct Slot {
  void *start;
  void *end;
  size_t size;
} Slot;
typedef struct OStack {
  Slot *slots;
  size_t slotsnum;
  Frame *last;
  void *top;
  size_t index;
} OStack;

#define OSTACK_MINSLOTSIZE 1024

#define obj2fobj(o) (((FObject*)(o))-1)

void *stack_alloc(OStack *s, size_t size);
void *stack_lalloc(OStack *s, size_t size);
Frame *stack_newframe(OStack *s);
Frame *stack_closeframe(OStack *s, Frame *f);
OStack *stack_init(OStack *s);
void stack_close(OStack *s);
Frame *stack_fmove(OStack *s, Frame *to, FObject *fo);

