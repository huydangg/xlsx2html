#include <stdio.h>
#include <expat.h>
#include <string.h>
#include <stdlib.h>
#include <read_styles.h>

struct ArrayNumFMTs array_numfmts;
struct ArrayFonts array_fonts;
struct ArrayFills array_fills;
struct ArrayBorderCells array_borders;
struct ArrayXfs array_cellStyleXfs;
struct ArrayXfs array_cellXfs;


void styles_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name, "numFmts") == 0){
    array_numfmts.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	array_numfmts.numfmts = calloc((unsigned short int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct NumFMT));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_numfmts.numfmts);
    XML_SetElementHandler(xmlparser, numFmt_main_start_element, NULL);
  } else if(strcmp(name, "fonts") == 0){
    array_fonts.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	array_fonts.fonts = calloc((unsigned short int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct Font));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_fonts.fonts);
    XML_SetElementHandler(xmlparser, font_main_start_element, NULL);
  } else if (strcmp(name, "fills") == 0) {
    array_fills.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	array_fills.fills = calloc((unsigned short int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct Fill));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_fills.fills);
    XML_SetElementHandler(xmlparser, fill_main_start_element, NULL);
  } else if (strcmp(name, "borders") == 0) {
    array_borders.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	array_borders.borders = calloc((unsigned short int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct BorderCell));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_borders.borders);
    XML_SetElementHandler(xmlparser, border_main_start_element, NULL);
  } else if (strcmp(name, "cellStyleXfs") == 0) {
    array_cellStyleXfs.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	array_cellStyleXfs.Xfs = calloc((unsigned short int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct Xf));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_cellStyleXfs.Xfs);
    XML_SetElementHandler(xmlparser, xf_main_start_element, NULL);
  } else if (strcmp(name, "cellXfs") == 0) {
    array_cellXfs.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "count") == 0) {
	array_cellXfs.Xfs = calloc((unsigned short int)strtol((char *)attrs[i + 1], NULL, 10), sizeof(struct Xf));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_cellXfs.Xfs);
    XML_SetElementHandler(xmlparser, xf_main_start_element, NULL);
  }
}

void styles_end_element(void *userData, const XML_Char *name) {
  (void)name;
  if (strcmp(name, "fonts") == 0) {
  } else if (strcmp(name, "fills") == 0) {
  }

  XML_SetElementHandler(xmlparser, styles_start_element, NULL);
}

void numFmt_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct NumFMT *numFmts_callbackdata = userData;
  if (strcmp(name, "numFmt") == 0) {
    array_numfmts.length++;
    for (int i = 0; attrs[i]; i += 2){
      if (strcmp(attrs[i], "formatCode") == 0){
        numFmts_callbackdata[array_numfmts.length - 1].formatCode = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(numFmts_callbackdata[array_numfmts.length - 1].formatCode, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
      if (strcmp(attrs[i], "numFmtId") == 0){
        numFmts_callbackdata[array_numfmts.length - 1].numFmtId = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(numFmts_callbackdata[array_numfmts.length - 1].numFmtId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, numFmt_main_end_element);
}

void numFmt_main_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, numFmt_main_start_element, styles_end_element);
}


void font_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Font *fonts_callbackdata = userData;
  if (strcmp(name, "font") == 0) {
    array_fonts.length++; 
    fonts_callbackdata[array_fonts.length - 1].isBold = '0';
    fonts_callbackdata[array_fonts.length - 1].isItalic = '0';
    XML_SetElementHandler(xmlparser, font_item_start_element, NULL);
  }
}

void font_main_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "font") == 0) {
   XML_SetElementHandler(xmlparser, font_main_start_element, styles_end_element);
  }
}

