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
  void *slot;
  void *last;
  size_t size;
} Slot;
typedef struct Stack {
  Slot *slots;
  size_t slotsnum;
  Frame *last;
  Frame cur;
} Stack;

#define STACK_MINSLOTSIZE 1024

void *stack_alloc(Stack *s, size_t size);
void *stack_lalloc(Stack *s, size_t size);
Frame *stack_newframe(Stack *s);
Frame *stack_closeframe(Stack *s);
Stack *stack_init(Stack *s);
void stack_close(Stack *s);
