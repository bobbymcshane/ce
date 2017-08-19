#include "ce_app.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

bool buffer_node_insert(BufferNode_t** head, CeBuffer_t* buffer){
     BufferNode_t* node = malloc(sizeof(*node));
     if(!node) return false;
     node->buffer = buffer;
     node->next = *head;
     *head = node;
     return true;
}

bool buffer_node_delete(BufferNode_t** head, CeBuffer_t* buffer){
     BufferNode_t* prev = NULL;
     BufferNode_t* itr = *head;
     while(itr){
          if(itr->buffer == buffer) break;
          prev = itr;
          itr = itr->next;
     }

     if(!itr) return false;

     if(prev){
          prev->next = itr->next;
     }else{
          *head = (*head)->next;
     }

     // TODO: compress with below
     free(itr->buffer->user_data);
     ce_buffer_free(itr->buffer);
     free(itr->buffer);
     free(itr);
     return true;
}

void buffer_node_free(BufferNode_t** head){
     BufferNode_t* itr = *head;
     while(itr){
          BufferNode_t* tmp = itr;
          itr = itr->next;
          free(tmp->buffer->user_data);
          ce_buffer_free(tmp->buffer);
          free(tmp->buffer);
          free(tmp);
     }
     *head = NULL;
}

StringNode_t* string_node_insert(StringNode_t** head, const char* string){
     StringNode_t* tail = *head;
     StringNode_t* node;
     if(tail){
          while(tail->next) tail = tail->next;

          // NOTE: we probably don't want this if we want the linked list to be general
          // skip the insertion if the string matches the previous string
          if(strcmp(string, tail->string) == 0) return NULL;

          node = calloc(1, sizeof(*node));
          if(!node) return node;
          node->string = strdup(string);

          tail->next = node;
          node->prev = tail;
     }else{
          node = calloc(1, sizeof(*node));
          if(!node) return node;
          node->string = strdup(string);

          *head = node;
     }

     return node;
}

void string_node_free(StringNode_t** head){
     StringNode_t* itr = *head;
     while(itr){
          StringNode_t* tmp = itr;
          itr = itr->next;
          free(tmp->string);
          free(tmp);
     }

     *head = NULL;
}

bool history_insert(History_t* history, const char* string){
     StringNode_t* new_node = string_node_insert(&history->head, string);
     if(new_node) return true;
     return false;
}

char* history_previous(History_t* history){
     if(history->current){
          if(history->current->prev){
               history->current = history->current->prev;
          }
          return history->current->string;
     }

     StringNode_t* tail = history->head;
     if(!tail) return NULL;

     while(tail->next) tail = tail->next;
     history->current = tail;

     return tail->string;
}

char* history_next(History_t* history){
     if(history->current){
          if(history->current->next){
               history->current = history->current->next;
          }
          return history->current->string;
     }

     return NULL;
}

void convert_bind_defs(KeyBinds_t* binds, KeyBindDef_t* bind_defs, int64_t bind_def_count){
     binds->count = bind_def_count;
     binds->binds = malloc(binds->count * sizeof(*binds->binds));

     for(int64_t i = 0; i < binds->count; ++i){
          ce_command_parse(&binds->binds[i].command, bind_defs[i].command);
          binds->binds[i].key_count = 0;

          for(int k = 0; k < 4; ++k){
               if(bind_defs[i].keys[k] == 0) break;
               binds->binds[i].key_count++;
          }

          if(!binds->binds[i].key_count) continue;

          binds->binds[i].keys = malloc(binds->binds[i].key_count * sizeof(binds->binds[i].keys[0]));

          for(int k = 0; k < binds->binds[i].key_count; ++k){
               binds->binds[i].keys[k] = bind_defs[i].keys[k];
          }
     }
}

void app_update_terminal_view(App_t* app){
     getmaxyx(stdscr, app->terminal_height, app->terminal_width);
     app->terminal_rect = (CeRect_t){0, app->terminal_width - 1, 0, app->terminal_height - 1};
     ce_layout_distribute_rect(app->tab_list_layout, app->terminal_rect, app->config_options.horizontal_scroll_off,
                               app->config_options.vertical_scroll_off, app->config_options.tab_width);
}

CeComplete_t* app_is_completing(App_t* app){
     if(app->input_mode){
          if(strcmp(app->input_view.buffer->name, "COMMAND") == 0) return &app->command_complete;
          if(strcmp(app->input_view.buffer->name, "LOAD FILE") == 0) return &app->load_file_complete;
          if(strcmp(app->input_view.buffer->name, "SWITCH BUFFER") == 0) return &app->switch_buffer_complete;
     }

     return NULL;
}
