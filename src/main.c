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
XML_Parser *xmlparser;

struct SheetData {
   XML_Char *name;
   XML_Char *sheet_id;
   char *path_name;
};

struct NumFMT {
  XML_Char *format_code;
  XML_Char *format_id;
};

struct Color{
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

struct Fill {
  XML_Char *pattern_type;
  struct Color bg_color;
};

struct Border{
  XML_Char *style;
  struct Color border_color;
};

struct BorderCell{
  struct Border left;
  struct Border right;
  struct Border top;
  struct Border bottom;
};

struct SheetData sheets_data[50];
struct NumFMT numfmts[50];
struct Font fonts[50];
struct Fill fills[50];
struct Border borders[50];

int count_sheet = 0;
int count_numFmt = 0;

char *insert_substr_to_str_at_pos(char *des, char *substr, int pos){
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
	sheets_data_callbackdata[count_sheet - 1].sheet_id = malloc(strlen(attrs[i + 1]));
	char *pattern_name = "xl/worksheets/sheet.xml";
	sheets_data_callbackdata[count_sheet - 1].path_name = insert_substr_to_str_at_pos(pattern_name, attrs[i + 1], 19);
	memcpy(sheets_data_callbackdata[count_sheet - 1].sheet_id, attrs[i + 1], strlen(attrs[i + 1]));
      }

    } 
  }
  if (strcmp(name, "numFmts")){
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
  if (strcmp(name, "font")){
    for (i = 0; attrs[i]; i += 2) {
    }
  }
  /*printf("%" XML_FMT_STR "\n", name);*/
}

static void XMLCALL
endElement(void *userData, const XML_Char *name) {
  (void)name;
  if (strcmp(name, "numFmts") == 0){
    userData = fonts;
  }

}

void content_handler(void *userData, const XML_Char *s, int len){
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

zip_t *open_zip(const char *file_name){
  return zip_open(file_name, ZIP_RDONLY, NULL);
}

zip_file_t *open_zip_file(zip_t *zip, const char *zip_file_name){
  return zip_fopen(zip, zip_file_name, ZIP_FL_UNCHANGED);
}

int load_workbook(zip_t *zip){
  const char *zip_file_name = "xl/workbook.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  int status = process_zip_file(archive, &sheets_data);
  for(int i = 0; i < count_sheet; i++){
    printf("Name %" XML_FMT_STR "\n", sheets_data[i].name);
    printf("sheetID: %s\n", sheets_data[i].sheet_id);
    printf("Path name: %s\n", sheets_data[i].path_name);
  }
  return status;
}

int load_styles(zip_t *zip){
  const char *zip_file_name = "xl/styles.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  // Load NumFMT first
  int status = process_zip_file(archive, &numfmts);
  for (int i = 0; i < count_numFmt; i++){
    printf("Format code: %s\n", numfmts[i].format_code);
    printf("Format id: %s\n", numfmts[i].format_id);
  }
  return status;
}

int load_sheet(zip_t *zip, const char *sheet_file_name){
  return 1;
}

int process_zip_file(zip_file_t *archive, void *callbackdata){
  void *buf;
  zip_int64_t buflen;
  XML_Parser parser = XML_ParserCreate(NULL);
  int done;
  enum XML_Status status = XML_STATUS_ERROR;

  XML_SetUserData(parser, callbackdata);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, content_handler);
  buf = XML_GetBuffer(parser, PARSE_BUFFER_SIZE);
  while(buf && (buflen = zip_fread(archive, buf, PARSE_BUFFER_SIZE)) >= 0){
    done = buflen < PARSE_BUFFER_SIZE;
    if((status = XML_ParseBuffer(parser, (int)buflen, (done ? 1 : 0))) == XML_STATUS_ERROR){
      fprintf(stderr, "%" XML_FMT_STR " at line %" XML_FMT_INT_MOD "u\n",
              XML_ErrorString(XML_GetErrorCode(parser)),
              XML_GetCurrentLineNumber(parser));
       XML_ParserFree(parser); 
       zip_fclose(archive);
       return 0;
    }
    if(done){
      break;
    }
    buf = XML_GetBuffer(parser, PARSE_BUFFER_SIZE);
  }
  XML_ParserFree(parser); 
  zip_fclose(archive);
  return 1;
}

int main(void){
  const char *file_name = "/home/huydang/Downloads/excelsample/report__codestringers.xlsx";
  zip_t *zip = open_zip(file_name);
  if (zip == NULL){
    fprintf(stderr, "File not found");
    return 0;
  }
  int status_workbook = load_workbook(zip);
  if (!status_workbook){
    fprintf(stderr, "Failed to read workbook");
    zip_close(zip);
    return 0;
  }
  int status_styles = load_styles(zip);
  if (!status_styles){
    fprintf(stderr, "Failed to read workbook");
    zip_close(zip);
    return 0;
  } 

  zip_close(zip);
  return 0; 
}
