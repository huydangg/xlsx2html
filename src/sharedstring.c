#include "sharedstring.h"
#include "private.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

struct SharedStringsPosition sharedStrings_position;

struct Font new_font() {
  struct Font font;
  font.sz = 0.0;
  font.isBold = '0';
  font.isItalic = '0';
  font.underline = NULL;
  font.name = NULL;
  font.color.rgb = NULL;
  return font;
}

struct Font font;
int current_index;

int clean_ss_data(const char *file_name) {
  struct stat st = {0};
  printf("%d\n", stat(file_name, &st));
  if (stat(file_name, &st) != -1) {
    return remove(file_name);
  }
  return -1;
}

char *concat(const char *s1, const char *s2) {
  const size_t len1 = XML_Char_len(s1);
  const size_t len2 = XML_Char_len(s2);
  // +1 for the null-terminator

  char *result = (char *)XML_Char_malloc(len1 + len2 + 1);
  if (result == NULL) {
    debug_print("%s\n", strerror(errno));
    return NULL;
  }
  // in real code you would check for errors in XML_Char_malloc here
  memcpy(result, s1, len1);
  memcpy(result + len1, s2, len2 + 1); // +1 to copy the null-terminator
  return result;
}

void sharedStrings_main_start_element(void *callbackdata, const XML_Char *name,
                                      const XML_Char **attrs) {
  (void)attrs;

  if (XML_Char_icmp(name, "sst") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "uniqueCount") == 0) {
        sharedStrings_position.positions = (long *)XML_Char_malloc(
            (int)XML_Char_tol(attrs[i + 1]) * sizeof(long));
        current_index = -1;
      }
    }
    font = new_font();
    XML_SetElementHandler(xmlparser, sharedStrings_lv1_start_element, NULL);
  }
}

void sharedStrings_main_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "sst") == 0) {
    sharedStrings_position.length = current_index;
    XML_SetElementHandler(xmlparser, sharedStrings_main_start_element, NULL);
  }
}

void sharedStrings_lv1_start_element(void *callbackdata, const XML_Char *name,
                                     const XML_Char **attrs) {
  (void)attrs;
  FILE *sharedStrings_file_callbackdata = (FILE *)callbackdata;
  if (XML_Char_icmp(name, "si") == 0) {
    current_index++;
    fflush(sharedStrings_file_callbackdata);
    sharedStrings_position.positions[current_index] =
        ftell(sharedStrings_file_callbackdata);
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element,
                          sharedStrings_main_end_element);
  }
}

void sharedStrings_lv1_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "si") == 0) {
    XML_SetElementHandler(xmlparser, sharedStrings_lv1_start_element,
                          sharedStrings_main_end_element);
  }
}

