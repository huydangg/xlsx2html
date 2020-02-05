#include <stdio.h>
#include <zip.h>
#include <expat.h>
#include <string.h>
#include <stdlib.h>

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
#define PARSE_BUFFER_SIZE 256

int process_zip_file(zip_file_t *archive);
static void XMLCALL sheet_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL sheet_main_end_element(void *userData, const XML_Char *name);
static void XMLCALL numFmt_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL numFmt_main_end_element(void *userData, const XML_Char *name);
static void XMLCALL font_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL font_main_end_element(void *userData, const XML_Char *name);
static void XMLCALL font_item_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL font_item_end_element(void *userData, const XML_Char *name);
static void XMLCALL fill_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL fill_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs); 
static void XMLCALL fill_item_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL fill_item_lv1_end_element(void *userData, const XML_Char *name); 
static void XMLCALL fill_item_lv2_end_element(void *userData, const XML_Char *name);
static void XMLCALL border_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL border_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL border_item_lv1_end_element(void *userData, const XML_Char *name);
static void XMLCALL border_item_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL border_item_lv2_end_element(void *userData, const XML_Char *name);
static void XMLCALL xf_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL xf_main_end_element(void *userData, const XML_Char *name);
static void XMLCALL xf_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
static void XMLCALL xf_item_lv1_end_element(void *userData, const XML_Char *name);
XML_Parser xmlparser = NULL;

struct SheetData {
   XML_Char *name;
   XML_Char *sheet_id;
   char *path_name;
   char isHidden;
};

struct NumFMT {
  XML_Char *format_code;
  XML_Char *format_id;
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

struct PatternFill {
  XML_Char *pattern_type;
  struct Color bg_color;
  struct Color fg_color;
};

struct Fill {
  struct PatternFill pattern_fill;
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

struct SheetData *sheets_data;
struct NumFMT *numfmts;
struct Font *fonts;
struct Fill *fills;
struct BorderCell *borders;
struct Xf *cellStyleXfs;
struct Xf *cellXfs;

int count_sheet = 0;
int count_numFmt = 0;
int count_font = 0;
int count_fill = 0;
int count_border = 0;
int count_cellStyleXfs = 0;
int count_cellXfs = 0;


XML_Char *insert_substr_to_str_at_pos(XML_Char *des, XML_Char *substr, int pos) {
  XML_Char *_tmp_sheet_id = malloc(sizeof(XML_Char) * (strlen(substr) + 1));
  memcpy(_tmp_sheet_id, substr, sizeof(XML_Char) * (strlen(substr) + 1));
  XML_Char *_tmp_path_name = malloc(sizeof(XML_Char) * (strlen(_tmp_sheet_id) + strlen(des) + 1));
  memcpy(_tmp_path_name, des, pos);
  _tmp_path_name[pos] = '\0';
  memcpy(_tmp_path_name + pos, _tmp_sheet_id, sizeof(XML_Char) * (strlen(_tmp_sheet_id) + 1));
  memcpy(_tmp_path_name + pos + strlen(_tmp_sheet_id), des + pos, sizeof(XML_Char) * (strlen(des + pos) + 1));
  free(_tmp_sheet_id);
  return _tmp_path_name;
}

static void XMLCALL
startElement(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;

  if (strcmp(name, "sheets") == 0) {
    sheets_data = malloc(sizeof(struct SheetData));
    XML_SetUserData(xmlparser, sheets_data);
    XML_SetElementHandler(xmlparser, sheet_main_start_element, NULL);
  } else if (strcmp(name, "numFmts") == 0){
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	numfmts = calloc((int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct NumFMT));
      }
    }
    XML_SetUserData(xmlparser, numfmts);
    XML_SetElementHandler(xmlparser, numFmt_main_start_element, NULL);
  } else if(strcmp(name, "fonts") == 0){
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	fonts = calloc((int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct Font));
      }
    }
    XML_SetUserData(xmlparser, fonts);
    XML_SetElementHandler(xmlparser, font_main_start_element, NULL);
  } else if (strcmp(name, "fills") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	fills = calloc((int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct Fill));
      }
    }
    XML_SetUserData(xmlparser, fills);
    XML_SetElementHandler(xmlparser, fill_main_start_element, NULL);
  } else if (strcmp(name, "borders") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	borders = calloc((int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct BorderCell));
      }
    }
    XML_SetUserData(xmlparser, borders);
    XML_SetElementHandler(xmlparser, border_main_start_element, NULL);
  } else if (strcmp(name, "cellStyleXfs") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	cellStyleXfs = calloc((int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct Xf));
      }
    }
    XML_SetUserData(xmlparser, cellStyleXfs);
    XML_SetElementHandler(xmlparser, xf_main_start_element, NULL);
  } else if (strcmp(name, "cellXfs") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	cellXfs  = calloc((int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct Xf));
      }
    }
    XML_SetUserData(xmlparser, cellXfs);
    XML_SetElementHandler(xmlparser, xf_main_start_element, NULL);
  }
}

