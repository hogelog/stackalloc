TARGET = libstackalloc.a
SOURCES = stackalloc.c
OBJS = $(SOURCES:.c=.o)
TESTS = test test-obstack
AR = ar
CC = gcc
CFLAGS = -g -Wall
LDFLAGS =  

all: deps $(TARGET) $(TESTS)
clean:
	rm -f $(OBJS) $(TARGET) $(TESTS) deps
deps:
	$(CC) -MM $(SOURCES) >deps

$(TARGET): $(OBJS)
	$(AR) rcs libstackalloc.a $^

test: test.c libstackalloc.a
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

test-obstack: test-obstack.c
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $<

include deps
