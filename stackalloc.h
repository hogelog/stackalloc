#include <stdlib.h>

typedef struct FObject {
  void *obj;
  struct FObject *next;
} FObject;
typedef struct Frame {
  struct Frame *prevframe;
  void *top;
  size_t index;
  FObject *list;
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
  Frame cur;
} OStack;

#define OSTACK_MINSLOTSIZE 1024

void *stack_alloc(OStack *s, size_t size);
void *stack_lalloc(OStack *s, size_t size);
Frame *stack_newframe(OStack *s);
Frame *stack_closeframe(OStack *s);
OStack *stack_init(OStack *s);
void stack_close(OStack *s);