static void XMLCALL
endElement(void *userData, const XML_Char *name) {
  (void)name;
  if (strcmp(name, "fonts") == 0) {
  } else if (strcmp(name, "fills") == 0) {
  }

  XML_SetElementHandler(xmlparser, startElement, NULL);
}

static void XMLCALL numFmt_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct NumFMT *numFmts_callbackdata = userData;
  if (strcmp(name, "numFmt") == 0) {
    count_numFmt++;
    for (int i = 0; attrs[i]; i += 2){
      if (strcmp(attrs[i], "formatCode") == 0){
        numFmts_callbackdata[count_numFmt - 1].format_code = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(numFmts_callbackdata[count_numFmt - 1].format_code, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
      if (strcmp(attrs[i], "numFmtId") == 0){
        numFmts_callbackdata[count_numFmt - 1].format_id = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(numFmts_callbackdata[count_numFmt - 1].format_id, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, numFmt_main_end_element);
}

static void XMLCALL numFmt_main_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, numFmt_main_start_element, endElement);
}

static void XMLCALL sheet_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct SheetData *sheets_data_callbackdata = userData;
  if (strcmp(name, "sheet") == 0){
    count_sheet++;
    if (count_sheet > 1) {
      sheets_data_callbackdata = realloc(sheets_data_callbackdata, sizeof(struct SheetData) * count_sheet);
    }
    for(int i = 0; attrs[i]; i += 2){
      if(strcmp(attrs[i], "state") == 0){
        sheets_data_callbackdata[count_sheet - 1].isHidden = strcmp(attrs[i + 1], "hidden") == 0 ? '1' : '0';
      }
      if (strcmp(attrs[i], "name") == 0){
	sheets_data_callbackdata[count_sheet - 1].name = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(sheets_data_callbackdata[count_sheet - 1].name, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
      if (strcmp(attrs[i], "sheetId") == 0){
	sheets_data_callbackdata[count_sheet - 1].sheet_id = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(sheets_data_callbackdata[count_sheet - 1].sheet_id, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	char *pattern_name = "xl/worksheets/sheet.xml";
	sheets_data_callbackdata[count_sheet - 1].path_name = insert_substr_to_str_at_pos(pattern_name, attrs[i + 1], 19);
      }
    } 
  }
  XML_SetElementHandler(xmlparser, NULL, sheet_main_end_element);
}

static void XMLCALL sheet_main_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, sheet_main_start_element, endElement);
}

static void XMLCALL font_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  if (strcmp(name, "font") == 0) {
    count_font++;
    XML_SetElementHandler(xmlparser, font_item_start_element, NULL);
  }
}

static void XMLCALL font_main_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "font") == 0) {
   XML_SetElementHandler(xmlparser, font_main_start_element, endElement);
  }
}

