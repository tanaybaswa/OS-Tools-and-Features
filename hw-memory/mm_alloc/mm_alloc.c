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

  if (first == 1){
    //printf("In first loop\n");
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

    //printf("%x\n", base);

    return &base->ptr;

  }

  Block* b = base;

  while(b != NULL){

    if (b->free == 1){

      //printf("Found free block.\n");
      

      

      if (b->size >= size){
        //printf("%d\n", b->size - size);
        memset(&b->ptr, 0, b->size);
        

        if ((b->size - size) >= (sizeof(Block))){

          //printf("Splitting Blocks.\n");
        
          Block* new = (Block *) ((uint)&b->ptr + size);
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

  //printf("Creating new block.\n");

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

  b = base;

  while(b != NULL){
    //printf("%d\n", (uint) b % 1000000);
    //printf("%d\n", (uint) b->size % 1000000);
    //printf("%d\n", (uint) ((uint) &b->ptr + 99976) % 1000000);

    b = b->next;
  }

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

  //printf("%d\n", (uint)b % 1000000);

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

  b = base;

  while(b != NULL){
    //printf("%d\n", b->free);
    //printf("%d\n", (uint) &b->ptr % 1000000);
    //printf("%d\n", (uint) ((uint) &b->ptr + 99976) % 1000000);

    b = b->next;
  }

}
