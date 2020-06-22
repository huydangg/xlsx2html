#include <private.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <relationship.h>
#include <errno.h>


void rels_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct ArrayRelationships *array_rels_callbackdata = callbackdata;
  if (XML_Char_icmp(name, "Relationship") == 0) {
    array_rels_callbackdata->length++;
    array_rels_callbackdata->relationships = XML_Char_realloc(
      array_rels_callbackdata->relationships,
      array_rels_callbackdata->length * sizeof(struct Relationship *)
    );
    array_rels_callbackdata->relationships[array_rels_callbackdata->length-1] = XML_Char_malloc(sizeof(struct Relationship));
    for (int i = 0; attrs[i]; i+=2) {
      if (XML_Char_icmp(attrs[i], "Id") == 0) {
	int len_id = XML_Char_len(attrs[i + 1]);
        array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->id = XML_Char_malloc(len_id + 1);
	memcpy(array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->id, attrs[i + 1], len_id + 1);
      } else if (XML_Char_icmp(attrs[i], "Target") == 0) {
	int len_target = XML_Char_len(attrs[i + 1]);
        array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->target = XML_Char_malloc(len_target + 1);
	memcpy(array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->target, attrs[i + 1], len_target + 1);
      } else if (XML_Char_icmp(attrs[i], "Type") == 0) {
	int len_type = XML_Char_len(attrs[i + 1]);
	array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->type = XML_Char_malloc(len_type + 1);
	memcpy(array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->type, attrs[i + 1], len_type + 1);
      }
    }
  }
  XML_SetElementHandler(xmlparser, rels_start_element, rels_end_element);
}

void rels_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "Relationship") == 0) {
    XML_SetElementHandler(xmlparser, rels_start_element, NULL);
  }
}
