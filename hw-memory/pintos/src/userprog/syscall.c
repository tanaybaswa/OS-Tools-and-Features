#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/palloc.h"
#include "pagedir.h"


static void syscall_handler(struct intr_frame*);
static void* syscall_sbrk(intptr_t increment);
static void* alloc(intptr_t increment);
static void* dealloc(intptr_t increment);

void syscall_init(void) { intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall"); }

void syscall_exit(int status) {
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit();
}

/*
 * This does not check that the buffer consists of only mapped pages; it merely
 * checks the buffer exists entirely below PHYS_BASE.
 */
static void validate_buffer_in_user_region(const void* buffer, size_t length) {
  uintptr_t delta = PHYS_BASE - buffer;
  if (!is_user_vaddr(buffer) || length > delta)
    syscall_exit(-1);
}

/*
 * This does not check that the string consists of only mapped pages; it merely
 * checks the string exists entirely below PHYS_BASE.
 */
static void validate_string_in_user_region(const char* string) {
  uintptr_t delta = PHYS_BASE - (const void*)string;
  if (!is_user_vaddr(string) || strnlen(string, delta) == delta)
    syscall_exit(-1);
}

static int syscall_open(const char* filename) {
  struct thread* t = thread_current();
  if (t->open_file != NULL)
    return -1;

  t->open_file = filesys_open(filename);
  if (t->open_file == NULL)
    return -1;

  return 2;
}

static int syscall_write(int fd, void* buffer, unsigned size) {
  struct thread* t = thread_current();
  if (fd == STDOUT_FILENO) {
    putbuf(buffer, size);
    return size;
  } else if (fd != 2 || t->open_file == NULL)
    return -1;

  return (int)file_write(t->open_file, buffer, size);
}

static int syscall_read(int fd, void* buffer, unsigned size) {
  struct thread* t = thread_current();
  if (fd != 2 || t->open_file == NULL)
    return -1;

  return (int)file_read(t->open_file, buffer, size);
}

static void syscall_close(int fd) {
  struct thread* t = thread_current();
  if (fd == 2 && t->open_file != NULL) {
    file_close(t->open_file);
    t->open_file = NULL;
  }
}

static void* syscall_sbrk(intptr_t increment) {
  struct thread* t = thread_current();

  uint32_t start = t->start_of_heap;
  uint32_t sbreak = t->segment_break;
  bool first = false;
  int current;
  int next;

  if (increment == 0){
    //printf("%x  sbreak\n\n", sbreak);
    //printf("%x  ptalloc\n\n", ptalloc);
    return (void *) sbreak;
  }

  if (start == sbreak){
    first = true;
  }

  if (first){
    if (increment < 0){
      return (void *) -1;
    }
    void* firstpage = palloc_get_page(PAL_ZERO | PAL_USER);
    if (firstpage == NULL){
      return (void *) -1;
    }
    pagedir_set_page(t->pagedir, (void *) start, firstpage, true);
  }


  current = pg_no((void *) sbreak);
  next = pg_no((void *) (sbreak + increment));

  if (current == next){

    if (increment < 0){
      if (pg_ofs((void *) (sbreak + increment)) == 0){

        //printf("%x  sbreak\n\n", sbreak);

        void* page = pagedir_get_page(t->pagedir, (void *) (sbreak + increment));
        pagedir_clear_page(t->pagedir, (void *) (sbreak + increment));
        palloc_free_page(page);

      }
    }
    
    t->previous_break = sbreak;
    t->segment_break = sbreak + increment;
    return (void *) t->previous_break;

  }

  if (next > current){
    return alloc(increment);
  }

  return dealloc(increment);

}

static void* alloc(intptr_t increment) {
  struct thread* t = thread_current();

  uint32_t start = t->start_of_heap;
  uint32_t sbreak = t->segment_break;
  bool first = false;
  bool fail = false;
  uint32_t ptalloc;
  int j;
  int current;
  int next;

  if (start == sbreak){
    ptalloc = PGSIZE + sbreak;
    first = true;
  } else {
    ptalloc = (uint32_t) pg_round_up((void *) sbreak);
  }

  current = pg_no((void *) sbreak);
  next = pg_no((void *) (sbreak + increment));


  for(int i = 0; i < next - current; i++){
    void* page = palloc_get_page(PAL_ZERO | PAL_USER);
    if (page == NULL){
      j = i;
      fail = true;
      break;
    }
    
    if (!pagedir_set_page(t->pagedir, (void *) ptalloc, page, true)){
      j = i;
      fail = true;
      break;
    }
    

    //pagedir_set_page(t->pagedir, (void *) ptalloc, page, true);

    ptalloc += PGSIZE;
  }

  if (fail){
    if (first){
      void* page = pagedir_get_page(t->pagedir, (void *) start);
      pagedir_clear_page(t->pagedir, (void *) start);
      palloc_free_page(page);
    }

    ptalloc -= PGSIZE;

    for (int k = 0; k < j; k++){
      void* page = pagedir_get_page(t->pagedir, (void *) ptalloc);
      pagedir_clear_page(t->pagedir, (void *) ptalloc);
      palloc_free_page(page);
      ptalloc -= PGSIZE;
    }

    return (void *) -1;
  }


  t->previous_break = sbreak;
  t->segment_break = sbreak + increment;

  return (void *) t->previous_break;
}

static void* dealloc(intptr_t increment) {
  struct thread* t = thread_current();

  uint32_t start = t->start_of_heap;
  uint32_t sbreak = t->segment_break;
  uint32_t ptalloc;
  int current;
  int next;

 
  if (start == sbreak || sbreak + increment < start){
    return (void *) -1;
  } else {
    ptalloc = (uint32_t) pg_round_down((void *) sbreak);
  }

  current = pg_no((void *) sbreak);
  next = pg_no((void *) (sbreak + increment));

  for(int i = 0; i < current - next; i++){
    void* page = pagedir_get_page(t->pagedir, (void *) ptalloc);
    pagedir_clear_page(t->pagedir, (void *) ptalloc);
    palloc_free_page(page);
    ptalloc -= PGSIZE;
  }

  if (pg_ofs((void *) sbreak + increment) == 0){
    void* page = pagedir_get_page(t->pagedir, (void *) ptalloc);
    pagedir_clear_page(t->pagedir, (void *) ptalloc);
    palloc_free_page(page);
  } 

  t->previous_break = sbreak;
  t->segment_break = sbreak + increment;

  return (void *) t->previous_break;
}



static void syscall_handler(struct intr_frame* f) {
  uint32_t* args = (uint32_t*)f->esp;
  struct thread* t = thread_current();
  t->in_syscall = true;

  validate_buffer_in_user_region(args, sizeof(uint32_t));
  switch (args[0]) {
    case SYS_EXIT:
      validate_buffer_in_user_region(&args[1], sizeof(uint32_t));
      syscall_exit((int)args[1]);
      break;

    case SYS_OPEN:
      validate_buffer_in_user_region(&args[1], sizeof(uint32_t));
      validate_string_in_user_region((char*)args[1]);
      f->eax = (uint32_t)syscall_open((char*)args[1]);
      break;

    case SYS_WRITE:
      validate_buffer_in_user_region(&args[1], 3 * sizeof(uint32_t));
      validate_buffer_in_user_region((void*)args[2], (unsigned)args[3]);
      f->eax = (uint32_t)syscall_write((int)args[1], (void*)args[2], (unsigned)args[3]);
      break;

    case SYS_READ:
      validate_buffer_in_user_region(&args[1], 3 * sizeof(uint32_t));
      validate_buffer_in_user_region((void*)args[2], (unsigned)args[3]);
      f->eax = (uint32_t)syscall_read((int)args[1], (void*)args[2], (unsigned)args[3]);
      break;

    case SYS_CLOSE:
      validate_buffer_in_user_region(&args[1], sizeof(uint32_t));
      syscall_close((int)args[1]);
      break;
    
    case SYS_SBRK:
      f->eax = (uint32_t) syscall_sbrk((intptr_t)args[1]);
      break;

    default:
      printf("Unimplemented system call: %d\n", (int)args[0]);
      break;
  }

  t->in_syscall = false;
}
