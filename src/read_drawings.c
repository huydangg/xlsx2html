#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <read_drawings.h>
#include <private.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct TwoCellAnchor new_twocellanchor() {
  struct TwoCellAnchor twocellanchor;
  twocellanchor.editAs = NULL;
  twocellanchor.from.col = NULL;
  twocellanchor.from.colOff = NULL;
  twocellanchor.from.row = NULL;
  twocellanchor.from.rowOff = NULL;
  twocellanchor.to.col = NULL;
  twocellanchor.to.colOff = NULL;
  twocellanchor.to.row = NULL;
  twocellanchor.to.rowOff = NULL;
  twocellanchor.pic.name = NULL;
  twocellanchor.pic.hlinkClick_id = NULL;
  twocellanchor.pic.blip_embed = NULL;
  twocellanchor.pic.cx = 0;
  twocellanchor.pic.cy = 0;
  return twocellanchor;
}

struct Offset new_offset() {
  struct Offset offset;
  offset.col = NULL;
  offset.colOff = NULL;
  offset.row = NULL;
  offset.rowOff = NULL;
  return offset;
}

void drawings_callbackdata_initialize (struct DrawingCallbackData *data, struct ArrayRelationships *array_drawing_rels, FILE *findexhtml, zip_t *zip) {
  data->array_drawing_rels = array_drawing_rels;
  data->twocellanchor = new_twocellanchor();
  data->_tmp_offset = new_offset();
  data->findexhtml = findexhtml;
  data->zip = zip;
  data->text = NULL;
  data->textlen = 0;
  data->skiptag = NULL;
  data->skiptagcount = 0;
  data->skip_start = NULL;
  data->skip_end = NULL;
  data->skip_data = NULL;
}

void drawings_skip_tag_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (name && strcmp(name, drawing_callbackdata->skiptag) == 0) {
    drawing_callbackdata->skiptagcount++;
  }
}

void drawings_skip_tag_end_element(void *callbackdata, const XML_Char *name) {
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (!name || strcmp(name, drawing_callbackdata->skiptag) == 0) {
    if (--drawing_callbackdata->skiptagcount == 0) {
      XML_SetElementHandler(xmlparser, drawing_callbackdata->skip_start, drawing_callbackdata->skip_end);
      XML_SetCharacterDataHandler(xmlparser, drawing_callbackdata->skip_data);
      free(drawing_callbackdata->skiptag);
      drawing_callbackdata->skiptag = NULL;
    }
  }
}

void drawings_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  printf("LV000000000000000000: <%s>\n", name);
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (strcmp(name,"xdr:twoCellAnchor") == 0) {
    for (int i = 0; attrs[i]; i+=2) {
      int len_editAs = strlen(attrs[i + 1]);
      drawing_callbackdata->twocellanchor.editAs = malloc(len_editAs + 1);
      memcpy(drawing_callbackdata->twocellanchor.editAs, attrs[i + 1], len_editAs + 1);
    }
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, NULL);
  } else {
    XML_SetElementHandler(xmlparser, drawings_start_element, NULL);
  }
}

