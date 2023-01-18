#include "drawing.h"
#include "main.h"
#include "private.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct GraphicFrame new_graphicframe() {
  struct GraphicFrame graphic_frame;
  graphic_frame.name = NULL;
  graphic_frame.chart_id = NULL;
  graphic_frame.cx = 0;
  graphic_frame.cy = 0;
  return graphic_frame;
}

struct TwoCellAnchor new_twocellanchor() {
  struct TwoCellAnchor twocellanchor;
  twocellanchor.from.col = 0;
  twocellanchor.from.colOff = 0;
  twocellanchor.from.row = 0;
  twocellanchor.from.rowOff = 0;
  twocellanchor.to.col = 0;
  twocellanchor.to.colOff = 0;
  twocellanchor.to.row = 0;
  twocellanchor.to.rowOff = 0;
  twocellanchor.pic.name = NULL;
  twocellanchor.pic.hlinkClick_id = NULL;
  twocellanchor.pic.blip_embed = NULL;
  twocellanchor.pic.cx = 0;
  twocellanchor.pic.cy = 0;
  twocellanchor.graphic_frame = new_graphicframe();
  return twocellanchor;
}

struct Offset new_offset() {
  struct Offset offset;
  offset.col = 0;
  offset.colOff = 0;
  offset.row = 0;
  offset.rowOff = 0;
  return offset;
}

void drawings_callbackdata_initialize(struct DrawingCallbackData *data) {
  data->twocellanchor = new_twocellanchor();
  data->_tmp_offset = new_offset();
  data->text = NULL;
  data->textlen = 0;
  data->skiptag = NULL;
  data->skiptagcount = 0;
  data->skip_start = NULL;
  data->skip_end = NULL;
  data->skip_data = NULL;
  data->is_pic = '0';
  data->is_graphicframe = '0';
}

void drawings_skip_tag_start_element(void *callbackdata, const XML_Char *name,
                                     const XML_Char **attrs) {
  struct ArrayDrawingCallbackData *array_drawing_callbackdata =
      (struct ArrayDrawingCallbackData *)callbackdata;
  if (name &&
      XML_Char_icmp(
          name,
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->skiptag) == 0) {
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->skiptagcount++;
  }
}

void drawings_skip_tag_end_element(void *callbackdata, const XML_Char *name) {
  struct ArrayDrawingCallbackData *array_drawing_callbackdata =
      (struct ArrayDrawingCallbackData *)callbackdata;
  if (!name ||
      XML_Char_icmp(
          name,
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->skiptag) == 0) {
    if (--array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->skiptagcount == 0) {
      XML_SetElementHandler(
          xmlparser,
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->skip_start,
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->skip_end);
      XML_SetCharacterDataHandler(
          xmlparser,
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->skip_data);
      free(array_drawing_callbackdata
               ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
               ->skiptag);
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->skiptag = NULL;
    }
  }
}

void drawings_start_element(void *callbackdata, const XML_Char *name,
                            const XML_Char **attrs) {
  (void)attrs;
  struct ArrayDrawingCallbackData *array_drawing_callbackdata =
      (struct ArrayDrawingCallbackData *)callbackdata;
  if (XML_Char_icmp(name, "xdr:twoCellAnchor") == 0) {
    array_drawing_callbackdata->length++;
    array_drawing_callbackdata->drawing_callbackdata =
        (struct DrawingCallbackData **)XML_Char_realloc(
            array_drawing_callbackdata->drawing_callbackdata,
            array_drawing_callbackdata->length *
                sizeof(struct DrawingCallbackData *));
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1] =
        (struct DrawingCallbackData *)XML_Char_malloc(
            sizeof(struct DrawingCallbackData));
    if (array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1] ==
        NULL)
      debug_print("");
    drawings_callbackdata_initialize(
        array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]);
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, NULL);
  } else {
    XML_SetElementHandler(xmlparser, drawings_start_element, NULL);
  }
}

void drawings_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "xdr:twoCellAnchor") == 0) {
  }
  XML_SetElementHandler(xmlparser, drawings_start_element,
                        drawings_end_element);
}

