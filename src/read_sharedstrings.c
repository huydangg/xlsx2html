#include <read_sharedstrings.h>
#include <string.h>

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

struct FontObj {
  int has_font;
  struct Font font;
};

struct FontObj new_fontobj() {
  struct FontObj _fontobj;  
  _fontobj.has_font = 0;
  _fontobj.font.sz = 0.0;
  _fontobj.font.isBold = '0';
  _fontobj.font.isItalic = '0';
  _fontobj.font.underline = malloc(1);
  _fontobj.font.underline = '\0';
  _fontobj.font.name = malloc(1);
  _fontobj.font.name = '\0';
  _fontobj.font.color.rgb = malloc(1);
  _fontobj.font.color.rgb[0] = '\0';
  return _fontobj;
}

struct FontObj fontobj;

void sharedStrings_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  fontobj= new_fontobj();
  if (strcmp(name, "sst") == 0) {
    XML_SetElementHandler(xmlparser, sharedStrings_lv1_start_element, NULL);
  }
}

void sharedStrings_main_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "sst") == 0) {
    XML_SetElementHandler(xmlparser, sharedStrings_main_start_element, NULL);
  }
}

void sharedStrings_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  FILE *_tmp_sharedStrings_callbackdata = userData;
  if (strcmp(name, "si") == 0) {
    fprintf(_tmp_sharedStrings_callbackdata, "<td>");
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, sharedStrings_main_end_element);
  }
}

void sharedStrings_lv1_end_element(void *userData, const XML_Char *name) {
  FILE *_tmp_sharedStrings_callbackdata = userData;
  if (strcmp(name, "si") == 0) {
    fprintf(_tmp_sharedStrings_callbackdata, "</td>");
    XML_SetElementHandler(xmlparser, sharedStrings_lv1_start_element, sharedStrings_main_end_element);
  }
}

void sharedStrings_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name, "t") == 0) {
    FILE *_tmp_sharedStrings_callbackdata = userData;
    /*if (fontobj.has_font == 1) {
        printf("---------------------------------------------\n");
        printf("FONT SIZE: %f\n", fontobj.font.sz);
        printf("FONT NAME: %s\n", fontobj.font.name);
        printf("FONT IS BOLD: %c\n", fontobj.font.isBold);
        printf("FONT IS ITALIC: %c\n", fontobj.font.isItalic);
        printf("FONT UNDERLINE: %s\n", fontobj.font.underline);
        printf("FONT COLOR RGB: %s\n", fontobj.font.color.rgb);
    }*/
    fprintf(_tmp_sharedStrings_callbackdata, "<p>");
    free(fontobj.font.name);
    free(fontobj.font.underline);
    free(fontobj.font.color.rgb);
    XML_SetElementHandler(xmlparser, NULL, sharedStrings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, sharedStrings_content_handler);
  } else if(strcmp(name, "rPr") == 0) {
    fontobj = new_fontobj();
    fontobj.has_font = 1;
    XML_SetElementHandler(xmlparser, sharedStrings_rPritem_start_element, NULL);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void sharedStrings_lv2_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "t") == 0) {
    FILE *_tmp_sharedStrings_callbackdata = userData;
    fprintf(_tmp_sharedStrings_callbackdata, "</p>");
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, sharedStrings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else {
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, sharedStrings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void sharedStrings_rPritem_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name, "sz") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fontobj.font.sz = strtof((char *)attrs[i + 1], NULL);
      }
    }
  }
  if (strcmp(name, "rFont") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fontobj.font.name = realloc(fontobj.font.name, sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(fontobj.font.name, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  if (strcmp(name, "b") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fontobj.font.isBold = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  }
  if (strcmp(name, "i") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fontobj.font.isItalic = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  }
  if (strcmp(name, "u") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fontobj.font.underline = realloc(fontobj.font.underline, sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(fontobj.font.underline, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  if (strcmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fontobj.font.color.rgb = realloc(fontobj.font.color.rgb, sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(fontobj.font.color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, sharedStrings_rPritem_end_element);
}

void sharedStrings_rPritem_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, sharedStrings_rPritem_start_element, sharedStrings_lv2_end_element);
  XML_SetCharacterDataHandler(xmlparser, NULL);
}

void sharedStrings_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  FILE *_tmp_sharedStrings_callbackdata = callbackdata;
  XML_Char *value;
  if ((value = malloc(len + 1)) == NULL) {
    return;
  } 
  memcpy(value, buf, len);
  value[len] = '\0';
  fputs(value, _tmp_sharedStrings_callbackdata);
  free(value);
}
