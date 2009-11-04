#include <stdlib.h>

typedef struct Frame {
  struct Frame *prevframe;
  void *top;
  size_t index;
} Frame;
typedef struct Slot {
  void *slot;
  void *last;
  size_t size;
} Slot;
typedef struct Stack {
  Slot *slots;
  size_t slotsnum;
  Frame *lastframe;
  void *top;
  size_t index;
} Stack;

#define STACK_MINSLOTSIZE 1024

void *stack_alloc(Stack *s, size_t size);
Frame *stack_newframe(Stack *s);
Frame *stack_closeframe(Stack *s);
Stack *stack_init(Stack *s);
void stack_close(Stack *s);
