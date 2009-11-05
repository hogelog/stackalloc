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
  size_t nextindex = s->cur.index + 1;
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
  s->cur.index = nextindex;
  s->cur.top = s->slots[nextindex].slot;
  return s->cur.index;
}
void *stack_alloc(Stack *s, size_t size) {
  void *new = s->cur.top;
  void *newtop = (void*)((char*)new + size);
  Slot *curslot = &s->slots[s->cur.index];
  if (newtop > curslot->last) {
    stack_grow(s);
    return stack_alloc(s, size);
  }
  s->cur.top = newtop;
  return new;
}
void *stack_lalloc(Stack *s, size_t size) {
  Frame *f = s->last ? s->last : &s->cur;
  FObject *fo = stack_alloc(s, sizeof(FObject));
  fo->next = f->list;
  fo->obj = malloc(size);
  f->list = fo;
  return fo->obj;
}
Frame *stack_newframe(Stack *s) {
  void *ptop = s->cur.top;
  size_t pindex = s->cur.index;
  Frame *f = stack_alloc(s, sizeof(Frame));
  f->prevframe = s->last;
  f->top = ptop;
  f->index = pindex;
  f->list = NULL;
  s->last = f;
  return f;
}
Frame *stack_closeframe(Stack *s) {
  Frame *last = s->last;
  while(last->list!=NULL) {
    void *o = last->list->obj;
    last->list = last->list->next;
    free(o);
  }
  s->cur = *last;
  s->last = last->prevframe;
  return last;
}
Stack *stack_init(Stack *s) {
  s->slots = NULL;
  s->slotsnum = 0;
  s->last = NULL;
  addslot(s, STACK_MINSLOTSIZE);
  s->last = NULL;
  s->cur.top = s->slots[0].slot;
  s->cur.index = 0;
  s->cur.list = NULL;
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
