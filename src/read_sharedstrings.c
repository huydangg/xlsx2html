#include <read_sharedstrings.h>
#include <string.h>

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

struct Font font;

void sharedStrings_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
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
    fprintf(_tmp_sharedStrings_callbackdata, "<p>");
    XML_SetElementHandler(xmlparser, NULL, sharedStrings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, sharedStrings_content_handler);
  } else if(strcmp(name, "rPr") == 0) {
    XML_SetElementHandler(xmlparser, sharedStrings_rPritem_start_element, sharedStrings_rPritem_end_element);
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
    sharedStrings_lv1_end_element(userData, name);
  }
}

void sharedStrings_rPritem_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name, "sz") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font.sz = (int)strtol((char *)attrs[i + 1], NULL, 10);
      }
    }
  }
  if (strcmp(name, "rFont") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font.name = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
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
	font.underline = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(font.underline, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  if (strcmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font.color.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(font.color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
}

void sharedStrings_rPritem_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, sharedStrings_lv1_end_element);
  XML_SetCharacterDataHandler(xmlparser, NULL);
}

void sharedStrings_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0 || buf == NULL){
    return;
  }
  FILE *_tmp_sharedStrings_callbackdata = callbackdata;
  char *value = malloc(len + 1);
  memcpy(value, buf, len + 1);
  fprintf(_tmp_sharedStrings_callbackdata, "%s", value);
  free(value);
}
