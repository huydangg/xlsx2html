#ifndef INCLUDED_PRIVATE_H
#define INCLUDED_PRIVATE_H


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
#endif