static void XMLCALL font_item_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Font *fonts_callbackdata = userData;
  if (strcmp(name, "sz") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if(strcmp(attrs[i], "val") == 0){
        fonts_callbackdata[count_font - 1].size = (int)strtol((char *)attrs[i + 1], NULL, 10);
      }
    }
  } else if (strcmp(name, "name") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
        fonts_callbackdata[count_font - 1].name = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(fonts_callbackdata[count_font - 1].name, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1)); 
      }
    }
  } else if (strcmp(name, "b") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
        fonts_callbackdata[count_font - 1].is_bold = strcmp(attrs[i + 1], "true") == 0 ? 1 : 0;
      }
    }
  } else if (strcmp(name, "i") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fonts_callbackdata[count_font - 1].is_italic = strcmp(attrs[i + 1], "true") == 0 ? 1 : 0;
      }
    }
  } else if (strcmp(name, "u") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fonts_callbackdata[count_font - 1].underline = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(fonts_callbackdata[count_font - 1].underline, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  } else if (strcmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "rgb") == 0) {
	fonts_callbackdata[count_font - 1].color.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(fonts_callbackdata[count_font - 1].color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  } else if(strcmp(name, "family") == 0){

  } else if(strcmp(name, "charset") == 0){

  }
  XML_SetElementHandler(xmlparser, NULL, font_item_end_element);

}

static void XMLCALL font_item_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, font_item_start_element, font_main_end_element);
}

static void XMLCALL fill_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  if (strcmp(name, "fill") == 0) {
    count_fill++;
    XML_SetElementHandler(xmlparser, fill_item_lv1_start_element, NULL);
  }
}

static void XMLCALL fill_main_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "fill") == 0)  {
    XML_SetElementHandler(xmlparser, fill_main_start_element, endElement);
  }
}

static void XMLCALL fill_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Fill *fills_callbackdata = userData;
  if (strcmp(name, "patternFill") == 0)  {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "patternType") == 0) {
        fills_callbackdata[count_fill - 1].pattern_fill.pattern_type = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(fills_callbackdata[count_fill - 1].pattern_fill.pattern_type, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetElementHandler(xmlparser, fill_item_lv2_start_element, fill_item_lv1_end_element);
  } else if (strcmp(name, "gradientFill") == 0) {
    
  }
}

static void XMLCALL fill_item_lv1_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "patternFill") == 0) {

  } else if (strcmp(name, "gradientFill") == 0) {
    
  }
  XML_SetElementHandler(xmlparser, fill_item_lv1_start_element, fill_main_end_element);
}

static void XMLCALL fill_item_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Fill *fills_callbackdata = userData;
  if (strcmp(name, "bgColor") == 0)  {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "rgb") == 0) {
        fills_callbackdata[count_fill - 1].pattern_fill.bg_color.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(fills_callbackdata[count_fill - 1].pattern_fill.bg_color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  } else if (strcmp(name, "fgColor") == 0) {
     for (int i = 0; attrs[i]; i += 2) {
       if (strcmp(attrs[i], "rgb") == 0) {
         fills_callbackdata[count_fill - 1].pattern_fill.fg_color.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
         memcpy(fills_callbackdata[count_fill - 1].pattern_fill.fg_color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
       }
     }   
  }
  XML_SetElementHandler(xmlparser, NULL, fill_item_lv2_end_element);
}

static void XMLCALL fill_item_lv2_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "bgColor") == 0) {
    
  } else if (strcmp(name, "fgColor") == 0) {

  }
  XML_SetElementHandler(xmlparser, fill_item_lv2_start_element, fill_item_lv1_end_element);
}

static void XMLCALL border_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  if (strcmp(name, "border") == 0) {
    count_border++;
  }
  XML_SetElementHandler(xmlparser, border_item_lv1_start_element, border_item_lv1_end_element);
} 

static void XMLCALL border_main_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, border_main_start_element, endElement);
}

