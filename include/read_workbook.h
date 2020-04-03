#ifndef INCLUDED_READ_WORKBOOK_H
#define INCLUDED_READ_WORKBOOK_H

#include <expat.h>


struct Sheet {
   XML_Char *name;
   XML_Char *sheetId;
   XML_Char *path_name;
   char isHidden;
   char hasMergedCells;
};

struct ArraySheets {
  unsigned short int length;
  struct Sheet **sheets;
};

extern XML_Parser xmlparser;
extern struct ArraySheets array_sheets;

void workbook_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void workbook_end_element(void *userData, const XML_Char *name);
void sheet_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void sheet_main_end_element(void *userData, const XML_Char *name);

#endif