void drawings_end_element(void *callbackdata, const XML_Char *name) {
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  printf("LV000000000000000000: </%s>\n", name);
  if (strcmp(name,"xdr:twoCellAnchor") == 0) {
    //TODO: Record image html syntax with data from rels drawings to findexhtml
    //At last, free twocellanchor obj


    if (drawing_callbackdata->twocellanchor.pic.name != NULL) {
      int len_row = strlen(drawing_callbackdata->twocellanchor.from.row);
      int _tmp_col = strtol(drawing_callbackdata->twocellanchor.from.col, NULL, 10);
      char *col_name = int_to_column_name(_tmp_col);
      int len_col = strlen(col_name);
      //38: "<div id=\"img_%d_%d\" style="display:none;">"
      int len_div_img = len_row + len_col + 38;
      char *DIV_IMG = malloc(len_div_img + 1);
      snprintf(
        DIV_IMG, len_div_img + 1, "<div id=\"img_%s_%s\" style=\"display:none;\">",
	drawing_callbackdata->twocellanchor.from.row,
	col_name
      );
      fputs(DIV_IMG, drawing_callbackdata->findexhtml);
      free(DIV_IMG);
      free(col_name);
      for (int i = 0; i < drawing_callbackdata->array_drawing_rels->length; i++) {
	if (strcmp(drawing_callbackdata->twocellanchor.pic.blip_embed, drawing_callbackdata->array_drawing_rels->relationships[i]->id) == 0) {
	  struct zip_stat sb;
          struct zip_file *img_zf;
	  int img_fd;
	  int len_target = strlen(drawing_callbackdata->array_drawing_rels->relationships[i]->target);
	  char *_tmp_target = malloc(len_target + 1);
	  snprintf(_tmp_target, len_target + 1, "xl%s", drawing_callbackdata->array_drawing_rels->relationships[i]->target + 2);
	  if (zip_stat(drawing_callbackdata->zip, _tmp_target, 0, &sb) == 0) {
	    img_zf = zip_fopen(drawing_callbackdata->zip, sb.name, 0);
	    if (!img_zf) {
	      fprintf(stderr, "Error during unzip img file");
	      continue;
	    }
	    char *img_zf_name = strdup(sb.name);
	    char *token = strtok(img_zf_name , "/");
	    int count = 0;
	    count++;
	    while (count <= 2) {
	      token = strtok(NULL, "/");
	      count++;
	    }
	    char *img_name = strdup(token);
	    img_fd = open(img_name, O_WRONLY | O_CREAT, 0644);
	    if (img_fd < 0) {
	      fprintf(stderr, "Error during create a new image");
	      continue;
	    }
	    char buf[100];
	    int len = 0;
	    long long sum = 0;
	    while (sum != sb.size) {
              len = zip_fread(img_zf, buf, 100);
	      if (len < 0) {
		fprintf(stderr, "Error during extract image");
		break;
	      }
	      write(img_fd, buf, len);
	      sum += len;
	    }
	    close(img_fd);
	    zip_fclose(img_zf);
	    free(_tmp_target);
	    char *IMG_URL = NULL;
            int len_resource_url, len_img_name, len_img_url;
	    if (strstr(RESOURCE_URL, "https") != NULL) {
	      token = strtok(img_name, ".");
	      char *img_ext = strdup(token);
	      while (token != NULL) {
	        free(img_ext);
	        img_ext = NULL;
	        img_ext = strdup(token);
	        token = strtok(NULL, ".");
	      }
	      free(token);

	      len_resource_url = strlen(RESOURCE_URL);
	      len_img_name = strlen(img_name);
	      int len_img_ext = strlen(img_ext);
	      // 17: /img/%s?format_img=
	      len_img_url = len_img_name + len_resource_url + len_img_ext + 17;
	      IMG_URL = malloc(len_img_url + 1);
	      snprintf(IMG_URL, len_img_url + 1, "%s/img/%s?format_img=%s", RESOURCE_URL, img_name, img_ext);
	    } else {
	      len_resource_url = strlen(RESOURCE_URL);
	      len_img_name = strlen(img_name);
	      len_img_url = len_img_name + len_resource_url + 1;
	      IMG_URL = malloc(len_img_url + 1);
	      snprintf(IMG_URL, len_img_url + 1, "%s/%s", RESOURCE_URL, img_name);
	    }
	    printf("IMG_URLLLLLLLLL: %s\n", IMG_URL);
	    free(IMG_URL);
	  }
	  break;
	}
      }



      fputs("</div>", drawing_callbackdata->findexhtml);
      free(drawing_callbackdata->twocellanchor.pic.name);
      free(drawing_callbackdata->twocellanchor.pic.blip_embed);
      free(drawing_callbackdata->twocellanchor.pic.hlinkClick_id);
    }
    if (drawing_callbackdata->twocellanchor.editAs != NULL) {
      free(drawing_callbackdata->twocellanchor.editAs);
      free(drawing_callbackdata->twocellanchor.from.col);
      free(drawing_callbackdata->twocellanchor.from.colOff);
      free(drawing_callbackdata->twocellanchor.from.row);
      free(drawing_callbackdata->twocellanchor.from.rowOff);
      free(drawing_callbackdata->twocellanchor.to.col);
      free(drawing_callbackdata->twocellanchor.to.colOff);
      free(drawing_callbackdata->twocellanchor.to.row);
      free(drawing_callbackdata->twocellanchor.to.rowOff);
    }
    drawing_callbackdata->twocellanchor = new_twocellanchor();
  }
  XML_SetElementHandler(xmlparser, drawings_start_element, drawings_end_element);
}

