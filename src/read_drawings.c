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
    XML_SetUserData(xmlparser, &array_drawings.twocellanchor[array_drawings.length - 1].from);
    XML_SetUserData(xmlparser, &array_drawings.twocellanchor[array_drawings.length-1].from);
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if(strcmp(name, "xdr:to") == 0) {
    XML_SetUserData(xmlparser, &array_drawings.twocellanchor[array_drawings.length - 1].to);
    XML_SetUserData(xmlparser, &array_drawings.twocellanchor[array_drawings.length-1].to);
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if(strcmp(name, "xdr:sp") == 0) {
    // Free TwoCellAnchor obj if this one is not a picture
    rollback();
  }
}

void drawings_lv1_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "xdr:from") == 0) {
    XML_SetUserData(xmlparser, NULL);
  } else if (strcmp(name, "xdr:to") == 0) {
    XML_SetUserData(xmlparser, NULL);
  }
  XML_SetElementHandler(xmlparser, drawings_lv1_start_element, NULL);
}

void drawings_lv2_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)atts;
  if (strcmp(name, "xdr:col") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
    XML_SetCharacterDataHandler(xmlparser, drawings_col_content_handler);
  } else if(strcmp(name, "xdr:colOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
    XML_SetCharacterDataHandler(xmlparser, drawings_colOff_content_handler);
  } else if(strcmp(name, "xdr:row") == 0) {
    XML_SetElementHandler(xmlparser, drawings_from_lv2_start_element, NULL);
    XML_SetCharacterDataHandler(xmlparser, drawings_row_content_handler);
  } else if(strcmp(name, "xdr:rowOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_from_lv2_start_element, NULL);
    XML_SetCharacterDataHandler(xmlparser, drawings_rowOff_content_handler);
  }
}

void drawings_lv2_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "xdr:col") == 0) {
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:colOff") == 0) {
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:row") == 0) {
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:rowOff") == 0) {
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
  XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
}

void drawings_col_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  struct Offset *offset_callbackdata = callbackdata;
  if (offset_callbackdata->col == NULL) {
    if ((offset_callbackdata->col = realloc(offset_callbackdata->col, len + 1)) == NULL) {
      return;
    }
    memcpy(offset_callbackdata->col, buf, len);
    offset_callbackdata->col[len] = '\0';
  } else {
    int len_offset_col = strlen(offset_callbackdata->col);
    if ((offset_callbackdata->col = realloc(offset_callbackdata->col, len_offset_col + len)) == NULL) {
      return;
    }
    memcpy(offset_callbackdata->col + len_offset_col - 1, buf, len);
    offset_callbackdata->col[len_offset_col + len - 1] = '\0';
  }
}
void drawings_colOff_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  struct Offset *offset_callbackdata = callbackdata;
  if (offset_callbackdata->colOff == NULL) {
    if ((offset_callbackdata->colOff = realloc(offset_callbackdata->colOff, len + 1)) == NULL) {
      return;
    }
    memcpy(offset_callbackdata->colOff, buf, len);
    offset_callbackdata->colOff[len] = '\0';
  } else {
    int len_offset_colOff = strlen(offset_callbackdata->colOff);
    if ((offset_callbackdata->colOff = realloc(offset_callbackdata->colOff, len_offset_colOff + len)) == NULL) {
      return;
    }
    memcpy(offset_callbackdata->colOff + len_offset_colOff - 1, buf, len);
    offset_callbackdata->colOff[len_offset_colOff + len - 1] = '\0';
  }
}
void drawings_row_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  struct Offset *offset_callbackdata = callbackdata;
  if (offset_callbackdata->row == NULL) {
    if ((offset_callbackdata->row = realloc(offset_callbackdata->row, len + 1)) == NULL) {
      return;
    }
    memcpy(offset_callbackdata->row, buf, len);
    offset_callbackdata->row[len] = '\0';
  } else {
    int len_offset_row = strlen(offset_callbackdata->rwo);
    if ((offset_callbackdata->row = realloc(offset_callbackdata->row, len_offset_row + len)) == NULL) {
      return;
    }
    memcpy(offset_callbackdata->row + len_offset_row - 1, buf, len);
    offset_callbackdata->row[len_offset_row + len - 1] = '\0';
  }
}
void drawings_rowOff_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  struct Offset *offset_callbackdata = callbackdata;
  if (offset_callbackdata->rowOff == NULL) {
    if ((offset_callbackdata->rowOff = realloc(offset_callbackdata->rowOff, len + 1)) == NULL) {
      return;
    }
    memcpy(offset_callbackdata->rowOff, buf, len);
    offset_callbackdata->rowOff[len] = '\0';
  } else {
    int len_offset_rowOff = strlen(offset_callbackdata->rowOff);
    if ((offset_callbackdata->rowOff = realloc(offset_callbackdata->rowOff, len_offset_rowOff + len)) == NULL) {
      return;
    }
    memcpy(offset_callbackdata->rowOff + len_offset_rowOff - 1, buf, len);
    offset_callbackdata->rowOff[len_offset_rowOff + len - 1] = '\0';
  }
}
