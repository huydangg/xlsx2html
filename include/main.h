#include <read_styles.h>
#include <read_worksheet.h>
#include <read_workbook.h>
#include <read_sharedstrings.h>
#include <read_relationships.h>
#include <read_drawings.h>
#include <read_chart.h>
#include <private.h>

#define PARSE_BUFFER_SIZE 256


zip_file_t *open_zip_file(zip_t *, const char *);
int process_zip_file(XML_Parser *, zip_file_t *, void *, XML_CharacterDataHandler, XML_StartElementHandler, XML_EndElementHandler); 

