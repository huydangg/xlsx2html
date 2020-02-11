#include <zip.h>
#include <expat.h>

#define PARSE_BUFFER_SIZE 256

int process_zip_file(zip_file_t *archive, void *callbackdata, XML_StartElementHandler start_element, XML_EndElementHandler end_element); 
