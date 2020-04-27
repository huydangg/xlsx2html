#ifndef INCLUDED_READ_DRAWINGS_H
#define INCLUDED_READ_DRAWINGS_H

#include <expat.h>
#include <read_relationships.h>

struct Offset {
  char *col;
  char *colOff;
  char *row;
  char *rowOff;
};

struct Pic {
  XML_Char *name;
  char *hlinkClick_id;
  char *blip_embed;
  size_t cx;
  size_t cy;
};

struct TwoCellAnchor {
  char *editAs;
  struct Offset from;
  struct Offset to;
  struct Pic pic;
};

extern XML_Parser xmlparser;

void drawings_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs);
void drawings_end_element(void *callbackdata, const XML_Char *name);

#endif
