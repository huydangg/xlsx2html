#include "book.h"
#include "chart.h"
#include "drawing.h"
#include "relationship.h"
#include "sharedstring.h"
#include "sheet.h"
#include "style.h"

#define PARSE_BUFFER_SIZE 256

int process_zip_file(zip_file_t *, void *, XML_CharacterDataHandler,
                     XML_StartElementHandler, XML_EndElementHandler);
