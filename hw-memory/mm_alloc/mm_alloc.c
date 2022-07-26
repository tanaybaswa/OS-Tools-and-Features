/*
 * mm_alloc.c
 */

#include "mm_alloc.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

int first = 1;
Block* base;
Block* end;

void* mm_malloc(size_t size) {

  if (size == 0){
    return NULL;
  }

  if (first){
    first = 0;

    
    base = (Block *) sbrk(size + sizeof(Block));
    base->free = 0;
    base->prev = NULL;
    base->next = NULL;
    base->size = size;
    end = base;

    //printf("%x\n", base);

    return &base->ptr;

  }

  Block* b = base;

  while(b != NULL){

    if (b->free){

      if (b->size >= size){
        memset(&b->ptr, 0, b->size);

        if (b->size - size >= sizeof(Block) + sizeof(int)){

          Block* new = (Block *) &b->ptr + size;
          new->free = 1;
          new->prev = b;
          new->next = b->next;
          b->next = new;
          new->size = b->size - sizeof(Block) - size;

          b->size = size;
          b->free = 0;

          return &b->ptr;

        } else {

          b->free = 0;

          return &b->ptr;

        }
      }
    }

    b = b->next;
  }

  Block* new = (Block *) sbrk(size + sizeof(Block));
  new->free = 0;
  new->prev = end;
  end->next = new;
  new->next = NULL;
  new->size = size;

  end = new;

  return &new->ptr;
}

void* mm_realloc(void* ptr, size_t size) {
  //printf("%x\n", b);
  //TODO: Implement realloc

  

  return NULL;
}

void mm_free(void* ptr) {
  if (ptr == NULL){
    return;
  }

  Block* b = (Block *) (ptr - offsetof(Block, ptr));

  //printf("%x\n", b);

  b->free = 1;

  memset(&b->ptr, 0, b->size);
  
  Block* p = b->prev;

  if (p != NULL && p->free == 1){
    p->size = p->size + sizeof(Block) + b->size;
    p->next = b->next;
    memset(&p->ptr, 0, p->size);
    b = p;
  }

  Block* n = b->next;

  if (n != NULL && n->free == 1){
    b->size = b->size + sizeof(Block) + n->size;
    b->next = n->next;
    memset(&b->ptr, 0, b->size);
  }

}
