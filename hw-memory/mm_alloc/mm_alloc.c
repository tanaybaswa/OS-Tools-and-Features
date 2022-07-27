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
  if (size == 0){
    return NULL;
  }

  if (first == 1){
 
    first = 0;

    
    base = (Block *) sbrk(size + sizeof(Block));
    if (base == -1){
      return NULL;
    }
    base->free = 0;
    base->prev = NULL;
    base->next = NULL;
    base->size = size;
    end = base;

    return &base->ptr;

  }

  Block* b = base;

  while(b != NULL){

    if (b->free == 1){

      if (b->size >= size){

        if ((b->size - size) >= (sizeof(Block))){

          Block* new = (Block *) ((void *)&b->ptr + size);

          new->free = 1;
          new->size = b->size - sizeof(Block) - size;
          new->prev = b;
          new->next = b->next;
          if (b->next != NULL){
            b->next->prev = new;
          }
          b->next = new;
          if (new->next == NULL){
            end = new;
          }

          b->size = size;
          b->free = 0;


          return &b->ptr;

        } else {

          b->free = 0;
          //b->size = size;

          return &b->ptr;
          
        }
      }
    }
    b = b->next;
  }


  Block* new = (Block *) sbrk(size + sizeof(Block));
  if (new == -1){
    return NULL;
  }
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

  b->free = 1;

  memset(&b->ptr, 0, b->size);
  
  Block* p = b->prev;

  if (p != NULL && p->free == 1){
    p->size = p->size + sizeof(Block) + b->size;
    p->next = b->next;
    if (b->next != NULL){
      b->next->prev = p;
    }
    memset(&p->ptr, 0, p->size);
    b = p;
  }

  Block* n = b->next;

  if (n != NULL && n->free == 1){
    b->size = b->size + sizeof(Block) + n->size;
    b->next = n->next;
    if (n->next != NULL){
      n->next->prev = b;
    }

    if (n->next == NULL){
      end = b;
    }
    memset(&b->ptr, 0, b->size);
  }
}
