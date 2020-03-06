#include <read_styles.h>
#include <read_worksheet.h>
#include <read_workbook.h>
#include <read_sharedstrings.h>

#define PARSE_BUFFER_SIZE 256


int process_zip_file(zip_file_t *, void *, XML_CharacterDataHandler, XML_StartElementHandler, XML_EndElementHandler); 
int generate_columns(struct ArrayCols, const char *, int);
