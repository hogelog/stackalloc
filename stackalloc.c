#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
static Slot *addslot(Stack *s, size_t slotsize) {
  size_t n = s->slotsnum;
  s->slots = realloc(s->slots, (n+1)*sizeof(Slot));
  s->slots[n].slot = malloc(slotsize);
  s->slots[n].last = (void*)((char*)s->slots[n].slot + slotsize);
  s->slotsnum = n+1;
  return s->slots[n].slot;
}
static size_t stack_grow(Stack *s) {
  size_t nextindex = s->index + 1;
  size_t slotsnum = s->slotsnum;
  if (nextindex >= slotsnum) {
    int i;
    size_t newsize = 0;
    Slot *newslot;
    for(i=0;i<slotsnum;++i) {
      Slot *slot = &s->slots[i];
      newsize += (char*)slot->last - (char*) slot->slot;
    }
    addslot(s, newsize);
  }
  s->index = nextindex;
  s->top = s->slots[nextindex].slot;
  return s->index;
}
void *stack_alloc(Stack *s, size_t size) {
  void *new = s->top;
  void *newtop = (void*)((char*)new + size);
  Slot *curslot = &s->slots[s->index];
  if (newtop > curslot->last) {
    stack_grow(s);
    return stack_alloc(s, size);
  }
  s->top = newtop;
  return new;
}
Frame *stack_newframe(Stack *s) {
  void *ptop = s->top;
  size_t pindex = s->index;
  Frame *f = stack_alloc(s, sizeof(Frame));
  if (f) {
    f->prevframe = s->lastframe;
    f->top = ptop;
    f->index = pindex;
    s->lastframe = f;
  }
  return f;
}
Frame *stack_closeframe(Stack *s) {
  Frame *f = s->lastframe;
  s->top = f->top;
  s->index = f->index;
  s->lastframe = f->prevframe;
  return f;
}
Stack *stack_init(Stack *s) {
  s->slots = NULL;
  s->slotsnum = 0;
  s->lastframe = NULL;
  addslot(s, STACK_MINSLOTSIZE);
  s->index = 0;
  s->lastframe = NULL;
  s->top = s->slots[0].slot;
  s->index = 0;
  return s;
}
void stack_close(Stack *s) {
  int i;
  for(i=s->slotsnum-1;i>=0;--i) {
    free(s->slots[i].slot);
  }
  free(s->slots);
  s->slots = NULL;
}
static int *new_int(Stack *s, const int i) {
  int *dst = stack_alloc(s, sizeof(int));
  *dst = i;
  return dst;
}
typedef struct Array {
  size_t size;
  size_t length;
  char buffer[1];
} Array;
#define array_elem(a, i) ((void*)&((a)->buffer[(a)->size*(i)]))
static Array *new_array(Stack *s, size_t size, size_t len) {
  Array *a = stack_alloc(s, sizeof(Array)+size*(len-1));
  a->size = size;
  a->length = len;
  return a;
}
static Array *new_range(Stack *s, int start, int end) {
  size_t len = end - start + 1;
  Array *a = new_array(s, sizeof(int), len);
  int i;
  for(i=0;i<len;++i) {
    int *p = array_elem(a, i);
    *p = i+start;
  }
  return a;
}
void test01() {
  Stack s_;
  Stack *s = &s_;
  int i, j;
  void *p;
  stack_init(s);
  for(i=0;stack_newframe(s) && i<10000;++i) {
    int *sum = new_int(s, 1);
    Array *a = new_range(s, 1, 1024);
    for(j=0;stack_newframe(s) && j<1024;++j) {
      int *p = array_elem(a, j);
      *sum *= *p;
      printf("%2d(%p) = %7d(%p)\n", *p, p, *sum, sum);
      stack_closeframe(s);
    }
    stack_closeframe(s);
  }
  stack_close(s);
}
int main(int argc, char **argv) {
  test01();
  return 0;
}
