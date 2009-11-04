#include "stackalloc.h"

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
