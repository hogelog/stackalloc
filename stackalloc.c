#include "stackalloc.h"

static Slot *addslot(OStack *s, size_t slotsize) {
  size_t n = s->slotsnum;
  s->slots = realloc(s->slots, (n+1)*sizeof(Slot));
  s->slots[n].start = malloc(slotsize);
  s->slots[n].end = (void*)((char*)s->slots[n].start + slotsize);
  s->slotsnum = n+1;
  return s->slots[n].start;
}
static size_t stack_grow(OStack *s) {
  size_t slotsnum = s->slotsnum;
  int i;
  size_t newsize = 0;
  for(i=0;i<slotsnum;++i) {
    Slot *slot = &s->slots[i];
    newsize += (char*)slot->end - (char*) slot->start;
  }
  addslot(s, newsize);
  return s->slotsnum;
}
void *stack_alloc(OStack *s, size_t size) {
  void *new = s->cur.top;
  void *newtop = (void*)((char*)new + size);
  Slot *curslot = &s->slots[s->cur.index];
  while (newtop > curslot->end) {
    ++s->cur.index;
    if (s->cur.index == s->slotsnum)
      stack_grow(s);
    s->cur.top = s->slots[s->cur.index].start;
    curslot = &s->slots[s->cur.index];
    new = curslot->start;
    newtop = (void*)((char*)new + size);
  }
  s->cur.top = newtop;
  return new;
}
void *stack_lalloc(OStack *s, size_t size) {
  Frame *f = s->last ? s->last : &s->cur;
  FObject *fo = stack_alloc(s, sizeof(FObject));
  fo->next = f->list;
  fo->obj = malloc(size);
  f->list = fo;
  return fo->obj;
}
Frame *stack_newframe(OStack *s) {
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
Frame *stack_closeframe(OStack *s, Frame *f) {
  Frame *last = s->last;
  while (last != f->prevframe) {
    while (last->list!=NULL) {
      void *o = last->list->obj;
      last->list = last->list->next;
      free(o);
    }
    last = last->prevframe;
  }
  s->cur.top = f->top;
  s->cur.index = f->index;
  s->last = f->prevframe;
  return f;
}
OStack *stack_init(OStack *s) {
  s->slots = malloc(sizeof(Slot));
  s->slots[0].start = malloc(OSTACK_MINSLOTSIZE);
  s->slots[0].end = (void*)((char*)s->slots[0].start + OSTACK_MINSLOTSIZE);
  s->slotsnum = 1;
  s->last = NULL;
  s->cur.top = s->slots[0].start;
  s->cur.index = 0;
  s->cur.list = NULL;
  return s;
}
void stack_close(OStack *s) {
  int i;
  while (s->last)
    stack_closeframe(s, s->last);
  while (s->cur.list!=NULL) {
    void *o = s->cur.list->obj;
    s->cur.list = s->cur.list->next;
    free(o);
  }
  for (i=s->slotsnum-1;i>=0;--i)
    free(s->slots[i].start);
  free(s->slots);
  s->slots = NULL;
}
