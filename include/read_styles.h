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


struct Sheet {
   XML_Char *name;
   XML_Char *sheet_id;
   char *path_name;
   char isHidden;
};

struct ArraySheets {
  unsigned short int length;
  struct Sheet **sheets;
};

struct NumFMT {
  XML_Char *format_code;
  XML_Char *format_id;
};

struct ArrayNumFMTs {
  unsigned short int length;
  struct NumFMT *numfmts;
};

struct Color {
  XML_Char *rgb;
};

struct Font {
  int size;
  XML_Char *name;
  int is_bold;
  int is_italic; 
  XML_Char *underline;
  struct Color color;
};

struct ArrayFonts {
  unsigned short int length;
  struct Font *fonts;
};

struct PatternFill {
  XML_Char *pattern_type;
  struct Color bg_color;
  struct Color fg_color;
};

struct Fill {
  struct PatternFill pattern_fill;
};

struct ArrayFills {
  unsigned short int length;
  struct Fill *fills;
};

struct Border {
  XML_Char *style;
  struct Color border_color;
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
  struct Alignment alignment;
};

struct ArrayXfs {
  unsigned short int length;
  struct Xf *Xfs;
};
  
extern XML_Parser xmlparser;
extern struct ArraySheets array_sheets;
extern struct ArrayNumFMTs array_numfmts;
extern struct ArrayFonts array_fonts;
extern struct ArrayFills array_fills;
extern struct ArrayBorderCells array_borders;
extern struct ArrayXfs array_cellStyleXfs;
extern struct ArrayXfs array_cellXfs;



void content_handler(void *userData, const XML_Char *s, int len); 
void styles_start_element(void *userData, const XML_Char *name, const XML_Char **attrs); 
void styles_end_element(void *userData, const XML_Char *name); 
void sheet_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void sheet_main_end_element(void *userData, const XML_Char *name);
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

