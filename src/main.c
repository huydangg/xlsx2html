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

int process_zip_file(zip_file_t *archive, void *callbackdata);
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
XML_Parser xmlparser = NULL;

struct SheetData {
   XML_Char *name;
   XML_Char *sheet_id;
   char *path_name;
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

struct SheetData sheets_data[50];
struct NumFMT numfmts[50];
struct Font fonts[50];
struct Fill fills[50];
struct BorderCell borders[50];

int count_sheet = 0;
int count_numFmt = 0;
int count_font = 0;
int count_fill = 0;
int count_border = 0;

char *insert_substr_to_str_at_pos(char *des, char *substr, int pos) {
  char *_tmp_sheet_id = malloc(strlen(substr));
  strcpy(_tmp_sheet_id, substr);
  char *_tmp_path_name = malloc(strlen(_tmp_sheet_id) + strlen(des) + 1);
  strncpy(_tmp_path_name, des, pos);
  _tmp_path_name[pos] = '\0';
  strcat(_tmp_path_name, _tmp_sheet_id);
  strcat(_tmp_path_name, des + pos);
  free(_tmp_sheet_id);
  return _tmp_path_name;
}

static void XMLCALL
startElement(void *userData, const XML_Char *name, const XML_Char **attrs) {
  int i;
  (void)attrs;

  if (strcmp(name, "Override") == 0){
    for(i = 0; attrs[i]; i += 2){
      if(strcmp(attrs[i], "PartName") == 0){
        printf(" %s = '%s'\n", attrs[i], attrs[i + 1]);
      }
    }
  }
  if (strcmp(name, "sheet") == 0){
    struct SheetData *sheets_data_callbackdata = userData;
    count_sheet++;
    for(i = 0; attrs[i]; i += 2){
      if(strcmp(attrs[i], "state") == 0 && strcmp(attrs[i + 1], "hidden") == 0){
	count_sheet--;
	return;
      }
      if (strcmp(attrs[i], "name") == 0){
	sheets_data_callbackdata[count_sheet - 1].name = malloc(strlen(attrs[i + 1]));
	memcpy(sheets_data_callbackdata[count_sheet - 1].name, attrs[i + 1], strlen(attrs[i + 1]));
      }
      if (strcmp(attrs[i], "sheetId") == 0){
	sheets_data_callbackdata[count_sheet - 1].sheet_id = malloc(strlen(attrs[i + 1]) + 1);
	sheets_data_callbackdata[count_sheet - 1].sheet_id[strlen(attrs[i + 1])] = '\0';
	char *pattern_name = "xl/worksheets/sheet.xml";
	sheets_data_callbackdata[count_sheet - 1].path_name = insert_substr_to_str_at_pos(pattern_name, attrs[i + 1], 19);
	memcpy(sheets_data_callbackdata[count_sheet - 1].sheet_id, attrs[i + 1], strlen(attrs[i + 1]));
      }
    } 
  }
  if (strcmp(name, "numFmt") == 0){
    struct NumFMT *numFmts_callbackdata = userData;
    count_numFmt++;
    for (i = 0; attrs[i]; i += 2){
      if (strcmp(attrs[i], "formatCode") == 0){
        numFmts_callbackdata[count_numFmt - 1].format_code = malloc(strlen(attrs[i + 1]));
	memcpy(numFmts_callbackdata[count_numFmt - 1].format_code, attrs[i + 1], strlen(attrs[i + 1]));
      }
      if (strcmp(attrs[i], "numFmtId") == 0){
        numFmts_callbackdata[count_numFmt - 1].format_id = malloc(strlen(attrs[i + 1]));
	memcpy(numFmts_callbackdata[count_numFmt - 1].format_id, attrs[i + 1], strlen(attrs[i + 1]));
      }
    }
  }
  if(strcmp(name, "fonts") == 0){
    XML_SetUserData(xmlparser, &fonts);
    XML_SetElementHandler(xmlparser, font_main_start_element, NULL);
  }
  if (strcmp(name, "fills") == 0) {
    XML_SetUserData(xmlparser, &fills);
    XML_SetElementHandler(xmlparser, fill_main_start_element, NULL);
  }
  if (strcmp(name, "borders") == 0) {
    XML_SetUserData(xmlparser, &borders);
    XML_SetElementHandler(xmlparser, border_main_start_element, NULL);
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

static void XMLCALL font_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  if (strcmp(name, "font") == 0){
    count_font++; 
    printf("count_font: %d", count_font);
    XML_SetElementHandler(xmlparser, font_item_start_element, NULL);
  }
}

static void XMLCALL font_main_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "font") == 0){
   XML_SetElementHandler(xmlparser, font_main_start_element, endElement); }
}

