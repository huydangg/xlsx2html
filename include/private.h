#ifndef INCLUDED_PRIVATE_H
#define INCLUDED_PRIVATE_H

#include <expat.h>

#ifdef XML_LARGE_SIZE
#define XML_FMT_INT_MOD "ll"
#else
#define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
#include <wchar.h>
#define XML_FMT_STR "ls"
#else
#define XML_FMT_STR "s"
#endif

// UTF-8 version
#define X(s) s
#define XML_Char_icmp strcasecmp
#define XML_Char_len strlen
#define XML_Char_malloc(n) (malloc(n))
#define XML_Char_realloc(m, n) (realloc((m), (n)))
#define XML_Char_tol(s) strtol((s), NULL, 10)
#define XML_Char_tod(s) strtod((s), NULL)
#define XML_Char_snprintf snprintf

// UTF-16 version
// TODO

#define DEBUG_LEVEL 1

#define debug_print(fmt, ...)                                                  \
  do {                                                                         \
    if (DEBUG_LEVEL)                                                           \
      fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__,        \
              ##__VA_ARGS__);                                                  \
  } while (0)

struct Color {
  XML_Char *rgb;
};

struct Font {
  float sz;
  XML_Char *name;
  char isBold;
  char isItalic;
  XML_Char *underline;
  struct Color color;
};

extern const char *OUTPUT_FILE_NAME;
extern const char *OUTPUT_DIR;
extern const char *ORIGIN_FILE_PATH;
extern const char *TEMP_DIR;
extern const char *RESOURCE_URL;
extern const char *WORKING_DIR;
extern const char *CHUNKS_DIR_PATH;
extern const char *SHAREDSTRINGS_HTML_FILE_PATH;

#define CHUNKS_DIR_NAME "chunks"
#define BASE_CSS_FILE_NAME "base.min.css"
#define BASE_JS_FILE_NAME "xlsxmagic.min.js"
#define MANIFEST_FILE_NAME "manifest"
#define TEMPLATES_DIR_NAME "templates"
#define THIRD_PARTY_DIR_NAME "3rdparty"
#define SSF_BIN_DIR_NAME "ssf/ssf-bin/ssf"

void reversed(char *input);

char *int_to_column_name(unsigned int);
size_t get_col_nr(const XML_Char *A1col);
size_t get_row_nr(const XML_Char *A1col);
unsigned short column_name_to_number(const char *column_name);

#define TYPE_DRAWING                                                           \
  "http://schemas.openxmlformats.org/officeDocument/2006/relationships/"       \
  "drawing"

#endif
