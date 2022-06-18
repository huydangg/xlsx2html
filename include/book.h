#ifndef INCLUDED_BOOK_H
#define INCLUDED_BOOK_H

#include <relationship.h>

struct Sheet {
  XML_Char *name;
  XML_Char *sheetId;
  XML_Char *path_name;
  char isHidden;
  char hasMergedCells;
  unsigned short num_of_chunks;
  unsigned int max_row;
  unsigned short max_col_number;
  struct ArrayRelationships array_worksheet_rels;
  struct ArrayRelationships array_drawing_rels;
};

struct ArraySheets {
  unsigned short length;
  struct Sheet **sheets;
};

extern XML_Parser xmlparser;
extern struct ArraySheets array_sheets;

void workbook_start_element(void *userData, const XML_Char *name,
                            const XML_Char **attrs);
void workbook_end_element(void *userData, const XML_Char *name);
void sheet_main_start_element(void *userData, const XML_Char *name,
                              const XML_Char **attrs);
void sheet_main_end_element(void *userData, const XML_Char *name);

#endif