void drawings_lv1_start_element(void *callbackdata, const XML_Char *name,
                                const XML_Char **attrs) {
  (void)attrs;
  struct ArrayDrawingCallbackData *array_drawing_callbackdata =
      (struct ArrayDrawingCallbackData *)callbackdata;
  if (XML_Char_icmp(name, "xdr:from") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if (XML_Char_icmp(name, "xdr:to") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if (XML_Char_icmp(name, "xdr:pic") == 0) {
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->is_pic = '1';
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if (XML_Char_icmp(name, "xdr:sp") == 0) {
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->skiptag = strdup(name);
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->skiptagcount = 1;
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->skip_start = NULL;
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->skip_end = drawings_lv1_end_element;
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->skip_data = NULL;
    XML_SetElementHandler(xmlparser, drawings_skip_tag_start_element,
                          drawings_skip_tag_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (XML_Char_icmp(name, "xdr:graphicFrame") == 0) {
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->is_graphicframe = '1';
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  }
}

void drawings_lv1_end_element(void *callbackdata, const XML_Char *name) {
  struct ArrayDrawingCallbackData *array_drawing_callbackdata =
      (struct ArrayDrawingCallbackData *)callbackdata;
  if (XML_Char_icmp(name, "xdr:from") == 0) {
    if (array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->_tmp_offset.col != 0) {
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->twocellanchor.from.col =
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->_tmp_offset.col;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->twocellanchor.from.colOff =
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->_tmp_offset.colOff;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->twocellanchor.from.row =
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->_tmp_offset.row;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->twocellanchor.from.rowOff =
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->_tmp_offset.rowOff;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->_tmp_offset = new_offset();
    }
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element,
                          drawings_lv1_end_element);
  } else if (XML_Char_icmp(name, "xdr:to") == 0) {
    if (array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->_tmp_offset.row != 0) {
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->twocellanchor.to.col =
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->_tmp_offset.col;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->twocellanchor.to.colOff =
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->_tmp_offset.colOff;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->twocellanchor.to.row =
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->_tmp_offset.row;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->twocellanchor.to.rowOff =
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->_tmp_offset.rowOff;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->_tmp_offset = new_offset();
      if (array_drawing_callbackdata->max_row_drawing <
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->twocellanchor.to.row) {
        array_drawing_callbackdata->max_row_drawing =
            array_drawing_callbackdata
                ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                ->twocellanchor.to.row;
      }
    }
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element,
                          drawings_lv1_end_element);
  } else if (XML_Char_icmp(name, "xdr:pic") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element,
                          drawings_lv1_end_element);
  } else if (XML_Char_icmp(name, "xdr:clientData") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element,
                          drawings_end_element);
  } else if (XML_Char_icmp(name, "xdr:graphicFrame") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element,
                          drawings_lv1_end_element);
  }
}

