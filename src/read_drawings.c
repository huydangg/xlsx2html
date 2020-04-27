#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <read_drawings.h>

struct TwoCellAnchor *twocellanchor;


void drawings_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name,"xdr:twoCellAnchor") == 0) {
    twocellanchor = malloc(sizeof(TwoCellAnchor));

    for (int i = 0; attrs[i]; i+=2) {
      int len_editAs = strlen(attrs[i + 1]);
      twocellanchor->editAs = malloc(len_editAs + 1);
      memcpy(twocellanchor->editAs, attrs[i + 1], len_editAs + 1);
    }
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, NULL);
  }
}

void drawings_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name,"xdr:twoCellAnchor") == 0) {
    //TODO: Record image html syntax with data from rels drawings to findexhtml
    //At last, free twocellanchor obj
    XML_SetElementHandler(xmlparser, drawings_start_element, drawings_end_element);
  }
}


void drawings_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)atts;
  if (strcmp(name, "xdr:from") == 0) {
    XML_SetUserData(xmlparser, twocellanchor->from);
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if (strcmp(name, "xdr:to") == 0) {
    XML_SetUserData(xmlparser, twocellanchor->to);
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if (strcmp(name, "xdr:pic") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
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
  } else if (strcmp(name, "xdr:nvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (strcmp(name, "xdr:blipFill") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (strcmp(name, "xdr:spPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  }
}

void drawings_lv2_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "xdr:col") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:colOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:row") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:rowOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (strcmp(name, "xdr:nvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  } else if (strcmp(name, "xdr:blipFill") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  } else if (strcmp(name, "xdr:spPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  }
}

void drawings_lv3_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)atts;
  if (strcmp(name,"xdr:cNvPr") == 0) {
    for (int i=0 ; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "name") == 0) {
        int len_name = strlen(attrs[i + 1]);
        twocellanchor->pic.name = malloc(len_name + 1);
        memcpy(twocellanchor->pic.name, attrs[i + 1], len_name + 1);
      }
    }
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv3_end_element);
  } else if (strcmp(name,"a:blip") == 0) {
    for (int i=0 ; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r:embed") == 0) {
        int len_embedId = strlen(attrs[i + 1]);
        twocellanchor->pic.blip_embed = malloc(len_embedId + 1);
        memcpy(twocellanchor->pic.blip_embed , attrs[i + 1], len_embedId + 1);
      }
    }
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv3_end_element);
  } else if (strcmp(name, "a:xfrm") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element, drawings_lv3_end_element);
  }
}

void drawings_lv3_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name,"xdr:cNvPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv3_end_element);
  } else if (strcmp(name,"a:blip") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv3_end_element);
  } else if (strcmp(name, "a:xfrm") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element, drawings_lv3_end_element);
  }
}

void drawings_col_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  struct Offset *offset_callbackdata = callbackdata;
  if (offset_callbackdata ->col == NULL) {
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
