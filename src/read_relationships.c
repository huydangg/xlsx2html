#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <read_relationships.h>


void rels_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct ArrayRelationships *array_rels_callbackdata = callbackdata;
  if (strcmp(name, "Relationship") == 0) {
    array_rels_callbackdata->length++;
    array_rels_callbackdata->relationships = realloc(array_rels_callbackdata->relationships, array_rels_callbackdata->length * sizeof(struct Relationship *));
    array_rels_callbackdata->relationships[array_rels_callbackdata->length-1] = malloc(sizeof(struct Relationship));
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "Id") == 0) {
	printf("IDDDDDDDDDDDDDDDD: %s\n", attrs[i + 1]);
	int len_id = strlen(attrs[i + 1]);
        array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->id = malloc(len_id + 1);
	memcpy(array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->id, attrs[i + 1], len_id + 1);
      } else if (strcmp(attrs[i], "Target") == 0) {
	int len_target = strlen(attrs[i + 1]);
        array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->target = malloc(len_target + 1);
	memcpy(array_rels_callbackdata->relationships[array_rels_callbackdata->length-1]->target, attrs[i + 1], len_target + 1);
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, rels_end_element);
}

void rels_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "Relationship") == 0) {
    XML_SetElementHandler(xmlparser, rels_start_element, NULL);
  }
}