void drawings_lv2_start_element(void *callbackdata, const XML_Char *name,
                                const XML_Char **attrs) {
  (void)attrs;
  if (XML_Char_icmp(name, "xdr:col") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if (XML_Char_icmp(name, "xdr:colOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if (XML_Char_icmp(name, "xdr:row") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if (XML_Char_icmp(name, "xdr:rowOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if (XML_Char_icmp(name, "xdr:nvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (XML_Char_icmp(name, "xdr:blipFill") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (XML_Char_icmp(name, "xdr:spPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (XML_Char_icmp(name, "xdr:nvGraphicFramePr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (XML_Char_icmp(name, "xdr:xfrm") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (XML_Char_icmp(name, "a:graphic") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  }
}

void drawings_lv2_end_element(void *callbackdata, const XML_Char *name) {
  struct ArrayDrawingCallbackData *array_drawing_callbackdata =
      (struct ArrayDrawingCallbackData *)callbackdata;
  if (XML_Char_icmp(name, "xdr:col") == 0) {
    if (array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->text != NULL) {
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->text[array_drawing_callbackdata
                     ->drawing_callbackdata[array_drawing_callbackdata->length -
                                            1]
                     ->textlen] = '\0';
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->_tmp_offset.col =
          XML_Char_tol(
              array_drawing_callbackdata
                  ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                  ->text) +
          1;
      free(array_drawing_callbackdata
               ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
               ->text);
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->text = NULL;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->textlen = 0;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (XML_Char_icmp(name, "xdr:colOff") == 0) {
    if (array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->text != NULL) {
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->text[array_drawing_callbackdata
                     ->drawing_callbackdata[array_drawing_callbackdata->length -
                                            1]
                     ->textlen] = '\0';
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->_tmp_offset.colOff = XML_Char_tol(
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->text);
      free(array_drawing_callbackdata
               ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
               ->text);
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->text = NULL;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->textlen = 0;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (XML_Char_icmp(name, "xdr:row") == 0) {
    if (array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->text != NULL) {
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->text[array_drawing_callbackdata
                     ->drawing_callbackdata[array_drawing_callbackdata->length -
                                            1]
                     ->textlen] = '\0';
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->_tmp_offset.row =
          XML_Char_tol(
              array_drawing_callbackdata
                  ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                  ->text) +
          1;
      free(array_drawing_callbackdata
               ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
               ->text);
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->text = NULL;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->textlen = 0;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (XML_Char_icmp(name, "xdr:rowOff") == 0) {
    if (array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->text != NULL) {
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->text[array_drawing_callbackdata
                     ->drawing_callbackdata[array_drawing_callbackdata->length -
                                            1]
                     ->textlen] = '\0';
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->_tmp_offset.rowOff = XML_Char_tol(
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->text);
      free(array_drawing_callbackdata
               ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
               ->text);
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->text = NULL;
      array_drawing_callbackdata
          ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
          ->textlen = 0;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (XML_Char_icmp(name, "xdr:nvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
  } else if (XML_Char_icmp(name, "xdr:blipFill") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
  } else if (XML_Char_icmp(name, "xdr:spPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
  } else if (XML_Char_icmp(name, "xdr:nvGraphicFramePr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
  } else if (XML_Char_icmp(name, "xdr:xfrm") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
  } else if (XML_Char_icmp(name, "a:graphic") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element,
                          drawings_lv1_end_element);
  }
}

void drawings_lv3_start_element(void *callbackdata, const XML_Char *name,
                                const XML_Char **attrs) {
  (void)attrs;
  struct ArrayDrawingCallbackData *array_drawing_callbackdata =
      (struct ArrayDrawingCallbackData *)callbackdata;
  if (XML_Char_icmp(name, "xdr:cNvPr") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "name") == 0) {
        int len_name = XML_Char_len(attrs[i + 1]);
        if (array_drawing_callbackdata
                ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                ->is_pic == '1') {
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->twocellanchor.pic.name = (char *)XML_Char_malloc(len_name + 1);
          memcpy(
              array_drawing_callbackdata
                  ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                  ->twocellanchor.pic.name,
              attrs[i + 1], len_name + 1);
        } else if (array_drawing_callbackdata
                       ->drawing_callbackdata
                           [array_drawing_callbackdata->length - 1]
                       ->is_graphicframe == '1') {
          array_drawing_callbackdata
              ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
              ->twocellanchor.graphic_frame.name =
              (char *)XML_Char_malloc(len_name + 1);
          memcpy(
              array_drawing_callbackdata
                  ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                  ->twocellanchor.graphic_frame.name,
              attrs[i + 1], len_name + 1);
        }
      }
    }
    XML_SetElementHandler(xmlparser, NULL, drawings_lv3_end_element);
  } else if (XML_Char_icmp(name, "xdr:cNvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element,
                          drawings_lv3_end_element);
  } else if (XML_Char_icmp(name, "a:blip") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "r:embed") == 0) {
        int len_embedId = XML_Char_len(attrs[i + 1]);
        array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->twocellanchor.pic.blip_embed =
            (char *)XML_Char_malloc(len_embedId + 1);
        memcpy(
            array_drawing_callbackdata
                ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                ->twocellanchor.pic.blip_embed,
            attrs[i + 1], len_embedId + 1);
      }
    }
    XML_SetElementHandler(xmlparser, NULL, drawings_lv3_end_element);
  } else if (XML_Char_icmp(name, "a:stretch") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element,
                          drawings_lv3_end_element);
  } else if (XML_Char_icmp(name, "a:xfrm") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element,
                          drawings_lv3_end_element);
  } else if (XML_Char_icmp(name, "a:prstGeom") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element,
                          drawings_lv3_end_element);
  } else if (XML_Char_icmp(name, "a:ln") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element,
                          drawings_lv3_end_element);
  } else if (XML_Char_icmp(name, "a:graphicData") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element,
                          drawings_lv3_end_element);
  } else if (XML_Char_icmp(name, "a:ext") == 0) {
    if (array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->is_graphicframe == '1') {
      for (int i = 0; attrs[i]; i += 2) {
        if (XML_Char_icmp(attrs[i], "cx") == 0) {
          sscanf(
              attrs[i + 1], "%zu",
              &array_drawing_callbackdata
                   ->drawing_callbackdata[array_drawing_callbackdata->length -
                                          1]
                   ->twocellanchor.graphic_frame.cx);
        } else if (XML_Char_icmp(attrs[i], "cy") == 0) {
          sscanf(
              attrs[i + 1], "%zu",
              &array_drawing_callbackdata
                   ->drawing_callbackdata[array_drawing_callbackdata->length -
                                          1]
                   ->twocellanchor.graphic_frame.cy);
        }
      }
    }
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element,
                          drawings_lv3_end_element);
  }
}

void drawings_lv3_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "xdr:cNvPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element,
                          drawings_lv2_end_element);
  } else if (XML_Char_icmp(name, "xdr:cNvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element,
                          drawings_lv2_end_element);
  } else if (XML_Char_icmp(name, "a:blip") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element,
                          drawings_lv2_end_element);
  } else if (XML_Char_icmp(name, "a:stretch") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element,
                          drawings_lv2_end_element);
  } else if (XML_Char_icmp(name, "a:xfrm") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element,
                          drawings_lv2_end_element);
  } else if (XML_Char_icmp(name, "a:prstGeom") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element,
                          drawings_lv2_end_element);
  } else if (XML_Char_icmp(name, "a:ln") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element,
                          drawings_lv2_end_element);
  } else if (XML_Char_icmp(name, "a:ext") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element,
                          drawings_lv2_end_element);
  } else if (XML_Char_icmp(name, "a:graphicData") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element,
                          drawings_lv2_end_element);
  }
}

