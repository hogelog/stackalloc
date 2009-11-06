#include <stdio.h>
#include <assert.h>
#include "stackalloc.h"

static int *new_int(OStack *s, const int i) {
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
static Array *new_array(OStack *s, size_t size, size_t len) {
  Array *a = stack_lalloc(s, sizeof(Array)+size*(len-1));
  a->size = size;
  a->length = len;
  return a;
}
static Array *new_range(OStack *s, int start, int end) {
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
  OStack s_;
  OStack *s = &s_;
  int i, j;
  stack_init(s);
  for(i=0;i<100000;++i) {
    Frame *f = stack_newframe(s);
    int *sum = new_int(s, 1);
    Array *a = new_range(s, 1, 1024);
    *sum = 1;
    for(j=0;j<1024;++j) {
      int *p = array_elem(a, j);
      *sum *= *p;
      //printf("%2d(%p) = %7d(%p)\n", *p, p, *sum, sum);
    }
    stack_closeframe(s, f);
  }
  stack_close(s);
}
int main(int argc, char **argv) {
  test01();
  return 0;
}
