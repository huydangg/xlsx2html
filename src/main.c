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

int process_zip_file(zip_t *zip, const char *zip_file_name, void *callbackdata);

struct SheetData {
   XML_Char *name;
   XML_Char *sheet_id;
   int is_hidden;
   char *path_name;
   int index_sheet;
};

static void XMLCALL
startElement(void *userData, const XML_Char *name, const XML_Char **attrs) {
  int i;
  (void)attrs;
  struct SheetData *sheets_data = userData;

  /*for (i = 0; i < *depthPtr; i++)
    putchar('\t');
  */
  if (strcmp(name, "Override") == 0){
    for(i = 0; attrs[i]; i += 2){
      if(strcmp(attrs[i], "PartName") == 0){
        printf(" %s = '%s'\n", attrs[i], attrs[i + 1]);
      }
    }
  }
  if (strcmp(name, "sheet") == 0){
    for(i = 0; attrs[i]; i += 2){
      if (strcmp(attrs[i], "name") == 0){
	sheets_data->name = malloc(sizeof(attrs[i + 1]));
	memcpy(sheets_data->name, attrs[i + 1], sizeof(attrs[i + 1]));
      }
      if (strcmp(attrs[i], "sheetId") == 0){
	sheets_data->sheet_id = malloc(sizeof(attrs[i + 1]));
	memcpy(sheets_data->sheet_id, attrs[i + 1], sizeof(attrs[i + 1]));
      }
      if (strcmp(attrs[i], "state") == 0){
	sheets_data->is_hidden = strcmp(attrs[i + 1], "hidden") == 0 ? 1 : 0;
      }
    } 
  }
  
  /*printf("%" XML_FMT_STR "\n", name);*/
}

static void XMLCALL
endElement(void *userData, const XML_Char *name) {
  (void)name;

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

int load_workbook(zip_t *zip){
  const char *zip_file_name = "xl/workbook.xml";
  struct SheetData sheets_data;
  int status = process_zip_file(zip, zip_file_name, &sheets_data);
  printf("Name %s\n", sheets_data.name);
  printf("sheetID: %s\n", sheets_data.sheet_id);
  printf("is hidden? %d\n", sheets_data.is_hidden);
  return status;
}

int process_zip_file(zip_t *zip, const char *zip_file_name, void *callbackdata){
  zip_file_t *archive = zip_fopen(zip, zip_file_name, ZIP_FL_UNCHANGED);
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

  zip_close(zip);
  return 0; 
}
