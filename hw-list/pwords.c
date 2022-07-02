/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2021 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"


struct pwords_args
{
  word_count_list_t *wclist;
  char *file;
};

void *pcount_words(void* p_args) {
  struct pwords_args* pa = (struct pwords_args*) p_args;

  char* file = pa->file;
  word_count_list_t* wclist = pa->wclist;

  FILE* infile = fopen(file, "r");
  count_words(wclist, infile);
  fclose(infile);

  return NULL;
}


/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char* argv[]) {
  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);

  if (argc <= 1) {
    /* Process stdin in a single thread. */
    count_words(&word_counts, stdin);
  } else {

    int num_threads = argc - 1;
    struct pwords_args* arg_list[argc - 1];
    pthread_t threads[argc - 1];

    for (int t = 0; t < num_threads; t++) {

      arg_list[t] = malloc(sizeof(struct pwords_args));
      arg_list[t]->file = argv[t + 1];
      arg_list[t]->wclist = &word_counts; 
      
      pthread_create(&threads[t], NULL, pcount_words, (void*) arg_list[t]);

  }

  for (int t = 0; t < num_threads; t++) {
      pthread_join(threads[t], NULL);
    }
    
  }

  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);
  return 0;
}
