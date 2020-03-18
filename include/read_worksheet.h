#ifndef INCLUDED_READ_WORKSHEET_H
#define INCLUDED_READ_WORKSHEET_H

#include <expat.h>


struct Col {
  unsigned short int min;
  unsigned short int max;
  float width;
  char isHidden;
};

struct ArrayCols {
  unsigned short int length;
  struct Col **cols;
};

struct WorkSheet {
  char start_col; // A
  char *end_col;
  unsigned short end_col_number;
  char start_row; // 1
  char *end_row;
  unsigned short index_sheet;
  struct ArrayCols array_cols;
};

extern XML_Parser xmlparser;
extern unsigned short NUM_OF_CHUNKS;
extern unsigned int NUM_OF_CELLS;
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
