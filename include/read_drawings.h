#ifndef INCLUDED_READ_DRAWINGS_H
#define INCLUDED_READ_DRAWINGS_H

#include <expat.h>

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

void drawings_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs);
void drawings_lv1_end_element(void *callbackdata, const XML_Char *name);
void drawings_lv2_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs);
void drawings_lv2_end_element(void *callbackdata, const XML_Char *name);
void drawings_lv3_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs);
void drawings_lv3_end_element(void *callbackdata, const XML_Char *name);
void drawings_lv4_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs);
void drawings_lv4_end_element(void *callbackdata, const XML_Char *name);
void drawings_col_content_handler(void *callbackdata, const XML_Char *buf, int len);
void drawings_colOff_content_handler(void *callbackdata, const XML_Char *buf, int len);
void drawings_row_content_handler(void *callbackdata, const XML_Char *buf, int len);
void drawings_rowOff_content_handler(void *callbackdata, const XML_Char *buf, int len);
#endif
