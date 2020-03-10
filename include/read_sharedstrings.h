#ifndef INCLUDED_READ_SHAREDSTRINGS_H
#define INCLUDED_READ_SHAREDSTRINGS_H

#include <private.h>

//Mapping index with position.

extern XML_Parser xmlparser;
struct Font new_font();
extern long int *sharedStrings_position;

void sharedStrings_main_start_element(void *, const XML_Char *, const XML_Char **);
void sharedStrings_main_end_element(void *, const XML_Char *);
void sharedStrings_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs);
void sharedStrings_lv1_end_element(void *userData, const XML_Char *name);
void sharedStrings_lv2_start_element(void *, const XML_Char *, const XML_Char **);
void sharedStrings_lv2_end_element(void *userData, const XML_Char *name);

void sharedStrings_rPritem_start_element(void *, const XML_Char *, const XML_Char **);
void sharedStrings_rPritem_end_element(void *userData, const XML_Char *name);
void sharedStrings_content_handler(void *, const XML_Char *, int);

#endif
