#pragma once

#include "ce.h"
#include "ce_vim.h"

#define CE_SYNTAX_USE_CURRENT_COLOR -2

typedef enum{
     CE_SYNTAX_COLOR_NORMAL,
     CE_SYNTAX_COLOR_TYPE,
     CE_SYNTAX_COLOR_KEYWORD,
     CE_SYNTAX_COLOR_CONTROL,
     CE_SYNTAX_COLOR_CAPS_VAR,
     CE_SYNTAX_COLOR_COMMENT,
     CE_SYNTAX_COLOR_STRING,
     CE_SYNTAX_COLOR_CHAR_LITERAL,
     CE_SYNTAX_COLOR_NUMBER_LITERAL,
     CE_SYNTAX_COLOR_LITERAL,
     CE_SYNTAX_COLOR_PREPROCESSOR,
     CE_SYNTAX_COLOR_TRAILING_WHITESPACE,
     CE_SYNTAX_COLOR_VISUAL,
     CE_SYNTAX_COLOR_MATCH,
     CE_SYNTAX_COLOR_CURRENT_LINE,
     CE_SYNTAX_COLOR_COUNT,
}CeSyntaxColor_t;

typedef struct{
     int fg;
     int bg;
}CeSyntaxDef_t;

typedef struct CeDrawColorNode_t{
     int fg;
     int bg;
     CePoint_t point;
     struct CeDrawColorNode_t* next;
}CeDrawColorNode_t;

typedef struct{
     CeDrawColorNode_t* head;
     CeDrawColorNode_t* tail;
}CeDrawColorList_t;

typedef struct{
     int fg;
     int bg;
}CeColorPair_t;

typedef struct{
     int32_t count;
     int32_t current;
     CeColorPair_t pairs[256]; // NOTE: this is what COLOR_PAIRS was for me (which is for some reason not const?)
}CeColorDefs_t;

typedef void CeSyntaxHighlightFunc_t(CeView_t*, CeVim_t*, CeDrawColorList_t*, CeSyntaxDef_t*);

int ce_syntax_def_get_fg(CeSyntaxDef_t* syntax_defs, CeSyntaxColor_t syntax_color, int current_fg);
int ce_syntax_def_get_bg(CeSyntaxDef_t* syntax_defs, CeSyntaxColor_t syntax_color, int current_bg);

bool ce_draw_color_list_insert(CeDrawColorList_t* list, int fg, int bg, CePoint_t point);
void ce_draw_color_list_free(CeDrawColorList_t* list);
int ce_draw_color_list_last_fg_color(CeDrawColorList_t* draw_color_list);
int ce_draw_color_list_last_bg_color(CeDrawColorList_t* draw_color_list);
int ce_color_def_get(CeColorDefs_t* color_defs, int fg, int bg);

void ce_syntax_highlight_c(CeView_t* view, CeVim_t* vim, CeDrawColorList_t* draw_color_list, CeSyntaxDef_t* syntax_defs);