static void XMLCALL font_item_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Font *fonts_callbackdata = userData;
  int i;
  if (strcmp(name, "sz") == 0) {
    for (i = 0; attrs[i]; i += 2) {
      if(strcmp(attrs[i], "val") == 0){
	fonts_callbackdata[count_font - 1].size = (int)strtol((char *)attrs[i + 1], NULL, 10);
      }
    }
  } else if (strcmp(name, "name") == 0) {
    for (i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fonts_callbackdata[count_font - 1].name = malloc(strlen(attrs[i + 1]));
	memcpy(fonts_callbackdata[count_font - 1].name, attrs[i + 1], strlen(attrs[i + 1])); 
      }
    }
  } else if (strcmp(name, "b") == 0) {
    for (i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
        fonts_callbackdata[count_font - 1].is_bold = strcmp(attrs[i + 1], "true") == 0 ? 1 : 0;
      }
    }
  } else if (strcmp(name, "i") == 0) {
    for (i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fonts_callbackdata[count_font - 1].is_italic = strcmp(attrs[i + 1], "true") == 0 ? 1 : 0;
      }
    }
  } else if (strcmp(name, "u") == 0) {
    for (i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fonts_callbackdata[count_font - 1].underline = malloc(strlen(attrs[i + 1]));
	memcpy(fonts_callbackdata[count_font - 1].underline, attrs[i + 1], strlen(attrs[i + 1]));
      }
    }
  } else if (strcmp(name, "color") == 0) {
    for (i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "rgb") == 0) {
	fonts_callbackdata[count_font - 1].color.rgb = malloc(strlen(attrs[i + 1]));
	memcpy(fonts_callbackdata[count_font - 1].color.rgb, attrs[i + 1], strlen(attrs[i + 1]));
      }
    }
  } else if(strcmp(name, "family") == 0){

  } else if(strcmp(name, "charset") == 0){

  }
  XML_SetElementHandler(xmlparser, NULL, font_main_end_element);

}

static void XMLCALL font_item_end_element(void *userData, const XML_Char *name) {

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
        fills_callbackdata[count_fill - 1].pattern_fill.pattern_type = malloc(strlen(attrs[i + 1]));
        memcpy(fills_callbackdata[count_fill - 1].pattern_fill.pattern_type, attrs[i + 1], strlen(attrs[i + 1]));
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
        fills_callbackdata[count_fill - 1].pattern_fill.bg_color.rgb = malloc(strlen(attrs[i + 1]));
        memcpy(fills_callbackdata[count_fill - 1].pattern_fill.bg_color.rgb, attrs[i + 1], strlen(attrs[i + 1]));
      }
    }
  } else if (strcmp(name, "fgColor") == 0) {
     for (int i = 0; attrs[i]; i += 2) {
       if (strcmp(attrs[i], "rgb") == 0) {
         fills_callbackdata[count_fill - 1].pattern_fill.fg_color.rgb = malloc(strlen(attrs[i + 1]));
         memcpy(fills_callbackdata[count_fill - 1].pattern_fill.fg_color.rgb, attrs[i + 1], strlen(attrs[i + 1]));
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
    XML_SetElementHandler(xmlparser, border_item_lv1_start_element, NULL);
  }
}

static void XMLCALL border_main_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, border_main_start_element, endElement);
}

static void XMLCALL border_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct BorderCell *borders_callbackdata = userData;
  borders_callbackdata[count_border - 1].left.style = NULL;
  borders_callbackdata[count_border - 1].right.style = NULL;
  borders_callbackdata[count_border - 1].top.style = NULL;
  borders_callbackdata[count_border - 1].bottom.style = NULL;
  if (strcmp(name, "left") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        printf("%s\n", attrs[i + 1]);
        borders_callbackdata[count_border - 1].left.style = malloc(strlen(attrs[i + 1]));
	memcpy(borders_callbackdata[count_border - 1].left.style, attrs[i + 1], strlen(attrs[i + 1]));
      }
    }
  } else if (strcmp(name, "right") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[count_border - 1].right.style = malloc(strlen(attrs[i + 1]));
	memcpy(borders_callbackdata[count_border - 1].right.style, attrs[i + 1], strlen(attrs[i + 1]));
      }
    }
  } else if (strcmp(name, "top") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[count_border - 1].top.style = malloc(strlen(attrs[i + 1]));
	memcpy(borders_callbackdata[count_border - 1].top.style, attrs[i + 1], strlen(attrs[i + 1]));
      }
    }
  } else if (strcmp(name, "bottom") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[count_border - 1].bottom.style = malloc(strlen(attrs[i + 1]));
	memcpy(borders_callbackdata[count_border - 1].bottom.style, attrs[i + 1], strlen(attrs[i + 1]));
      }
    }
  } else if (strcmp(name, "diagonal") == 0) {
  }
  XML_SetElementHandler(xmlparser, border_item_lv2_start_element, border_item_lv1_end_element);
}