static void XMLCALL border_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct BorderCell *borders_callbackdata = userData; 
  if (strcmp(name, "left") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[count_border - 1].left.style = malloc(sizeof(XML_Char) * strlen(attrs[i + 1]) + 1);
	memcpy(borders_callbackdata[count_border - 1].left.style, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[count_border - 1].left);
  } else if (strcmp(name, "right") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[count_border - 1].right.style = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(borders_callbackdata[count_border - 1].right.style, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[count_border - 1].right);
  } else if (strcmp(name, "top") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[count_border - 1].top.style = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(borders_callbackdata[count_border - 1].top.style, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[count_border - 1].top);
  } else if (strcmp(name, "bottom") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[count_border - 1].bottom.style = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(borders_callbackdata[count_border - 1].bottom.style, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[count_border - 1].bottom);
  } else if (strcmp(name, "diagonal") == 0) {
  }
  XML_SetElementHandler(xmlparser, border_item_lv2_start_element, border_item_lv1_end_element);
}

static void XMLCALL border_item_lv1_end_element(void *userData, const XML_Char *name) {
  XML_SetUserData(xmlparser, borders);
  XML_SetElementHandler(xmlparser, border_item_lv1_start_element, border_main_end_element);
}

static void XMLCALL border_item_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Border *border_specific_callbackdata = userData;

  if(strcmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "rgb") == 0) {
        border_specific_callbackdata->border_color.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(border_specific_callbackdata->border_color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, border_item_lv2_end_element);
}

static void XMLCALL border_item_lv2_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, NULL, border_item_lv1_end_element);
}

static void XMLCALL xf_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Xf *xfs_callbackdata = userData;
  if (strcmp(name, "xf") == 0) {
    int _tmp_count;
    if (xfs_callbackdata == cellStyleXfs) {
       _tmp_count = ++count_cellStyleXfs;
    } else if (xfs_callbackdata == cellXfs) {
      _tmp_count = ++count_cellXfs;
    }
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "borderId") == 0) {
        xfs_callbackdata[_tmp_count - 1].borderId = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].borderId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "fillId") == 0) {
        xfs_callbackdata[_tmp_count - 1].fillId	= malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].fillId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "fontId") == 0) {
        xfs_callbackdata[_tmp_count - 1].fontId	= malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].fontId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "numFmtId") == 0) {
        xfs_callbackdata[_tmp_count - 1].numFmtId = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].numFmtId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "xfId") == 0) {
        xfs_callbackdata[_tmp_count - 1].xfId = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].xfId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  XML_SetElementHandler(xmlparser, xf_item_lv1_start_element, xf_main_end_element);
}

static void XMLCALL xf_main_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, xf_main_start_element, endElement);
}

static void XMLCALL xf_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Xf *xfs_callbackdata = userData;
  if (strcmp(name, "alignment") == 0) {
    int _tmp_count;
    if (xfs_callbackdata == cellStyleXfs) {
       _tmp_count = count_cellStyleXfs;
    } else if (xfs_callbackdata == cellXfs) {
      _tmp_count = count_cellXfs;
    }
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "horizontal") == 0) {
	xfs_callbackdata[_tmp_count - 1].alignment.horizontal = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].alignment.horizontal, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "vertical") == 0) {
	xfs_callbackdata[_tmp_count - 1].alignment.vertical = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].alignment.vertical, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "textRotation") == 0) {
	xfs_callbackdata[_tmp_count - 1].alignment.textRotation = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].alignment.textRotation, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "wrapText") == 0) {
        xfs_callbackdata[_tmp_count - 1].alignment.isWrapText = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }

  } else if (strcmp(name, "protection") == 0) {

  }
  XML_SetElementHandler(xmlparser, NULL, xf_item_lv1_end_element);
}

static void XMLCALL xf_item_lv1_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, xf_item_lv1_start_element, xf_main_end_element);
}

void content_handler(void *userData, const XML_Char *s, int len) {
  if (len == 0){
    return;
  }
  char *value = malloc((len + 1) * sizeof(XML_Char));
  strncpy(value, s, len);
  free(value);
}

/*int load_contenttype(zip_t *zip){
  const char *zip_file_name = "[Content_Types].xml";
  int status = process_zip_file(zip, zip_file_name);
  return status;
}*/

zip_t *open_zip(const char *file_name) {
  return zip_open(file_name, ZIP_RDONLY, NULL);
}

zip_file_t *open_zip_file(zip_t *zip, const char *zip_file_name) {
  return zip_fopen(zip, zip_file_name, ZIP_FL_UNCHANGED);
}