void drawings_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  printf("LV111111111: <%s>\n", name);
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (strcmp(name, "xdr:from") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if (strcmp(name, "xdr:to") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if (strcmp(name, "xdr:pic") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if (strcmp(name, "xdr:sp") == 0) {
    free(drawing_callbackdata->twocellanchor.editAs);
    free(drawing_callbackdata->twocellanchor.from.col);
    free(drawing_callbackdata->twocellanchor.from.colOff);
    free(drawing_callbackdata->twocellanchor.from.row);
    free(drawing_callbackdata->twocellanchor.from.rowOff);
    free(drawing_callbackdata->twocellanchor.to.col);
    free(drawing_callbackdata->twocellanchor.to.colOff);
    free(drawing_callbackdata->twocellanchor.to.row);
    free(drawing_callbackdata->twocellanchor.to.rowOff);
    drawing_callbackdata->twocellanchor = new_twocellanchor();
    drawing_callbackdata->skiptag = strdup(name);
    drawing_callbackdata->skiptagcount = 1;
    drawing_callbackdata->skip_start = NULL;
    drawing_callbackdata->skip_end = drawings_lv1_end_element;
    drawing_callbackdata->skip_data = NULL;
    XML_SetElementHandler(xmlparser, drawings_skip_tag_start_element, drawings_skip_tag_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void drawings_lv1_end_element(void *callbackdata, const XML_Char *name) {
  printf("LV111111111: </%s>\n", name);
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (strcmp(name, "xdr:from") == 0) {
    if (drawing_callbackdata->_tmp_offset.col != NULL) {
      drawing_callbackdata->twocellanchor.from.col = strdup(drawing_callbackdata->_tmp_offset.col);
      drawing_callbackdata->twocellanchor.from.colOff = strdup(drawing_callbackdata->_tmp_offset.colOff);
      drawing_callbackdata->twocellanchor.from.row = strdup(drawing_callbackdata->_tmp_offset.row);
      drawing_callbackdata->twocellanchor.from.rowOff = strdup(drawing_callbackdata->_tmp_offset.rowOff);
      free(drawing_callbackdata->_tmp_offset.col);
      free(drawing_callbackdata->_tmp_offset.colOff);
      free(drawing_callbackdata->_tmp_offset.row);
      free(drawing_callbackdata->_tmp_offset.rowOff);
      drawing_callbackdata->_tmp_offset = new_offset();
    }
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, drawings_lv1_end_element);
  } else if (strcmp(name, "xdr:to") == 0) {
    if (drawing_callbackdata->_tmp_offset.col != NULL) {
      drawing_callbackdata->twocellanchor.to.col = strdup(drawing_callbackdata->_tmp_offset.col);
      drawing_callbackdata->twocellanchor.to.colOff = strdup(drawing_callbackdata->_tmp_offset.colOff);
      drawing_callbackdata->twocellanchor.to.row = strdup(drawing_callbackdata->_tmp_offset.row);
      drawing_callbackdata->twocellanchor.to.rowOff = strdup(drawing_callbackdata->_tmp_offset.rowOff);
      free(drawing_callbackdata->_tmp_offset.col);
      free(drawing_callbackdata->_tmp_offset.colOff);
      free(drawing_callbackdata->_tmp_offset.row);
      free(drawing_callbackdata->_tmp_offset.rowOff);
      drawing_callbackdata->_tmp_offset = new_offset();
    }
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, drawings_lv1_end_element);
  } else if (strcmp(name, "xdr:pic") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, drawings_lv1_end_element);
  } else if (strcmp(name, "xdr:clientData") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, drawings_end_element);
  }
}

void drawings_lv2_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  printf("LV222222222: <%s>\n", name);
  if (strcmp(name, "xdr:col") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if(strcmp(name, "xdr:colOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if(strcmp(name, "xdr:row") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if(strcmp(name, "xdr:rowOff") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv2_end_element);
    XML_SetCharacterDataHandler(xmlparser, drawings_content_handler);
  } else if (strcmp(name, "xdr:nvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (strcmp(name, "xdr:blipFill") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (strcmp(name, "xdr:spPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  }
}

void drawings_lv2_end_element(void *callbackdata, const XML_Char *name) {
  printf("LV222222222: </%s>\n", name);
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (strcmp(name, "xdr:col") == 0) {
    if (drawing_callbackdata->text != NULL) {
      drawing_callbackdata->_tmp_offset.col = strdup(drawing_callbackdata->text);
      free(drawing_callbackdata->text);
      drawing_callbackdata->text = NULL;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:colOff") == 0) {
    if (drawing_callbackdata->text != NULL) {
      drawing_callbackdata->_tmp_offset.colOff = strdup(drawing_callbackdata->text);
      free(drawing_callbackdata->text);
      drawing_callbackdata->text = NULL;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:row") == 0) {
    if (drawing_callbackdata->text != NULL) {
      drawing_callbackdata->_tmp_offset.row = strdup(drawing_callbackdata->text);
      free(drawing_callbackdata->text);
      drawing_callbackdata->text = NULL;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:rowOff") == 0) {
    if (drawing_callbackdata->text != NULL) {
      drawing_callbackdata->_tmp_offset.rowOff = strdup(drawing_callbackdata->text);
      free(drawing_callbackdata->text);
      drawing_callbackdata->text = NULL;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (strcmp(name, "xdr:nvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
  } else if (strcmp(name, "xdr:blipFill") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
  } else if (strcmp(name, "xdr:spPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
  }
}

void drawings_lv3_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  printf("LV333333333333333333333: <%s>\n", name);
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (strcmp(name,"xdr:cNvPr") == 0) {
    for (int i=0 ; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "name") == 0) {
        int len_name = strlen(attrs[i + 1]);
        drawing_callbackdata->twocellanchor.pic.name = malloc(len_name + 1);
        memcpy(drawing_callbackdata->twocellanchor.pic.name, attrs[i + 1], len_name + 1);
      }
    }
    XML_SetElementHandler(xmlparser, NULL, drawings_lv3_end_element);
  } else if (strcmp(name, "xdr:cNvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element, drawings_lv3_end_element);
  } else if (strcmp(name,"a:blip") == 0) {
    for (int i=0 ; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r:embed") == 0) {
        int len_embedId = strlen(attrs[i + 1]);
        drawing_callbackdata->twocellanchor.pic.blip_embed = malloc(len_embedId + 1);
        memcpy(drawing_callbackdata->twocellanchor.pic.blip_embed , attrs[i + 1], len_embedId + 1);
      }
    }
    XML_SetElementHandler(xmlparser, NULL, drawings_lv3_end_element);
  } else if (strcmp(name, "a:stretch") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element, drawings_lv3_end_element);
  } else if (strcmp(name, "a:xfrm") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element, drawings_lv3_end_element);
  } else if (strcmp(name, "a:prstGeom") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element, drawings_lv3_end_element);
  } else if (strcmp(name, "a:ln") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv4_start_element, drawings_lv3_end_element);
  }
}

void drawings_lv3_end_element(void *callbackdata, const XML_Char *name) {
  printf("LV333333333333333333333: </%s>\n", name);
  if (strcmp(name,"xdr:cNvPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  } else if(strcmp(name, "xdr:cNvPicPr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  } else if (strcmp(name,"a:blip") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  } else if (strcmp(name, "a:stretch") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  } else if (strcmp(name, "a:xfrm") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  } else if (strcmp(name, "a:prstGeom") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  } else if (strcmp(name, "a:ln") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, drawings_lv2_end_element);
  }
}

void drawings_lv4_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  printf("LV44444444444444444: <%s>\n", name);
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (strcmp(name, "a:hlinkClick") == 0) {
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r:id") == 0) {
	int len_hlinkClickId = strlen(attrs[i + 1]);
	drawing_callbackdata->twocellanchor.pic.hlinkClick_id = malloc(len_hlinkClickId + 1);
	memcpy(drawing_callbackdata->twocellanchor.pic.hlinkClick_id, attrs[i + 1], len_hlinkClickId + 1);
      }
    }
  } else if (strcmp(name, "a:ext") == 0) {
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "cx") == 0) {
	sscanf(attrs[i + 1], "%zu", &drawing_callbackdata->twocellanchor.pic.cx);
      } else if (strcmp(attrs[i], "cy") == 0) {
	sscanf(attrs[i + 1], "%zu", &drawing_callbackdata->twocellanchor.pic.cy);
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, drawings_lv4_end_element);
}

void drawings_lv4_end_element(void *callbackdata, const XML_Char *name) {
  printf("LV44444444444444444: </%s>\n", name);
  XML_SetElementHandler(xmlparser, drawings_lv4_start_element, drawings_lv3_end_element);
}

void drawings_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (drawing_callbackdata->text == NULL) {
    if ((drawing_callbackdata->text = realloc(drawing_callbackdata->text, len + 1)) == NULL) {
      return;
    }
    memcpy(drawing_callbackdata->text, buf, len);
    drawing_callbackdata->text[len] = '\0';
  } else {
    int len_text = strlen(drawing_callbackdata->text);
    if ((drawing_callbackdata->text = realloc(drawing_callbackdata->text, len_text + len)) == NULL) {
      return;
    }
    memcpy(drawing_callbackdata->text + len_text - 1, buf, len);
    drawing_callbackdata->text[len_text + len - 1] = '\0';
  }
}
