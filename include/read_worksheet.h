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
  char start_row; // 1
  char *end_row;
  struct ArrayCols array_cols;
};

extern XML_Parser xmlparser;

struct WorkSheet;  

void worksheet_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void worksheet_end_element(void *userData, const XML_Char *name);
void col_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void col_end_element(void *userData, const XML_Char *name);
