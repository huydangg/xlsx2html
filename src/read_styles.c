#include <private.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <read_styles.h>
#include <errno.h>

struct ArrayNumFMTs array_numfmts;
struct ArrayFonts array_fonts;
struct ArrayFills array_fills;
struct ArrayBorderCells array_borders;
struct ArrayXfs array_cellStyleXfs;
struct ArrayXfs array_cellXfs;


void styles_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (XML_Char_icmp(name, "numFmts") == 0){
    array_numfmts.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "count") == 0) {
	array_numfmts.numfmts = calloc((unsigned short)XML_Char_tol((char *)attrs[i + 1]), sizeof(struct NumFMT));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_numfmts.numfmts);
    XML_SetElementHandler(xmlparser, numFmt_main_start_element, NULL);
  } else if(XML_Char_icmp(name, "fonts") == 0){
    array_fonts.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "count") == 0) {
	array_fonts.fonts = calloc((unsigned short)XML_Char_tol((char *)attrs[i + 1]), sizeof(struct Font));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_fonts.fonts);
    XML_SetElementHandler(xmlparser, font_main_start_element, NULL);
  } else if (XML_Char_icmp(name, "fills") == 0) {
    array_fills.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "count") == 0) {
	array_fills.fills = calloc((unsigned short)XML_Char_tol((char *)attrs[i + 1]), sizeof(struct Fill));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_fills.fills);
    XML_SetElementHandler(xmlparser, fill_main_start_element, NULL);
  } else if (XML_Char_icmp(name, "borders") == 0) {
    array_borders.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "count") == 0) {
	array_borders.borders = calloc((unsigned short)XML_Char_tol((char *)attrs[i + 1]), sizeof(struct BorderCell));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_borders.borders);
    XML_SetElementHandler(xmlparser, border_main_start_element, NULL);
  } else if (XML_Char_icmp(name, "cellStyleXfs") == 0) {
    array_cellStyleXfs.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "count") == 0) {
	array_cellStyleXfs.Xfs = calloc((unsigned short)XML_Char_tol((char *)attrs[i + 1]), sizeof(struct Xf));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_cellStyleXfs.Xfs);
    XML_SetElementHandler(xmlparser, xf_main_start_element, NULL);
  } else if (XML_Char_icmp(name, "cellXfs") == 0) {
    array_cellXfs.length = 0;
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "count") == 0) {
	array_cellXfs.Xfs = calloc((unsigned short)XML_Char_tol((char *)attrs[i + 1]), sizeof(struct Xf));
	break;
      }
    }
    XML_SetUserData(xmlparser, array_cellXfs.Xfs);
    XML_SetElementHandler(xmlparser, xf_main_start_element, NULL);
  }
}

void styles_end_element(void *callbackdata, const XML_Char *name) {
  (void)name;
  if (XML_Char_icmp(name, "fonts") == 0) {
  } else if (XML_Char_icmp(name, "fills") == 0) {
  }

  XML_SetElementHandler(xmlparser, styles_start_element, NULL);
}

void numFmt_main_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct NumFMT *numFmts_callbackdata = callbackdata;
  if (XML_Char_icmp(name, "numFmt") == 0) {
    array_numfmts.length++;
    for (int i = 0; attrs[i]; i += 2){
      if (XML_Char_icmp(attrs[i], "formatCode") == 0){
        numFmts_callbackdata[array_numfmts.length - 1].formatCode = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
        memcpy(numFmts_callbackdata[array_numfmts.length - 1].formatCode, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      } else if (XML_Char_icmp(attrs[i], "numFmtId") == 0){
        numFmts_callbackdata[array_numfmts.length - 1].numFmtId = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
        memcpy(numFmts_callbackdata[array_numfmts.length - 1].numFmtId, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, numFmt_main_end_element);
}

void numFmt_main_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, numFmt_main_start_element, styles_end_element);
}


void font_main_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct Font *fonts_callbackdata = callbackdata;
  if (XML_Char_icmp(name, "font") == 0) {
    array_fonts.length++; 
    fonts_callbackdata[array_fonts.length - 1].isBold = '0';
    fonts_callbackdata[array_fonts.length - 1].isItalic = '0';
    XML_SetElementHandler(xmlparser, font_item_start_element, NULL);
  }
}

void font_main_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "font") == 0) {
   XML_SetElementHandler(xmlparser, font_main_start_element, styles_end_element);
  }
}