void sharedStrings_lv2_start_element(void *callbackdata, const XML_Char *name,
                                     const XML_Char **attrs) {
  (void)attrs;
  FILE *sharedStrings_file_callbackdata = (FILE *)callbackdata;
  if (XML_Char_icmp(name, "t") == 0) {
    char *font_style = NULL;
    if (font.name != NULL) {
      // 12: "font-family:" | 1: ';'
      const int LEN_FONT_NAME = 14 + XML_Char_len(font.name); // ex
      char font_name[LEN_FONT_NAME];
      snprintf(font_name, LEN_FONT_NAME, "font-family:%s;", font.name);
      font_style = strdup(font_name);
      free(font.name);
      if (font.sz != 0.0f) {
        // 13: "font-size:px;" + 1: '\0' = 14
        const int LEN_FONT_SIZE = snprintf(NULL, 0, "%.2f", font.sz) + 14;
        char font_size[LEN_FONT_SIZE];
        snprintf(font_size, LEN_FONT_SIZE, "font-size:%.2fpx;", font.sz);
        char *tmp_font_style = strdup(font_style);
        free(font_style);
        font_style = concat(tmp_font_style, font_size);
        free(tmp_font_style);
      }
      if (font.isBold != '0') {
        char *tmp_font_style = strdup(font_style);
        free(font_style);
        font_style = concat(tmp_font_style, "font-weight:bold;");
        free(tmp_font_style);
      } else {
        char *tmp_font_style = strdup(font_style);
        free(font_style);
        font_style = concat(tmp_font_style, "font-weight:normal;");
        free(tmp_font_style);
      }
      if (font.isItalic != '0') {
        char *tmp_font_style = strdup(font_style);
        free(font_style);
        font_style = concat(tmp_font_style, "font-style:italic;");
        free(tmp_font_style);
      } else {
        char *tmp_font_style = strdup(font_style);
        free(font_style);
        font_style = concat(tmp_font_style, "font-style:normal;");
        free(tmp_font_style);
      }
      if (font.color.rgb != NULL) {
        const int LEN_FONT_COLOR_RGB = 8 + XML_Char_len(font.color.rgb);
        char font_color_rgb[LEN_FONT_COLOR_RGB];
        // 6: "color:" | 1: ';'
        snprintf(font_color_rgb, LEN_FONT_COLOR_RGB, "color:%s;",
                 font.color.rgb);
        char *tmp_font_style = strdup(font_style);
        free(font_style);
        font_style = concat(tmp_font_style, font_color_rgb);
        free(tmp_font_style);
        free(font.color.rgb);
      }
      if (font.underline != NULL &&
          XML_Char_icmp(font.underline, "none") != 0) {
        const int LEN_FONT_TEXT_DECORATION_LINE = 32;
        char font_text_decoration_line[LEN_FONT_TEXT_DECORATION_LINE];
        memcpy(font_text_decoration_line, "text-decoration-line:underline;",
               LEN_FONT_TEXT_DECORATION_LINE);
        char *tmp_font_style = strdup(font_style);
        free(font_style);
        font_style = concat(tmp_font_style, font_text_decoration_line);
        free(tmp_font_style);
        if (XML_Char_icmp(font.underline, "single") == 0) {
          const int LEN_FONT_TEXT_DECORATION_STYLE = 30;
          char font_text_decoration_style[LEN_FONT_TEXT_DECORATION_STYLE];
          memcpy(font_text_decoration_style, "text-decoration-style:single;",
                 LEN_FONT_TEXT_DECORATION_STYLE);
          char *tmp_font_style = strdup(font_style);
          free(font_style);
          font_style = concat(tmp_font_style, font_text_decoration_style);
          free(tmp_font_style);
        } else if (XML_Char_icmp(font.underline, "double") == 0) {
          const int LEN_FONT_TEXT_DECORATION_STYLE = 30;
          char font_text_decoration_style[LEN_FONT_TEXT_DECORATION_STYLE];
          memcpy(font_text_decoration_style, "text-decoration-style:double;",
                 LEN_FONT_TEXT_DECORATION_STYLE);
          char *tmp_font_style = strdup(font_style);
          free(font_style);
          font_style = concat(tmp_font_style, font_text_decoration_style);
          free(tmp_font_style);
        }
        free(font.underline);
      }
      fprintf(sharedStrings_file_callbackdata, "<span style=\"%s\">",
              font_style);
      free(font_style);
    } else {
      fputs("<span>", sharedStrings_file_callbackdata);
    }
    // Set font obj to default
    font = new_font();
    XML_SetElementHandler(xmlparser, NULL, sharedStrings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, sharedStrings_content_handler);
  } else if (XML_Char_icmp(name, "rPr") == 0) {
    font = new_font();
    XML_SetElementHandler(xmlparser, sharedStrings_rPritem_start_element,
                          sharedStrings_rPritem_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void sharedStrings_lv2_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "t") == 0) {
    FILE *sharedStrings_file_callbackdata = (FILE *)callbackdata;
    fprintf(sharedStrings_file_callbackdata, "</span>");
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element,
                          sharedStrings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (XML_Char_icmp(name, "rPr") == 0) {
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element,
                          sharedStrings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void sharedStrings_rPritem_start_element(void *callbackdata,
                                         const XML_Char *name,
                                         const XML_Char **attrs) {
  (void)attrs;
  if (XML_Char_icmp(name, "sz") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
        font.sz = strtof((char *)attrs[i + 1], NULL);
      }
    }
  }
  if (XML_Char_icmp(name, "rFont") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
        font.name = (XML_Char *)XML_Char_realloc(
            font.name, sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
        memcpy(font.name, attrs[i + 1],
               sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
  }
  if (XML_Char_icmp(name, "b") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
        font.isBold = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  }
  if (XML_Char_icmp(name, "i") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
        font.isItalic = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  }
  if (XML_Char_icmp(name, "u") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
        font.underline = (XML_Char *)XML_Char_realloc(
            font.underline,
            sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
        memcpy(font.underline, attrs[i + 1],
               sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
  }
  if (XML_Char_icmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
        font.color.rgb = (XML_Char *)XML_Char_realloc(
            font.color.rgb,
            sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
        memcpy(font.color.rgb, attrs[i + 1],
               sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
  }
}

void sharedStrings_rPritem_end_element(void *callbackdata,
                                       const XML_Char *name) {
  XML_SetElementHandler(xmlparser, sharedStrings_rPritem_start_element,
                        sharedStrings_lv2_end_element);
  XML_SetCharacterDataHandler(xmlparser, NULL);
}

void sharedStrings_content_handler(void *callbackdata, const XML_Char *buf,
                                   int len) {
  if (len == 0) {
    return;
  }
  FILE *sharedStrings_file_callbackdata = (FILE *)callbackdata;
  XML_Char *value;
  int len_value = len;
  if ((value = (XML_Char *)XML_Char_malloc(len_value + 1)) == NULL) {
    return;
  }
  memcpy(value, buf, len_value);
  if (value[len_value - 1] == '\n') {
    // 5: <br/>
    len_value = len_value + 4;
    if ((value = (XML_Char *)XML_Char_realloc(value, len_value + 1)) == NULL) {
      return;
    }
    memcpy(value + len - 1, "<br/>", 5);
  }
  value[len_value] = '\0';
  fputs(value, sharedStrings_file_callbackdata);
  free(value);
}
