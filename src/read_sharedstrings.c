#include <read_sharedstrings.h>
#include <string.h>



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

void sharedStrings_main_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;

  if (strcmp(name, "sst") == 0) {
    font = new_font();
    XML_SetElementHandler(xmlparser, sharedStrings_lv1_start_element, NULL);
  }
}

void sharedStrings_main_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "sst") == 0) {
    XML_SetElementHandler(xmlparser, sharedStrings_main_start_element, NULL);
  }
}

void sharedStrings_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  FILE *sharedStrings_file_callbackdata = callbackdata;
  if (strcmp(name, "si") == 0) {
    fprintf(sharedStrings_file_callbackdata, "<td>");
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, sharedStrings_main_end_element);
  }
}

void sharedStrings_lv1_end_element(void *callbackdata, const XML_Char *name) {
  FILE *sharedStrings_file_callbackdata = callbackdata;
  if (strcmp(name, "si") == 0) {
    fprintf(sharedStrings_file_callbackdata, "</td>");
    XML_SetElementHandler(xmlparser, sharedStrings_lv1_start_element, sharedStrings_main_end_element);
  }
}



void sharedStrings_lv2_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  FILE *sharedStrings_file_callbackdata = callbackdata;
  if (strcmp(name, "t") == 0) {
    if (font.name != NULL) {
      printf("---------------------------------------------\n");
      printf("FONT SIZE: %f\n", font.sz);
      printf("FONT NAME: %s\n", font.name);
      printf("FONT IS BOLD: %c\n", font.isBold);
      printf("FONT IS ITALIC: %c\n", font.isItalic);
      printf("FONT UNDERLINE: %s\n", font.underline);
      printf("FONT COLOR RGB: %s\n", font.color.rgb);
    }
    fprintf(sharedStrings_file_callbackdata, "<p>");
    
    if (font.name != NULL) {
      free(font.name);
    }
    if (font.color.rgb != NULL) {
      free(font.color.rgb);
    }
    if (font.underline != NULL) {
      free(font.underline);
    }
    // Destroy font obj
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
  if ((value = malloc(len + 1)) == NULL) {
    return;
  } 
  memcpy(value, buf, len);
  value[len] = '\0';
  fputs(value, sharedStrings_file_callbackdata);
  free(value);
}
