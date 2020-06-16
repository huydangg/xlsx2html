#ifndef INCLUDED_READ_WORKSHEET_H
#define INCLUDED_READ_WORKSHEET_H

#include <expat.h>
#include <stdio.h>

struct Col {
  unsigned short min;
  unsigned short max;
  float width;
  char isHidden;
};

struct ArrayCols {
  unsigned short length;
  struct Col **cols;
};


struct ArrayDrawingIds {
  unsigned short length;
  char **drawing_ids;
};

struct WorkSheet {
  char start_col; // A
  char *end_col;
  unsigned short end_col_number;
  char start_row; // 1
  char *end_row;
  unsigned short index_sheet;
  char hasMergedCells;
  FILE *worksheet_file;
  char *cell_name;
  char *type_content;
  unsigned short index_style;
  XML_Char *worksheet_content;
  int len_worksheet_content;
  struct ArrayCols array_cols;
  struct ArrayDrawingIds array_drawingids;
  FILE *fmergecell;
  unsigned short ROW_NUMBER; // Temp current row
  char has_dimension;
  unsigned short num_of_chunks;
};

extern XML_Parser xmlparser;
struct WorkSheet;

void worksheet_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void worksheet_end_element(void *userData, const XML_Char *name);
void col_row_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void col_row_end_element(void *userData, const XML_Char *name);
void cell_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs);
void cell_end_element(void *userData, const XML_Char *name);
void cell_item_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs);
void cell_item_end_element(void *callbackdata, const XML_Char *name);
void worksheet_content_handler(void *callbackdata, const XML_Char *s, int len);
#endif