void font_item_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Font *fonts_callbackdata = userData;
  if (strcmp(name, "sz") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if(strcmp(attrs[i], "val") == 0){
        fonts_callbackdata[array_fonts.length - 1].sz = (int)strtol((char *)attrs[i + 1], NULL, 10);
      }
    }
  } else if (strcmp(name, "name") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
        fonts_callbackdata[array_fonts.length - 1].name = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(fonts_callbackdata[array_fonts.length - 1].name, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1)); 
      }
    }
  } else if (strcmp(name, "b") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
        fonts_callbackdata[array_fonts.length - 1].isBold = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  } else if (strcmp(name, "i") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fonts_callbackdata[array_fonts.length - 1].isItalic = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  } else if (strcmp(name, "u") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "val") == 0) {
	fonts_callbackdata[array_fonts.length - 1].underline = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(fonts_callbackdata[array_fonts.length - 1].underline, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  } else if (strcmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "rgb") == 0) {
	fonts_callbackdata[array_fonts.length - 1].color.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(fonts_callbackdata[array_fonts.length - 1].color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  } else if(strcmp(name, "family") == 0){

  } else if(strcmp(name, "charset") == 0){

  }
  XML_SetElementHandler(xmlparser, NULL, font_item_end_element);

}

void font_item_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, font_item_start_element, font_main_end_element);
}

void fill_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  if (strcmp(name, "fill") == 0) {
    array_fills.length++;
    XML_SetElementHandler(xmlparser, fill_item_lv1_start_element, NULL);
  }
}

void fill_main_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "fill") == 0)  {
    XML_SetElementHandler(xmlparser, fill_main_start_element, styles_end_element);
  }
}

void fill_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Fill *fills_callbackdata = userData;
  if (strcmp(name, "patternFill") == 0)  {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "patternType") == 0) {
        fills_callbackdata[array_fills.length - 1].patternFill.patternType = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(fills_callbackdata[array_fills.length - 1].patternFill.patternType, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetElementHandler(xmlparser, fill_item_lv2_start_element, fill_item_lv1_end_element);
  } else if (strcmp(name, "gradientFill") == 0) {
    
  }
}

void fill_item_lv1_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "patternFill") == 0) {

  } else if (strcmp(name, "gradientFill") == 0) {
    
  }
  XML_SetElementHandler(xmlparser, fill_item_lv1_start_element, fill_main_end_element);
}

void fill_item_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Fill *fills_callbackdata = userData;
  if (strcmp(name, "bgColor") == 0)  {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "rgb") == 0) {
        fills_callbackdata[array_fills.length - 1].patternFill.bgColor.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(fills_callbackdata[array_fills.length - 1].patternFill.bgColor.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  } else if (strcmp(name, "fgColor") == 0) {
     for (int i = 0; attrs[i]; i += 2) {
       if (strcmp(attrs[i], "rgb") == 0) {
         fills_callbackdata[array_fills.length - 1].patternFill.fgColor.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
         memcpy(fills_callbackdata[array_fills.length - 1].patternFill.fgColor.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
       }
     }   
  }
  XML_SetElementHandler(xmlparser, NULL, fill_item_lv2_end_element);
}

void fill_item_lv2_end_element(void *userData, const XML_Char *name) {
  if (strcmp(name, "bgColor") == 0) {
    
  } else if (strcmp(name, "fgColor") == 0) {

  }
  XML_SetElementHandler(xmlparser, fill_item_lv2_start_element, fill_item_lv1_end_element);
}

void border_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  if (strcmp(name, "border") == 0) {
    array_borders.length++;
  }
  XML_SetElementHandler(xmlparser, border_item_lv1_start_element, border_item_lv1_end_element);
} 

void border_main_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, border_main_start_element, styles_end_element);
}

void border_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct BorderCell *borders_callbackdata = userData; 
  if (strcmp(name, "left") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[array_borders.length - 1].left.style = malloc(sizeof(XML_Char) * strlen(attrs[i + 1]) + 1);
	memcpy(borders_callbackdata[array_borders.length - 1].left.style, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[array_borders.length - 1].left);
  } else if (strcmp(name, "right") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[array_borders.length - 1].right.style = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(borders_callbackdata[array_borders.length - 1].right.style, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[array_borders.length - 1].right);
  } else if (strcmp(name, "top") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[array_borders.length - 1].top.style = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(borders_callbackdata[array_borders.length - 1].top.style, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[array_borders.length - 1].top);
  } else if (strcmp(name, "bottom") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "style") == 0) {
        borders_callbackdata[array_borders.length - 1].bottom.style = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(borders_callbackdata[array_borders.length - 1].bottom.style, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[array_borders.length - 1].bottom);
  } else if (strcmp(name, "diagonal") == 0) {
  }
  XML_SetElementHandler(xmlparser, border_item_lv2_start_element, border_item_lv1_end_element);
}

