#include <zip.h>
#include <expat.h>

struct Color {
  XML_Char *rgb;
};

struct Font {
  int sz;
  XML_Char *name;
  char isBold;
  char isItalic; 
  XML_Char *underline;
  struct Color color;
};

extern XML_Parser xmlparser;
void sharedStrings_main_start_element(void *, const XML_Char *, const XML_Char **);
void sharedStrings_main_end_element(void *, const XML_Char *);
void sharedStrings_content_handler(void *, const XML_Char *, int);
