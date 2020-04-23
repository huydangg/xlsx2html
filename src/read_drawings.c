#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <read_drawings.h>

struct ArrayDrawings array_drawings;

void rollback()	{
  array_drawings.length--;
  free(array_drawings.twocellanchor[array_drawings.length - 1].from.col);
  free(array_drawings.twocellanchor[array_drawings.length - 1].from.colOff);
  free(array_drawings.twocellanchor[array_drawings.length - 1].from.row);
  free(array_drawings.twocellanchor[array_drawings.length - 1].from.rowOff);
  free(array_drawings.twocellanchor[array_drawings.length - 1].to.col);
  free(array_drawings.twocellanchor[array_drawings.length - 1].to.colOff);
  free(array_drawings.twocellanchor[array_drawings.length - 1].to.row);
  free(array_drawings.twocellanchor[array_drawings.length - 1].to.rowOff);
  free(array_drawings.twocellanchor[array_drawings.length - 1].editAs);
  free(array_drawings.twocellanchor[array_drawings.length - 1]);
}

void drawings_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name,"xdr:twoCellAnchor") == 0) {
    array_drawings.length++;
    array_drawings.twocellanchor = realloc(array_drawings.twocellanchor, array_drawings.length * sizeof(struct TwoCellAnchor *));
    array_drawings.twocellanchor[array_drawings.length - 1] = malloc(sizeof(struct TwoCellAnchor));

    for (int i = 0; attrs[i]; i+=2) {
      int len_editAs = strlen(attrs[i + 1]);
      array_drawings.twocellanchor[array_drawings.length - 1].editAs = malloc(len_editAs + 1);
      memcpy(array_drawings.twocellanchor[array_drawings.length - 1].editAs, attrs[i + 1], len_editAs + 1);
    }
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, NULL);

  }
}

void drawings_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name,"xdr:twoCellAnchor") == 0) {
    XML_SetElementHandler(xmlparser, drawings_start_element, drawings_end_element);
  }
}


void drawings_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)atts;
  if (strcmp(name, "xdr:from") == 0) {
    XML_SetUserData(xmlparser, &array_drawings.twocellanchor[array_drawings.length-1].from);
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if(strcmp(name, "xdr:to") == 0) {
    XML_SetUserData(xmlparser, &array_drawings.twocellanchor[array_drawings.length-1].to);
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if(strcmp(name, "xdr:sp") == 0) {
    // Free TwoCellAnchor obj
    rollback()
  }
}

void drawings_lv1_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, drawings_lv1_start_element, NULL);
}

void drawings_lv2_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)atts;
  if (strcmp(name, "xdr:col") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if(strcmp(name, "xdr:colOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if(strcmp(name, "xdr:row") == 0) {
    XML_SetElementHandler(xmlparser, drawings_from_lv2_start_element, NULL);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if(strcmp(name, "xdr:rowOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_from_lv2_start_element, NULL);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  }

}

void drawings_lv2_end_element(void *callbackdata, const XML_Char *name) {

}

void drawings_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }

}
