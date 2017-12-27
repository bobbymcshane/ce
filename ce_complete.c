#include "ce_complete.h"
#include "ce_subprocess.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

bool ce_complete_init(CeComplete_t* complete, const char** strings, const char** descriptions, int64_t string_count){
     ce_complete_free(complete);

     complete->elements = calloc(string_count, sizeof(*complete->elements));
     if(!complete->elements) return false;

     for(int64_t i = 0; i < string_count; i++){
          complete->elements[i].string = strdup(strings[i]);
          if(descriptions && descriptions[i]){
               complete->elements[i].description = strdup(descriptions[i]);
          }
          if(!complete->elements[i].string) return false;
     }

     complete->element_count = string_count;
     complete->matches = complete->elements;
     complete->match_count = complete->element_count;
     return true;
}

void _free_matches(CeComplete_t* complete){
     // if matches doesn't just point to elements, free its elements
     if(complete->matches != complete->elements){
          for(int64_t i = 0; i < complete->match_count; i++){
               free(complete->matches[i].string);
               free(complete->matches[i].description);
          }
          free(complete->matches);
     }
     complete->matches = NULL;
     complete->match_count = 0;
}

void ce_complete_reset(CeComplete_t* complete){
     free(complete->current_match);
     complete->current_match = NULL;

     _free_matches(complete);
     complete->matches = complete->elements;
     complete->match_count = complete->element_count;
     complete->current = 0;
}

// add match to the beginning of the match list
static void _prepend_match(CeComplete_t* complete, const char* match, const char* description){
     // we should not have more matches than elements
     assert(complete->match_count < complete->element_count);
     // shift all elements down one
     memmove(&complete->matches[1], complete->matches, complete->match_count * sizeof(*complete->matches));
     complete->matches[0].string = strdup(match);
     if( description ) complete->matches[0].description = strdup(description);
     complete->match_count++;
}

// add match to the end of the match list
static void _append_match(CeComplete_t* complete, const char* match, const char* description){
     // we should not have more matches than elements
     assert(complete->match_count < complete->element_count);
     complete->matches[complete->match_count].string = strdup(match);
     if( description ) complete->matches[complete->match_count].description = strdup(description);
     complete->match_count++;
}

void _default_match(CeComplete_t* complete, const char* match){
     for(int64_t i = 0; i < complete->element_count; i++){
          const char* str = strstr(complete->elements[i].string, match);
          if(str != NULL){
               // the best matches go at the beginning
               if(strcmp(match, complete->elements[i].string) == 0)
                    _prepend_match(complete, complete->elements[i].string, complete->elements[i].description);
               else _append_match(complete, complete->elements[i].string, complete->elements[i].description);
          }
     }
}

void _match_with_fzf(CeComplete_t* complete, const char* match){
     CeSubprocess_t fzf;
     {
          char command[BUFSIZ];
          snprintf(command, sizeof(command), "fzf -f'%s'", match);
          bool success = ce_subprocess_open(&fzf, command);
          assert(success);
     }

     {
          // send elements to fzf to filter and rank
          for(int64_t i = 0; i < complete->element_count; i++){
               fprintf(fzf.stdin, "%s\n", complete->elements[i].string);
          }

          // close stdin to indicate we have sent all entries
          ce_subprocess_close_stdin(&fzf);
     }

     {
          // read filtered elements back from fzf
          char element[BUFSIZ];
          while(fgets(element, sizeof(element), fzf.stdout)){
               assert(element[strlen(element)-1] == '\n');
               element[strlen(element)-1] = '\0';
               // TODO: get descriptions as well
               _append_match(complete, element, NULL);
          }
     }

     ce_subprocess_close(&fzf);
}

void ce_complete_match(CeComplete_t* complete, const char* match){
     if(complete->element_count == 0) return;

     if(strlen(match)){
          _free_matches(complete);
          complete->matches = calloc(complete->element_count, sizeof(*complete->matches));
          #if 0
          _default_match(complete, match);
          #else
          _match_with_fzf(complete, match);
          #endif
     }else{
          // filter string is empty. all options are matches
          ce_complete_reset(complete);
     }

     if(complete->match_count > 0){
          complete->current = 0;
          free(complete->current_match);
          complete->current_match = strdup(match);
     } else {
          // no matches found!
          complete->current = -1;
     }
}

void ce_complete_next_match(CeComplete_t* complete){
     if(complete->current < 0 || !complete->match_count) return;

     complete->current++;
     if(complete->current >= complete->match_count){
          complete->match_count = 0;
     }
}

void ce_complete_previous_match(CeComplete_t* complete){
     if(complete->current < 0 || !complete->match_count) return;

     complete->current = complete->current ? complete->current - 1 : complete->match_count - 1;
}

void ce_complete_free(CeComplete_t* complete){
     _free_matches(complete);

     for(int64_t i = 0; i < complete->element_count; i++){
          free(complete->elements[i].string);
          free(complete->elements[i].description);
     }

     free(complete->elements);
     free(complete->current_match);
     memset(complete, 0, sizeof(*complete));
}
