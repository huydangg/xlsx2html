#include <zip.h>
#include <expat.h>

#ifdef XML_LARGE_SIZE
#  define XML_FMT_INT_MOD "ll"
#else
#  define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
#  include <wchar.h>
#  define XML_FMT_STR "ls"
#else
#  define XML_FMT_STR "s"
#endif




struct NumFMT {
  XML_Char *formatCode;
  XML_Char *numFmtId;
};

struct ArrayNumFMTs {
  unsigned short int length;
  struct NumFMT *numfmts;
};

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

struct ArrayFonts {
  unsigned short int length;
  struct Font *fonts;
};

struct PatternFill {
  XML_Char *patternType;
  struct Color bgColor;
  struct Color fgColor;
};

struct Fill {
  struct PatternFill patternFill;
};

struct ArrayFills {
  unsigned short int length;
  struct Fill *fills;
};

struct Border {
  XML_Char *style;
  struct Color color;
};

struct BorderCell {
  struct Border left;
  struct Border right;
  struct Border top;
  struct Border bottom;
};

struct ArrayBorderCells {
  unsigned short int length;
  struct BorderCell *borders;
};

struct Alignment {
  XML_Char *horizontal;
  XML_Char *vertical;
  XML_Char *textRotation;
  char isWrapText;
};


struct Xf {
  XML_Char *borderId;
  XML_Char *fillId;
  XML_Char *fontId;
  XML_Char *numFmtId;
  XML_Char *xfId; // CellStyleXfs
  char isApplyAlignment;
  char isApplyBorder;
  char isApplyFont;
  char isApplyNumberFormat;
  struct Alignment alignment;
};

struct ArrayXfs {
  unsigned short int length;
  struct Xf *Xfs;
};
  
extern XML_Parser xmlparser;
extern struct ArrayNumFMTs array_numfmts;
extern struct ArrayFonts array_fonts;
extern struct ArrayFills array_fills;
extern struct ArrayBorderCells array_borders;
extern struct ArrayXfs array_cellStyleXfs;
extern struct ArrayXfs array_cellXfs;



void styles_start_element(void *userData, const XML_Char *name, const XML_Char **attrs); 
void styles_end_element(void *userData, const XML_Char *name); 
void numFmt_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void numFmt_main_end_element(void *userData, const XML_Char *name);
void font_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void font_main_end_element(void *userData, const XML_Char *name);
void font_item_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void font_item_end_element(void *userData, const XML_Char *name);
void fill_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void fill_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs); 
void fill_item_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void fill_item_lv1_end_element(void *userData, const XML_Char *name); 
void fill_item_lv2_end_element(void *userData, const XML_Char *name);
void border_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void border_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void border_item_lv1_end_element(void *userData, const XML_Char *name);
void border_item_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void border_item_lv2_end_element(void *userData, const XML_Char *name);
void xf_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void xf_main_end_element(void *userData, const XML_Char *name);
void xf_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void xf_item_lv1_end_element(void *userData, const XML_Char *name);