void font_item_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct Font *fonts_callbackdata = callbackdata;
  if (XML_Char_icmp(name, "sz") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if(XML_Char_icmp(attrs[i], "val") == 0){
        fonts_callbackdata[array_fonts.length - 1].sz = strtof((char *)attrs[i + 1], NULL);
      }
    }
  } else if (XML_Char_icmp(name, "name") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
        fonts_callbackdata[array_fonts.length - 1].name = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	memcpy(fonts_callbackdata[array_fonts.length - 1].name, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1)); 
      }
    }
  } else if (XML_Char_icmp(name, "b") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
        fonts_callbackdata[array_fonts.length - 1].isBold = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  } else if (XML_Char_icmp(name, "i") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
	fonts_callbackdata[array_fonts.length - 1].isItalic = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  } else if (XML_Char_icmp(name, "u") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "val") == 0) {
	fonts_callbackdata[array_fonts.length - 1].underline = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	memcpy(fonts_callbackdata[array_fonts.length - 1].underline, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
  } else if (XML_Char_icmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "rgb") == 0) {
	fonts_callbackdata[array_fonts.length - 1].color.rgb = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) - 1 + 1));
	snprintf(fonts_callbackdata[array_fonts.length - 1].color.rgb, (XML_Char_len(attrs[i + 1]) - 1 + 1), "#%s", attrs[i + 1] + 2);
      }
    }
  } else if(XML_Char_icmp(name, "family") == 0){

  } else if(XML_Char_icmp(name, "charset") == 0){

  }
  XML_SetElementHandler(xmlparser, NULL, font_item_end_element);

}

void font_item_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, font_item_start_element, font_main_end_element);
}

void fill_main_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  if (XML_Char_icmp(name, "fill") == 0) {
    array_fills.length++;
    XML_SetElementHandler(xmlparser, fill_item_lv1_start_element, NULL);
  }
}

void fill_main_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "fill") == 0)  {
    XML_SetElementHandler(xmlparser, fill_main_start_element, styles_end_element);
  }
}

void fill_item_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct Fill *fills_callbackdata = callbackdata;
  if (XML_Char_icmp(name, "patternFill") == 0)  {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "patternType") == 0) {
        fills_callbackdata[array_fills.length - 1].patternFill.patternType = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
        memcpy(fills_callbackdata[array_fills.length - 1].patternFill.patternType, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
    XML_SetElementHandler(xmlparser, fill_item_lv2_start_element, fill_item_lv1_end_element);
  } else if (XML_Char_icmp(name, "gradientFill") == 0) {
    
  }
}

void fill_item_lv1_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "patternFill") == 0) {

  } else if (XML_Char_icmp(name, "gradientFill") == 0) {
    
  }
  XML_SetElementHandler(xmlparser, fill_item_lv1_start_element, fill_main_end_element);
}

void fill_item_lv2_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct Fill *fills_callbackdata = callbackdata;
  if (XML_Char_icmp(name, "bgColor") == 0)  {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "rgb") == 0) {
        fills_callbackdata[array_fills.length - 1].patternFill.bgColor.rgb = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) - 1 + 1));
	snprintf(fills_callbackdata[array_fills.length - 1].patternFill.bgColor.rgb, XML_Char_len(attrs[i + 1]) - 1 + 1, "#%s", attrs[i + 1] + 2);
      }
    }
  } else if (XML_Char_icmp(name, "fgColor") == 0) {
     for (int i = 0; attrs[i]; i += 2) {
       if (XML_Char_icmp(attrs[i], "rgb") == 0) {
         fills_callbackdata[array_fills.length - 1].patternFill.fgColor.rgb = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	 snprintf(fills_callbackdata[array_fills.length - 1].patternFill.fgColor.rgb, XML_Char_len(attrs[i + 1]) - 1 + 1, "#%s", attrs[i + 1] + 2);
       }
     }   
  }
  XML_SetElementHandler(xmlparser, NULL, fill_item_lv2_end_element);
}

void fill_item_lv2_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "bgColor") == 0) {
    
  } else if (XML_Char_icmp(name, "fgColor") == 0) {

  }
  XML_SetElementHandler(xmlparser, fill_item_lv2_start_element, fill_item_lv1_end_element);
}

void border_main_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  if (XML_Char_icmp(name, "border") == 0) {
    array_borders.length++;
  }
  XML_SetElementHandler(xmlparser, border_item_lv1_start_element, border_item_lv1_end_element);
} 

void border_main_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, border_main_start_element, styles_end_element);
}

void border_item_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct BorderCell *borders_callbackdata = callbackdata; 
  if (XML_Char_icmp(name, "left") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "style") == 0) {
        borders_callbackdata[array_borders.length - 1].left.style = XML_Char_malloc(sizeof(XML_Char) * XML_Char_len(attrs[i + 1]) + 1);
	memcpy(borders_callbackdata[array_borders.length - 1].left.style, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[array_borders.length - 1].left);
  } else if (XML_Char_icmp(name, "right") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "style") == 0) {
        borders_callbackdata[array_borders.length - 1].right.style = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	memcpy(borders_callbackdata[array_borders.length - 1].right.style, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[array_borders.length - 1].right);
  } else if (XML_Char_icmp(name, "top") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "style") == 0) {
        borders_callbackdata[array_borders.length - 1].top.style = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	memcpy(borders_callbackdata[array_borders.length - 1].top.style, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[array_borders.length - 1].top);
  } else if (XML_Char_icmp(name, "bottom") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "style") == 0) {
        borders_callbackdata[array_borders.length - 1].bottom.style = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	memcpy(borders_callbackdata[array_borders.length - 1].bottom.style, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
    XML_SetUserData(xmlparser, &borders_callbackdata[array_borders.length - 1].bottom);
  } else if (XML_Char_icmp(name, "diagonal") == 0) {
  }
  XML_SetElementHandler(xmlparser, border_item_lv2_start_element, border_item_lv1_end_element);
}