void drawings_lv4_start_element(void *callbackdata, const XML_Char *name,
                                const XML_Char **attrs) {
  (void)attrs;
  struct ArrayDrawingCallbackData *array_drawing_callbackdata =
      (struct ArrayDrawingCallbackData *)callbackdata;
  if (XML_Char_icmp(name, "a:hlinkClick") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "r:id") == 0) {
        int len_hlinkClickId = XML_Char_len(attrs[i + 1]);
        array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->twocellanchor.pic.hlinkClick_id =
            (char *)XML_Char_malloc(len_hlinkClickId + 1);
        memcpy(
            array_drawing_callbackdata
                ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                ->twocellanchor.pic.hlinkClick_id,
            attrs[i + 1], len_hlinkClickId + 1);
      }
    }
  } else if (XML_Char_icmp(name, "a:ext") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "cx") == 0) {
        sscanf(
            attrs[i + 1], "%zu",
            &array_drawing_callbackdata
                 ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                 ->twocellanchor.pic.cx);
      } else if (XML_Char_icmp(attrs[i], "cy") == 0) {
        sscanf(
            attrs[i + 1], "%zu",
            &array_drawing_callbackdata
                 ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                 ->twocellanchor.pic.cy);
      }
    }
  } else if (XML_Char_icmp(name, "c:chart") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "r:id") == 0) {
        array_drawing_callbackdata
            ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
            ->twocellanchor.graphic_frame.chart_id = strdup(attrs[i + 1]);
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, drawings_lv4_end_element);
}

void drawings_lv4_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, drawings_lv4_start_element,
                        drawings_lv3_end_element);
}

void drawings_content_handler(void *callbackdata, const XML_Char *buf,
                              int len) {
  struct ArrayDrawingCallbackData *array_drawing_callbackdata =
      (struct ArrayDrawingCallbackData *)callbackdata;
  if ((array_drawing_callbackdata
           ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
           ->text = (char *)XML_Char_realloc(
           array_drawing_callbackdata
               ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
               ->text,
           array_drawing_callbackdata
                   ->drawing_callbackdata[array_drawing_callbackdata->length -
                                          1]
                   ->textlen +
               len + 1)) == NULL) {
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->textlen = 0;
  } else {
    memcpy(
        array_drawing_callbackdata
                ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                ->text +
            array_drawing_callbackdata
                ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
                ->textlen,
        buf, len);
    array_drawing_callbackdata
        ->drawing_callbackdata[array_drawing_callbackdata->length - 1]
        ->textlen += len;
  }
}