int load_workbook(zip_t *zip) {
  const char *zip_file_name = "xl/workbook.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  int status = process_zip_file(archive);
  for(int i = 0; i < count_sheet; i++) {
    printf("Name %s\n", sheets_data[i].name);
    printf("sheetID: %s\n", sheets_data[i].sheet_id);
    printf("Path name: %s\n", sheets_data[i].path_name);
    free(sheets_data[i].name);
    free(sheets_data[i].sheet_id);
    free(sheets_data[i].path_name);
  }
  free(sheets_data);
  return status;
}

int load_styles(zip_t *zip) {
  const char *zip_file_name = "xl/styles.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  // Load NumFMT first
  int status = process_zip_file(archive);
  for (int i = 0; i < count_numFmt; i++) {
    printf("Format code: %s\n", numfmts[i].format_code);
    printf("Format id: %s\n", numfmts[i].format_id);
    free(numfmts[i].format_code);
    free(numfmts[i].format_id);
  }
  free(numfmts);
  printf("Count font: %d\n", count_font);
  for (int i = 0; i < count_font; i++) {
    printf("Font size: %d\n", fonts[i].size);
    printf("Font name: %s\n", fonts[i].name);
    printf("Font is bold: %d\n", fonts[i].is_bold);
    printf("Font is italic: %d\n", fonts[i].is_italic);
    printf("Font underline: %s\n", fonts[i].underline);
    printf("Font color rgb: %s\n", fonts[i].color.rgb);
    free(fonts[i].name);
    free(fonts[i].underline);
    free(fonts[i].color.rgb);
  }
  free(fonts);
  printf("Count fills: %d\n", count_fill);
  for (int i = 0; i < count_fill; i++) {
    printf("Fill pattern type: %s\n", fills[i].pattern_fill.pattern_type);
    printf("Fill bg_color rgb: %s\n", fills[i].pattern_fill.bg_color.rgb);
    printf("Fill fg_color rgb: %s\n", fills[i].pattern_fill.fg_color.rgb);
    free(fills[i].pattern_fill.pattern_type);
    free(fills[i].pattern_fill.bg_color.rgb);
    free(fills[i].pattern_fill.fg_color.rgb);
  }
  free(fills);
  printf("Count border: %d\n", count_border);
  for (int i = 0; i < count_border; i++) {
    printf("---------------------------------------------------------\n");
    printf("Border left style: %s\n", borders[i].left.style);
    printf("Border left color rgb: %s\n", borders[i].left.border_color.rgb);
    printf("Border right style: %s\n", borders[i].right.style);
    printf("Border right color rgb: %s\n", borders[i].right.border_color.rgb);
    printf("Border top style: %s\n", borders[i].top.style);
    printf("Border top color rgb: %s\n", borders[i].top.border_color.rgb);
    printf("Border bottom style: %s\n", borders[i].bottom.style);
    printf("Border bottom color rgb: %s\n", borders[i].bottom.border_color.rgb);
    free(borders[i].left.style);
    free(borders[i].left.border_color.rgb);
    free(borders[i].right.style);
    free(borders[i].right.border_color.rgb);
    free(borders[i].top.style);
    free(borders[i].top.border_color.rgb);
    free(borders[i].bottom.style);
    free(borders[i].bottom.border_color.rgb);
  }
  free(borders);
  for (int i = 0; i < count_cellStyleXfs; i++) {
    printf("---------------------------------------------------------\n");
    printf("Xf borderId: %s\n", cellStyleXfs[i].borderId);
    printf("Xf fillId: %s\n", cellStyleXfs[i].fillId);
    printf("Xf fontId: %s\n", cellStyleXfs[i].fontId);
    printf("Xf numFmtId: %s\n", cellStyleXfs[i].numFmtId);
    printf("Xf alignment horizontal: %s\n", cellStyleXfs[i].alignment.horizontal);
    printf("Xf alignment vertical: %s\n", cellStyleXfs[i].alignment.vertical);
    printf("Xf alignment textRotation: %s\n", cellStyleXfs[i].alignment.textRotation);
    printf("Xf alignment isWrapText: %c\n", cellStyleXfs[i].alignment.isWrapText);
    free(cellStyleXfs[i].borderId);
    free(cellStyleXfs[i].fillId);
    free(cellStyleXfs[i].fontId);
    free(cellStyleXfs[i].numFmtId);
    free(cellStyleXfs[i].alignment.horizontal);
    free(cellStyleXfs[i].alignment.vertical);
    free(cellStyleXfs[i].alignment.textRotation);
  }
  free(cellStyleXfs);
  printf("Count cellXfs: %d\n", count_cellXfs);
  for (int i = 0; i < count_cellXfs; i++) {
    printf("---------------------------------------------------------\n");
    printf("Xf borderId: %s\n", cellXfs[i].borderId);
    printf("Xf fillId: %s\n", cellXfs[i].fillId);
    printf("Xf fontId: %s\n", cellXfs[i].fontId);
    printf("Xf numFmtId: %s\n", cellXfs[i].numFmtId);
    printf("Xf xfId: %s\n", cellXfs[i].xfId);
    printf("Xf alignment horizontal: %s\n", cellXfs[i].alignment.horizontal);
    printf("Xf alignment vertical: %s\n", cellXfs[i].alignment.vertical);
    printf("Xf alignment textRotation: %s\n", cellXfs[i].alignment.textRotation);
    printf("Xf alignment isWrapText: %c\n", cellXfs[i].alignment.isWrapText);
    free(cellXfs[i].borderId);
    free(cellXfs[i].fillId);
    free(cellXfs[i].fontId);
    free(cellXfs[i].numFmtId);
    free(cellXfs[i].xfId);
    free(cellXfs[i].alignment.horizontal);
    free(cellXfs[i].alignment.vertical);
    free(cellXfs[i].alignment.textRotation);
  }
  free(cellXfs);
  return status;
}

