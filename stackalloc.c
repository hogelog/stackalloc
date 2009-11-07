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
  void *new = s->top;
  void *newtop = (void*)((char*)new + size);
  Slot *curslot = &s->slots[s->index];
  while (newtop > curslot->end) {
    ++s->index;
    if (s->index == s->slotsnum)
      stack_grow(s);
    s->top = s->slots[s->index].start;
    curslot = &s->slots[s->index];
    new = curslot->start;
    newtop = (void*)((char*)new + size);
  }
  s->top = newtop;
  return new;
}
#define setflist(f,fo) { \
    (fo)->prev = &(f)->list; \
    (fo)->next = (f)->list.next; \
    if ((f)->list.next) (f)->list.next->prev = (fo); \
    (f)->list.next = (fo); \
  }
void *stack_lalloc(OStack *s, size_t size) {
  FObject *fo;
  Frame *f = s->last;
  if (!f) return NULL;
  fo = malloc(sizeof(FObject)+size);
  setflist(f, fo);
  return (void*)(fo + 1);
}
Frame *stack_newframe(OStack *s) {
  void *ptop = s->top;
  size_t pindex = s->index;
  Frame *f = stack_alloc(s, sizeof(Frame));
  f->prevframe = s->last;
  f->top = ptop;
  f->index = pindex;
  f->list.prev = NULL;
  f->list.next = NULL;
  s->last = f;
  return f;
}
Frame *stack_closeframe(OStack *s, Frame *f) {
  Frame *last = s->last;
  while (last != f->prevframe) {
    FObject *list = &last->list;
    while (list->next!=NULL) {
      FObject *o = list->next;
      list->next = list->next->next;
      free(o);
    }
    last = last->prevframe;
  }
  s->top = f->top;
  s->index = f->index;
  s->last = f->prevframe;
  return f;
}
OStack *stack_init(OStack *s) {
  s->slots = malloc(sizeof(Slot));
  s->slots[0].start = malloc(OSTACK_MINSLOTSIZE);
  s->slots[0].end = (void*)((char*)s->slots[0].start + OSTACK_MINSLOTSIZE);
  s->slotsnum = 1;
  s->last = NULL;
  s->top = s->slots[0].start;
  s->index = 0;
  return s;
}
void stack_close(OStack *s) {
  int i;
  while (s->last)
    stack_closeframe(s, s->last);
  for (i=s->slotsnum-1;i>=0;--i)
    free(s->slots[i].start);
  free(s->slots);
  s->slots = NULL;
}
Frame *stack_fmove(OStack *s, Frame *to, FObject *fo) {
  if (fo->next) fo->next->prev = fo->prev;
  fo->prev->next = fo->next;
  setflist(to, fo);
  return to;
}