static void XMLCALL border_item_lv1_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, border_item_lv1_start_element, border_main_end_element);
}

static void XMLCALL border_item_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct BorderCell *borders_callbackdata = userData;
  struct Border *_tmp_border;
  if (borders_callbackdata[count_border - 1].left.style != NULL) {
    _tmp_border = &borders_callbackdata[count_border - 1].left;
  } else if (borders_callbackdata[count_border - 1].right.style != NULL) {
    _tmp_border = &borders_callbackdata[count_border - 1].right;
  } else if (borders_callbackdata[count_border - 1].top.style != NULL) {
    _tmp_border = &borders_callbackdata[count_border - 1].top;
  } else if (borders_callbackdata[count_border - 1].bottom.style != NULL) {
    _tmp_border = &borders_callbackdata[count_border - 1].bottom;
  }
  if(strcmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "rgb") == 0) {
        _tmp_border->border_color.rgb = malloc(strlen(attrs[i + 1]));
	memcpy(_tmp_border->border_color.rgb, attrs[i + 1], strlen(attrs[i + 1]));
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, border_item_lv2_end_element);
}

static void XMLCALL border_item_lv2_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, NULL, border_item_lv1_end_element);
}

void content_handler(void *userData, const XML_Char *s, int len) {
  if (len == 0){
    return;
  }
  char *value = malloc((len + 1) * sizeof(XML_Char));
  strncpy(value, s, len);
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
  int status = process_zip_file(archive, &sheets_data);
  for(int i = 0; i < count_sheet; i++) {
    printf("Name %s\n", sheets_data[i].name);
    printf("sheetID: %s\n", sheets_data[i].sheet_id);
    printf("Path name: %s\n", sheets_data[i].path_name);
  }
  return status;
}

int load_styles(zip_t *zip) {
  const char *zip_file_name = "xl/styles.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  // Load NumFMT first
  int status = process_zip_file(archive, &numfmts);
  for (int i = 0; i < count_numFmt; i++) {
    printf("Format code: %s\n", numfmts[i].format_code);
    printf("Format id: %s\n", numfmts[i].format_id);
    free(numfmts[i].format_code);
    free(numfmts[i].format_id);
  }
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
  for (int i = 0; i < count_fill; i++) {
    printf("Fill pattern type: %s\n", fills[i].pattern_fill.pattern_type);
    printf("Fill bg_color rgb: %s\n", fills[i].pattern_fill.bg_color.rgb);
    printf("Fill fg_color rgb: %s\n", fills[i].pattern_fill.fg_color.rgb);
    free(fills[i].pattern_fill.pattern_type);
    free(fills[i].pattern_fill.bg_color.rgb);
    free(fills[i].pattern_fill.fg_color.rgb);
  }
  printf("Count border: %d", count_border);
  for (int i = 0; i < count_border; i++) {
    printf("Border left style: %s\n", borders[i].left.style);
    printf("Border left color rgb: %s\n", borders[i].left.border_color.rgb);
    printf("Border right style: %s\n", borders[i].right.style);
    printf("Border right color rgb: %s\n", borders[i].right.border_color.rgb);
    printf("Border top style: %s\n", borders[i].top.style);
    printf("Border top color rgb: %s\n", borders[i].top.border_color.rgb);
    printf("Border bottom style: %s\n", borders[i].bottom.style);
    printf("Border bottom color rgb: %s\n", borders[i].bottom.border_color.rgb);

  }
  return status;
}

int load_sheet(zip_t *zip, const char *sheet_file_name) {
  return 1;
}

int process_zip_file(zip_file_t *archive, void *callbackdata) {
  void *buf;
  zip_int64_t buflen;
  xmlparser = XML_ParserCreate(NULL);
  int done;
  enum XML_Status status = XML_STATUS_ERROR;

  XML_SetUserData(xmlparser, callbackdata);
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
  const char *file_name = "/Volumes/PUBLIC/excelsample/report__codestringers.xlsx";
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
