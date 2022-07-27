/*
 * mm_alloc.c
 */

#include "mm_alloc.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

typedef struct block{
    size_t size;
    int free;
    struct block *next;
    struct block *prev;
    char ptr[];

} Block ;

int first = 1;
Block* base;
Block* end;

void* mm_malloc(size_t size) {
  return NULL;
}

void* mm_realloc(void* ptr, size_t size) {
  return NULL;
}

void mm_free(void* ptr) {
  return NULL;
}
