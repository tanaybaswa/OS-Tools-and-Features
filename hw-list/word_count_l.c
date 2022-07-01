/*
 * Implementation of the word_count interface using Pintos lists.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_l.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t* wclist) {
  if (wclist == NULL){
    return;
  }
  list_init(wclist);
}

size_t len_words(word_count_list_t* wclist) {
  if (wclist == NULL){
    return (size_t) 0;
  }
  return list_size(wclist);
}

word_count_t* find_word(word_count_list_t* wclist, char* word) {
  word_count_t *w = NULL;

   if (wclist == NULL || word == NULL) {
    return w;
    }

  struct list_elem *e;

  for (e = list_begin(wclist); e != list_end(wclist);
        e = list_next(e))
    {
      word_count_t *wct = list_entry(e, word_count_t, elem);
      if (strcmp(word, wct->word) == 0) {
        return wct;
      }
      
    }


  return w;
}


word_count_t* add_word(word_count_list_t* wclist, char* word) {
  word_count_t *w = find_word(wclist, word);
  if (w != NULL){
    w->count++;
  } else {
    w = (word_count_t *) malloc(sizeof(word_count_t));
    w->word = (char *) malloc(sizeof(char) * (strlen(word) + 1));
    strcpy(w->word, word);
    w->count = 1;
    // insert the new word_count_t into the list
    list_push_front(wclist, &(w->elem));
  }

  return w;

}

void fprint_words(word_count_list_t* wclist, FILE* outfile) { 
  struct list_elem *e;

  for (e = list_begin(wclist); e != list_end(wclist);
        e = list_next(e))
    {
      word_count_t *wct = list_entry(e, word_count_t, elem);
      fprintf(outfile, "%i\t%s\n", wct->count, wct->word);
    }

}

static bool less_list(const struct list_elem* ewc1, const struct list_elem* ewc2, void* aux) {
  bool (*l)(const word_count_t *, const word_count_t *) = aux;
  struct word_count *wc1 = list_entry(ewc1, struct word_count, elem);
  struct word_count *wc2 = list_entry(ewc2, struct word_count, elem);

  return l(wc1, wc2);
}

void wordcount_sort(word_count_list_t* wclist,
                    bool less(const word_count_t*, const word_count_t*)) {
  list_sort(wclist, less_list, less);
}
