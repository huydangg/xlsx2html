#ifndef INCLUDED_READ_RELATIONSHIPS_H
#define INCLUDED_READ_RELATIONSHIPS_H

#include <expat.h>

struct Relationship {
  XML_Char *id;
  XML_Char *target;
  XML_Char *type;
};

struct ArrayRelationships {
  unsigned short length;
  struct Relationship **relationships;
};

extern XML_Parser xmlparser;

void rels_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs);
void rels_end_element(void *callbackdata, const XML_Char *name);

#endif