void border_item_lv1_end_element(void *userData, const XML_Char *name) {
  XML_SetUserData(xmlparser, array_borders.borders);
  XML_SetElementHandler(xmlparser, border_item_lv1_start_element, border_main_end_element);
}

void border_item_lv2_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Border *border_specific_callbackdata = userData;
  if(strcmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "rgb") == 0) {
        border_specific_callbackdata->color.rgb = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
	memcpy(border_specific_callbackdata->color.rgb, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, border_item_lv2_end_element);
}

void border_item_lv2_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, NULL, border_item_lv1_end_element);
}

void xf_main_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Xf *xfs_callbackdata = userData;
  if (strcmp(name, "xf") == 0) {
    int _tmp_count;
    if (xfs_callbackdata == array_cellStyleXfs.Xfs) {
       _tmp_count = ++array_cellStyleXfs.length;
    } else if (xfs_callbackdata == array_cellXfs.Xfs) {
      _tmp_count = ++array_cellXfs.length;
    }
    xfs_callbackdata[_tmp_count - 1].isApplyAlignment = '0';
    xfs_callbackdata[_tmp_count - 1].isApplyFont = '0';
    xfs_callbackdata[_tmp_count - 1].isApplyNumberFormat = '0';
    xfs_callbackdata[_tmp_count - 1].isApplyBorder = '0';
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "borderId") == 0) {
        xfs_callbackdata[_tmp_count - 1].borderId = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].borderId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "fillId") == 0) {
        xfs_callbackdata[_tmp_count - 1].fillId	= malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].fillId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "fontId") == 0) {
        xfs_callbackdata[_tmp_count - 1].fontId	= malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].fontId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "numFmtId") == 0) {
        xfs_callbackdata[_tmp_count - 1].numFmtId = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].numFmtId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "xfId") == 0) {
        xfs_callbackdata[_tmp_count - 1].xfId = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].xfId, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "applyAlignment") == 0) {
        xfs_callbackdata[_tmp_count - 1].isApplyAlignment = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      } else if (strcmp(attrs[i], "applyNumberFormat") == 0) {
        xfs_callbackdata[_tmp_count - 1].isApplyNumberFormat = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      } else if (strcmp(attrs[i], "applyBorder") == 0) {
        xfs_callbackdata[_tmp_count - 1].isApplyBorder = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      } else if (strcmp(attrs[i], "applyFont") == 0) {
        xfs_callbackdata[_tmp_count - 1].isApplyFont = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  }
  XML_SetElementHandler(xmlparser, xf_item_lv1_start_element, xf_main_end_element);
}

void xf_main_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, xf_main_start_element, styles_end_element);
}

void xf_item_lv1_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  struct Xf *xfs_callbackdata = userData;
  if (strcmp(name, "alignment") == 0) {
    int _tmp_count;
    if (xfs_callbackdata == array_cellStyleXfs.Xfs) {
       _tmp_count = array_cellStyleXfs.length;
    } else if (xfs_callbackdata == array_cellXfs.Xfs) {
      _tmp_count = array_cellXfs.length;
    }
    xfs_callbackdata[_tmp_count - 1].alignment.isWrapText = '0';
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "horizontal") == 0) {
	xfs_callbackdata[_tmp_count - 1].alignment.horizontal = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].alignment.horizontal, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "vertical") == 0) {
	xfs_callbackdata[_tmp_count - 1].alignment.vertical = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].alignment.vertical, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "textRotation") == 0) {
	xfs_callbackdata[_tmp_count - 1].alignment.textRotation = malloc(sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].alignment.textRotation, attrs[i + 1], sizeof(XML_Char) * (strlen(attrs[i + 1]) + 1));
      } else if (strcmp(attrs[i], "wrapText") == 0) {
        xfs_callbackdata[_tmp_count - 1].alignment.isWrapText = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }

  } else if (strcmp(name, "protection") == 0) {

  }
  XML_SetElementHandler(xmlparser, NULL, xf_item_lv1_end_element);
}

void xf_item_lv1_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, xf_item_lv1_start_element, xf_main_end_element);
}

