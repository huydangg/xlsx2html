#include <stdio.h>
#include <zip.h>
#include <expat.h>
#include <string.h>


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

struct Font{
  char color[8];
  XML_Char *family;

}
struct ContentCell {

}
struct ShareString {

}
static void XMLCALL
startElement(void *userData, const XML_Char *name, const XML_Char **attrs) {
  int i;
  int *depthPtr = (int *)userData;
  (void)attrs;
  for (i = 0; i < *depthPtr; i++)
    putchar('\t');
  for(i = 0; attrs[i]; i += 2){
    printf(" %s = '%s'\n", attrs[i], attrs[i + 1]);
  }
  printf("%" XML_FMT_STR "\n", name);
  *depthPtr += 1;
}

static void XMLCALL
endElement(void *userData, const XML_Char *name) {
  int *depthPtr = (int *)userData;
  (void)name;

  *depthPtr -= 1;
}

void content_handler(void *userData, const XML_Char *s, int len){
  if (len == 0){
    return;
  }
  char *value = malloc((len + 1) * sizeof(XML_Char));
  strncpy(value, s, len);
  printf("%s\n", value);
}
void load_share_string(file_name){

}
int main(void){
  zip_t *file= zip_open("/home/huydang/Downloads/excelsample/report__codestringers.xlsx", ZIP_RDONLY, NULL);
  zip_uint64_t num_of_files = zip_get_num_entries(file, ZIP_FL_UNCHANGED);
  zip_file_t *content_type = zip_fopen(file, "xl/sharedStrings.xml", ZIP_FL_UNCHANGED);
  for(zip_uint64_t i=0; i < num_of_files; i++){
    printf("%s\n", zip_get_name(file, i, ZIP_FL_ENC_RAW));
  }
  
  void *buf;
  zip_int64_t buflen;
  XML_Parser parser = XML_ParserCreate(NULL);
  int done;
  enum XML_Status status = XML_STATUS_ERROR;
  int depth = 0;

  XML_SetUserData(parser, &depth);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, content_handler);
  buf = XML_GetBuffer(parser, PARSE_BUFFER_SIZE);
  while(buf && (buflen = zip_fread(content_type, buf, PARSE_BUFFER_SIZE)) >= 0){
    done = buflen < PARSE_BUFFER_SIZE;
    if((status = XML_ParseBuffer(parser, (int)buflen, (done ? 1 : 0))) == XML_STATUS_ERROR){
      fprintf(stderr, "%" XML_FMT_STR " at line %" XML_FMT_INT_MOD "u\n",
              XML_ErrorString(XML_GetErrorCode(parser)),
              XML_GetCurrentLineNumber(parser));
       break;
    }
    if(done){
      printf("DONE");
      break;
    }
    buf = XML_GetBuffer(parser, PARSE_BUFFER_SIZE);
  }
  XML_ParserFree(parser); 
  zip_fclose(content_type);
  zip_close(file);
  return 0;	  
}