int load_sheet(zip_t *zip, const char *sheet_file_name) {
  return 1;
}

int process_zip_file(zip_file_t *archive) {
  void *buf;
  zip_int64_t buflen;
  xmlparser = XML_ParserCreate(NULL);
  int done;
  enum XML_Status status = XML_STATUS_ERROR;

  XML_SetElementHandler(xmlparser, startElement, endElement);
  XML_SetCharacterDataHandler(xmlparser, content_handler);
  buf = XML_GetBuffer(xmlparser, PARSE_BUFFER_SIZE);
  while(buf && (buflen = zip_fread(archive, buf, PARSE_BUFFER_SIZE)) >= 0) {
    done = buflen < PARSE_BUFFER_SIZE;
    if((status = XML_ParseBuffer(xmlparser, (int)buflen, (done ? 1 : 0))) == XML_STATUS_ERROR) {
      fprintf(stderr, "%" XML_FMT_STR " at line %" XML_FMT_INT_MOD "u\n",
              XML_ErrorString(XML_GetErrorCode(xmlparser)),
              XML_GetCurrentLineNumber(xmlparser));
       XML_ParserFree(xmlparser); 
       zip_fclose(archive);
       return 0;
    }
    if(done) {
      break;
    }
    buf = XML_GetBuffer(xmlparser, PARSE_BUFFER_SIZE);
  }
  XML_ParserFree(xmlparser); 
  zip_fclose(archive);
  return 1;
}

int main(void) {
  const char *file_name = "/home/huydang/Downloads/excelsample/report__codestringers.xlsx";
  zip_t *zip = open_zip(file_name);
  if (zip == NULL){
    fprintf(stderr, "File not found");
    return 0;
  }
  int status_workbook = load_workbook(zip);
  if (!status_workbook) {
    fprintf(stderr, "Failed to read workbook");
    zip_close(zip);
    return 0;
  }
  int status_styles = load_styles(zip);
  if (!status_styles) {
    fprintf(stderr, "Failed to read workbook");
    zip_close(zip);
    return 0;
  } 
  zip_close(zip);
  return 0; 
}
