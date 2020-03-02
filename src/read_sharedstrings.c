#include <read_sharedstrings.h>
#include <string.h>


void sharedStrings_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  FILE *_tmp_sharedStrings_callbackdata = userData;
  if (strcmp(name, "si") == 0){
      fwrite("<td>", 4, 4, _tmp_sharedStrings_callbackdata);
  }
  XML_SetElementHandler(xmlparser, sharedStrings_main_start_element, NULL);
}

void sharedStrings_main_end_element(void *userData, const XML_Char *name) {
  (void)attrs;
  FILE *_tmp_sharedStrings_callbackdata = userData;
  fwrite("</td>", 5, 5, _tmp_sharedStrings_callbackdata);
  XML_SetElementHandler(xmlparser, sharedStrings_main_start_element, NULL);
}

void sharedStrings_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  FILE *_tmp_sharedStrings_callbackdata = userData;
  if (strcmp(name, "t") == 0) {
    fwrite("<p>", 3, 3, _tmp_sharedStrings_callbackdata);
    XML_SetCharacterHandler(xmlparser, sharedStrings_contetn_handler);
  } else if(strcmp(name, "r") == 0) {
    XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, NULL);
  }
}

void sharedStrings_lv1_end_element(void *userData, const XML_Char *name) {
  (void)attrs;
  FILE *_tmp_sharedStrings_callbackdata = userData;
  if (strcmp(name, "t") == 0) {
    fwrite("</p>", 4, 4, _tmp_sharedStrings_callbackdata);
    XML_SetElementHandler(xmlparser, sharedStrings_main_end_element, NULL);
  } else if (strcmp(name, "r") == 0) {
    XML_SetElementHandler(xmlparser, sharedStrings_lv1_start_element, sharedStrings_main_end_element);
  }
}

void sharedStrings_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  FILE *_tmp_sharedStrings_callbackdata = userData;
  if(strcmp(name, "rPr") == 0) {
    struct Font font; 
    XML_SetUserData(xmlparser, &font);
    XML_SetElementHandler(xmlparser, sharedStrings_rPr_attrs_start_element, NULL);
  } else if (strcmp(name, "t") == 0) {
    fwrite("<p>", 3, 3, _tmp_sharedStrings_callbackdata);
    XML_SetCharacterHandler(xmlparser, sharedStrings_content_handler);
  }
}

void sharedStrings_lv2_end_element(void *userData, const XML_Char *name) {
  (void)attrs;
  XML_SetElementHandler(xmlparser, sharedStrings_lv2_start_element, sharedStrings_lv1_end_element);
}

void sharedStrings_rPr_attrs_start_element(void *userData, const XML_Char *name const XML_Char **attrs) {
  (void)attrs;
  struct Font font_callbackdata = userData;
  if (strcmp(name, "sz") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font_callbackdata.sz = (int)strtol((char *)attrs[i + 1], NULL, 10);
      }
    }
  }
  if (strcmp(name, "rFont") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font_callbackdata.name = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(font_callbackdata.name, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  if (strcmp(name, "b") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font_callbackdata.isBold = strcmp(attrs[i + 1], "true") == 0 ? 1 : 0;
      }
    }
  }
  if (strcmp(name, "i") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font_callbackdata.isItalic = strcmp(attrs[i + 1], "true") == 0 ? 1 : 0;
      }
    }
  }
  if (strcmp(name, "u") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font_callbackdata.underline = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(font_callbackdata.underline, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  if (strcmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	font_callbackdata.color.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(font_callbackdata.color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
}

void sharedStrings_rPr_attrs_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, sharedStrings_rPr_start_element, sharedStrings_lv2_end_element);
}

void sharedStrings_content_handler(void *userData, const XML_Char *s, int len) {
  if (len == 0){
    return;
  }
  FILE *_tmp_sharedStrings_callbackdata = userData;
  char *value = malloc((len + 1) * sizeof(XML_Char));
  fwrite(value, len * sizeof(XML_Char), len, _tmp_sharedStrings_callbackdata);
  free(value);
}