void border_item_lv1_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetUserData(xmlparser, array_borders.borders);
  XML_SetElementHandler(xmlparser, border_item_lv1_start_element, border_main_end_element);
}

void border_item_lv2_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct Border *border_specific_callbackdata = callbackdata;
  if(XML_Char_icmp(name, "color") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "rgb") == 0) {
        border_specific_callbackdata->color.rgb = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	memcpy(border_specific_callbackdata->color.rgb, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, border_item_lv2_end_element);
}

void border_item_lv2_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, NULL, border_item_lv1_end_element);
}

void xf_main_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct Xf *xfs_callbackdata = callbackdata;
  if (XML_Char_icmp(name, "xf") == 0) {
    int _tmp_count;
    if (xfs_callbackdata == array_cellStyleXfs.Xfs) {
       _tmp_count = ++array_cellStyleXfs.length;
    } else if (xfs_callbackdata == array_cellXfs.Xfs) {
      _tmp_count = ++array_cellXfs.length;
    }
    xfs_callbackdata[_tmp_count - 1].isApplyAlignment = '1';
    xfs_callbackdata[_tmp_count - 1].isApplyFont = '1';
    xfs_callbackdata[_tmp_count - 1].isApplyNumberFormat = '1';
    xfs_callbackdata[_tmp_count - 1].isApplyBorder = '1';
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "borderId") == 0) {
        xfs_callbackdata[_tmp_count - 1].borderId = (unsigned short)XML_Char_tol(attrs[i + 1]);
      } else if (XML_Char_icmp(attrs[i], "fillId") == 0) {
        xfs_callbackdata[_tmp_count - 1].fillId	= (unsigned short)XML_Char_tol(attrs[i + 1]);
      } else if (XML_Char_icmp(attrs[i], "fontId") == 0) {
        xfs_callbackdata[_tmp_count - 1].fontId	= (unsigned short)XML_Char_tol(attrs[i + 1]);
      } else if (XML_Char_icmp(attrs[i], "numFmtId") == 0) {
        xfs_callbackdata[_tmp_count - 1].numFmtId = (unsigned short)XML_Char_tol(attrs[i + 1]);
      } else if (XML_Char_icmp(attrs[i], "xfId") == 0) {
	xfs_callbackdata[_tmp_count - 1].xfId = (unsigned short)XML_Char_tol(attrs[i + 1]);
      } else if (XML_Char_icmp(attrs[i], "applyAlignment") == 0) {
        xfs_callbackdata[_tmp_count - 1].isApplyAlignment = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      } else if (XML_Char_icmp(attrs[i], "applyNumberFormat") == 0) {
        xfs_callbackdata[_tmp_count - 1].isApplyNumberFormat = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      } else if (XML_Char_icmp(attrs[i], "applyBorder") == 0) {
        xfs_callbackdata[_tmp_count - 1].isApplyBorder = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      } else if (XML_Char_icmp(attrs[i], "applyFont") == 0) {
        xfs_callbackdata[_tmp_count - 1].isApplyFont = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }
  }
  XML_SetElementHandler(xmlparser, xf_item_lv1_start_element, xf_main_end_element);
}

void xf_main_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, xf_main_start_element, styles_end_element);
}

void xf_item_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct Xf *xfs_callbackdata = callbackdata;
  if (XML_Char_icmp(name, "alignment") == 0) {
    int _tmp_count;
    if (xfs_callbackdata == array_cellStyleXfs.Xfs) {
       _tmp_count = array_cellStyleXfs.length;
    } else if (xfs_callbackdata == array_cellXfs.Xfs) {
      _tmp_count = array_cellXfs.length;
    }
    xfs_callbackdata[_tmp_count - 1].alignment.isWrapText = '0';
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "horizontal") == 0) {
	xfs_callbackdata[_tmp_count - 1].alignment.horizontal = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].alignment.horizontal, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      } else if (XML_Char_icmp(attrs[i], "vertical") == 0) {
	xfs_callbackdata[_tmp_count - 1].alignment.vertical = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].alignment.vertical, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      } else if (XML_Char_icmp(attrs[i], "textRotation") == 0) {
	xfs_callbackdata[_tmp_count - 1].alignment.textRotation = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
        memcpy(xfs_callbackdata[_tmp_count - 1].alignment.textRotation, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      } else if (XML_Char_icmp(attrs[i], "wrapText") == 0) {
        xfs_callbackdata[_tmp_count - 1].alignment.isWrapText = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0';
      }
    }

  } else if (XML_Char_icmp(name, "protection") == 0) {

  }
  XML_SetElementHandler(xmlparser, NULL, xf_item_lv1_end_element);
}

void xf_item_lv1_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, xf_item_lv1_start_element, xf_main_end_element);
}

