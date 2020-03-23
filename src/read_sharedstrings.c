#include <read_sharedstrings.h>
#include <string.h>

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

char* concat(const char *s1, const char *s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1); // +1 to copy the null-terminator
    return result;
}

void sharedStrings_main_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;

  if (strcmp(name, "sst") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "uniqueCount") == 0) {
	sharedStrings_position.positions = malloc((int)strtol(attrs[i + 1], NULL, 10) * sizeof(long int));
	current_index = -1;
      }
    }
    font = new_font();
    XML_SetElementHandler(xmlparser, sharedStrings_lv1_start_element, NULL);
  }
}

void sharedStrings_main_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "sst") == 0) {
    sharedStrings_position.length = current_index;
    XML_SetElementHandler(xmlparser, sharedStrings_main_start_element, NULL);
  }
}

void sharedStrings_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  FILE *sharedStrings_file_callbackdata = callbackdata;
  if (strcmp(name, "si") == 0) {
    current_index++;
    fflush(sharedStrings_file_callbackdata);
    sharedStrings_position.positions[current_index] = ftell(sharedStrings_file_callbackdata);
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, sharedStrings_main_end_element);
  }
}

void sharedStrings_lv1_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "si") == 0) {
    XML_SetElementHandler(xmlparser, sharedStrings_lv1_start_element, sharedStrings_main_end_element);
  }
}

void sharedStrings_lv2_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  FILE *sharedStrings_file_callbackdata = callbackdata;
  if (strcmp(name, "t") == 0) {
    char *font_style = NULL;
    if (font.name != NULL) {
      printf("---------------------------------------------\n");
      printf("FONT SIZE: %f\n", font.sz);
      printf("FONT NAME: %s\n", font.name);
      printf("FONT IS BOLD: %c\n", font.isBold);
      printf("FONT IS ITALIC: %c\n", font.isItalic);
      printf("FONT UNDERLINE: %s\n", font.underline);
      printf("FONT COLOR RGB: %s\n", font.color.rgb);
      //12: "font-family:" | 1: ';'
      const int LEN_FONT_NAME = 14 + strlen(font.name); //ex
      char font_name[LEN_FONT_NAME];
      snprintf(font_name, LEN_FONT_NAME, "font-family:%s;", font.name);
      font_style = strdup(font_name); 
      free(font.name);
      if (font.sz != 0.0) {
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
      }
      if (font.isItalic != '0') {
	char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, "font-style:italic;");
	free(tmp_font_style);
      }
      if (font.color.rgb != NULL) {
	const int LEN_FONT_COLOR_RGB = 8 + strlen(font.color.rgb);
        char font_color_rgb[LEN_FONT_COLOR_RGB];
        // 6: "color:" | 1: ';'
        snprintf(font_color_rgb, LEN_FONT_COLOR_RGB, "color:%s;", font.color.rgb);
	char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, font_color_rgb);
	free(tmp_font_style);
        free(font.color.rgb);
      }
      if (font.underline != NULL && strcmp(font.underline, "none") != 0) {
	const int LEN_FONT_TEXT_DECORATION_LINE = 32;
        char font_text_decoration_line[LEN_FONT_TEXT_DECORATION_LINE];
	memcpy(font_text_decoration_line, "text-decoration-line:underline;", LEN_FONT_TEXT_DECORATION_LINE);
	char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, font_text_decoration_line);
	free(tmp_font_style);
        if (strcmp(font.underline, "single") == 0) {
	  const int LEN_FONT_TEXT_DECORATION_STYLE = 30;
          char font_text_decoration_style[LEN_FONT_TEXT_DECORATION_STYLE];
	  memcpy(font_text_decoration_style, "text-decoration-style:single;", LEN_FONT_TEXT_DECORATION_STYLE);
	  char *tmp_font_style = strdup(font_style);
	  free(font_style);
	  font_style = concat(tmp_font_style, font_text_decoration_style);
	  free(tmp_font_style);
        } else if(strcmp(font.underline, "double") == 0) {
	  const int LEN_FONT_TEXT_DECORATION_STYLE = 30;
          char font_text_decoration_style[LEN_FONT_TEXT_DECORATION_STYLE];
	  memcpy(font_text_decoration_style, "text-decoration-style:double;", LEN_FONT_TEXT_DECORATION_STYLE);
	  char *tmp_font_style = strdup(font_style);
	  free(font_style);
	  font_style = concat(tmp_font_style, font_text_decoration_style);
	  free(tmp_font_style);
        }
        free(font.underline);
      }
      printf("FONT STYLE STRING: %s\n", font_style);
      fprintf(sharedStrings_file_callbackdata, "<p style=\"%s\">", font_style);
      free(font_style);
    } else {
      fputs("<p>", sharedStrings_file_callbackdata);
    }
    // Set font obj to default
    font = new_font();
    XML_SetElementHandler(xmlparser, NULL, sharedStrings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, sharedStrings_content_handler);
  } else if(strcmp(name, "rPr") == 0) {
    font = new_font();
    XML_SetElementHandler(xmlparser, sharedStrings_rPritem_start_element, sharedStrings_rPritem_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void sharedStrings_lv2_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "t") == 0) {
    FILE *sharedStrings_file_callbackdata = callbackdata;
    fprintf(sharedStrings_file_callbackdata, "</p>");
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, sharedStrings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (strcmp(name, "rPr") == 0){
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, sharedStrings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void sharedStrings_rPritem_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name, "sz") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font.sz = strtof((char *)attrs[i + 1], NULL);
      }
    }
  }
  if (strcmp(name, "rFont") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font.name = realloc(font.name, sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(font.name, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  if (strcmp(name, "b") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font.isBold = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  }
  if (strcmp(name, "i") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font.isItalic = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  }
  if (strcmp(name, "u") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font.underline = realloc(font.underline, sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(font.underline, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  if (strcmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font.color.rgb = realloc(font.color.rgb, sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(font.color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
}

void sharedStrings_rPritem_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, sharedStrings_rPritem_start_element, sharedStrings_lv2_end_element);
  XML_SetCharacterDataHandler(xmlparser, NULL);
}

void sharedStrings_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  FILE *sharedStrings_file_callbackdata = callbackdata;
  XML_Char *value;
  int len_value = len;
  if ((value = malloc(len_value + 1)) == NULL) {
    return;
  }
  memcpy(value, buf, len_value);
  if (value[len_value - 1] == '\n') {
    //5: <br/>
    len_value = len_value + 4;
    if ((value = realloc(value, len_value + 1)) == NULL) {
      return;
    }
    memcpy(value + len - 1, "<br/>", 5);
  }
  value[len_value] = '\0';
  fputs(value, sharedStrings_file_callbackdata);
  free(value);
}
