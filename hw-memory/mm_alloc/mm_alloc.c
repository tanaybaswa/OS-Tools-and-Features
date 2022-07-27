/*
 * mm_alloc.c
 */

#include "mm_alloc.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>


void* mm_malloc(size_t size) {
  return NULL;
}

void* mm_realloc(void* ptr, size_t size) {
  
  if (ptr == NULL){
    return mm_malloc(size);
  }

  if (size == 0){
    mm_free(ptr);
    return NULL;
  }

  return NULL;
}

void mm_free(void* ptr) {
  return;
}