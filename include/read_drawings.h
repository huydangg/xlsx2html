#ifndef INCLUDED_READ_DRAWINGS_H
#define INCLUDED_READ_DRAWINGS_H

#include <expat.h>

struct From {
  char *col;
  char *colOff;
  char *row;
  char *rowOff;
};

struct To {
  char *col;
  char *colOff;
  char *row;
  char *rowOff;
};

struct TwoCellAnchor {
  char *editAs;
  struct From from;
  struct To to;
};

extern XML_Parser xmlparser;

void drawings_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs);
void drawings_end_element(void *callbackdata, const XML_Char *name);

#endif
