#include <style.h>
#include <sheet.h>
#include <book.h>
#include <sharedstring.h>
#include <relationship.h>
#include <drawing.h>
#include <chart.h>

#define PARSE_BUFFER_SIZE 256


int process_zip_file(zip_file_t *, void *, XML_CharacterDataHandler, XML_StartElementHandler, XML_EndElementHandler);

