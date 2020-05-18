#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <read_drawings.h>
#include <private.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <read_chart.h>
#include <main.h>

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
  twocellanchor.editAs = NULL;
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


/* 
  int len_chunks_dir_path = strlen(OUTPUT_DIR) + strlen(CHUNKS_DIR_NAME) + 1;
  data->chunks_dir_path = malloc(len_chunks_dir_path + 1);
  snprintf(data->chunks_dir_path, len_chunks_dir_path + 1, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);

  printf("---------------------------------------------------CHART--------------------------------------\n");
  char *zip_file_name = "xl/charts/chart16.xml";
  struct ChartCallBackData chart_callbackdata;
  chart_callbackdata_initialize(&chart_callbackdata, 0);
  int status_chart = load_chart(zip, zip_file_name, &chart_callbackdata);
  if (status_chart != 1) {
    fprintf(stderr, "%s\n", strerror(errno));
  }
  printf("--------------------------------------------------------------------------------------------------\n");
*/
int load_chart(zip_t *zip, char *zip_file_name, void *callbackdata, XML_Parser *xmlparser_chart) {
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  zip_error_t *err_zip = zip_get_error(zip);
  if (archive == NULL) {
    printf("%s\n", zip_error_strerror(err_zip));
    return -1;
  }
  int status = process_zip_file(xmlparser_chart, archive, callbackdata, NULL, chart_start_element, chart_end_element);
  return status;
}

void drawings_callbackdata_initialize (struct DrawingCallbackData *data, struct ArrayRelationships *array_drawing_rels, FILE *findexhtml, zip_t *zip, int index_sheet) {
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
  data->index_sheet = index_sheet;
  data->index_image = -1;
  data->index_graphicframe = -1;
  data->is_pic = '0';
  data->is_graphicframe = '0';
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
    char *drawing_id = NULL; 
    if (drawing_callbackdata->is_pic == '1') {
      drawing_id = strdup(drawing_callbackdata->twocellanchor.pic.blip_embed);
    }
    if (drawing_callbackdata->is_graphicframe == '1') {
      drawing_id = strdup(drawing_callbackdata->twocellanchor.graphic_frame.chart_id);
    }
    if (drawing_id != NULL) {
      for (int i = 0; i < drawing_callbackdata->array_drawing_rels->length; i++) {
	if (strcmp(drawing_id, drawing_callbackdata->array_drawing_rels->relationships[i]->id) == 0) {
	  int len_target = strlen(drawing_callbackdata->array_drawing_rels->relationships[i]->target);
	  char *_tmp_target = malloc(len_target + 1);
	  snprintf(_tmp_target, len_target + 1, "xl%s", drawing_callbackdata->array_drawing_rels->relationships[i]->target + 2);
	  int len_index_sheet = snprintf(NULL, 0, "%d", drawing_callbackdata->index_sheet);

	  if (drawing_callbackdata->is_pic == '1') {
	    drawing_callbackdata->index_image++;
	    struct zip_stat sb;
            struct zip_file *img_zf;
	    int img_fd;
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
	      free(img_zf_name);
	      int len_output_img_file_path = strlen(OUTPUT_DIR) + strlen(img_name) + 1;
	      char *OUTPUT_IMG_FILE_PATH = malloc(len_output_img_file_path + 1);
	      snprintf(OUTPUT_IMG_FILE_PATH, len_output_img_file_path + 1, "%s/%s", OUTPUT_DIR, img_name);
	      img_fd = open(OUTPUT_IMG_FILE_PATH, O_WRONLY | O_CREAT, 0644);
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
		IMG_URL = strdup(OUTPUT_IMG_FILE_PATH);
		len_img_url = len_output_img_file_path;
		free(OUTPUT_IMG_FILE_PATH);
	      }
	      free(img_name);
	      // http://officeopenxml.com/drwPicInSpread-oneCell.php
	      // EMUs to pixels: value / 9525 (1 pixel = 9525 EMUs)
	      size_t height = drawing_callbackdata->twocellanchor.pic.cy / 9525;
	      size_t width = drawing_callbackdata->twocellanchor.pic.cx / 9525;
	      int len_height = snprintf(NULL, 0, "%zu", height);
	      int len_width = snprintf(NULL, 0, "%zu", width);
	      size_t from_colOff = drawing_callbackdata->twocellanchor.from.colOff / 9525;
	      size_t from_rowOff = drawing_callbackdata->twocellanchor.from.rowOff / 9525;
	      int len_from_colOff = snprintf(NULL, 0, "%zu", from_colOff);
	      int len_from_rowOff = snprintf(NULL, 0, "%zu", from_rowOff);
	      int len_index_img = snprintf(NULL, 0, "%d", i);
	      int len_from_row = snprintf(NULL, 0, "%u", drawing_callbackdata->twocellanchor.from.row);
	      char *from_col_name = int_to_column_name(drawing_callbackdata->twocellanchor.from.col);
	      int len_from_col_name = strlen(from_col_name);
	      int len_div_img = len_index_sheet + len_index_img + len_img_url
		+ len_height + len_width + len_from_col_name + len_from_row
		+ len_from_colOff + len_from_rowOff + 141;
	      char *DIV_IMG = malloc(len_div_img + 1);
	      snprintf(
		DIV_IMG, len_div_img + 1,
		"<div id=\"chunk_%d_%d_img\" data-img-url=\"%s\" data-height=\"%zu\" data-width=\"%zu\" data-from-col=\"%s\" data-from-row=\"%u\" data-from-coloff=\"%zu\" data-from-rowoff=\"%zu\">",
		drawing_callbackdata->index_sheet,
		drawing_callbackdata->index_image,
		IMG_URL, height, width,
		from_col_name,
		drawing_callbackdata->twocellanchor.from.row,
		from_colOff, from_rowOff
	      );
	      fputs(DIV_IMG, drawing_callbackdata->findexhtml);
	      fputs("</div>", drawing_callbackdata->findexhtml);
	      fputs("\n", drawing_callbackdata->findexhtml);
	      free(DIV_IMG);
	      free(from_col_name);
	      free(IMG_URL);
	    }
	  } else if (drawing_callbackdata->is_graphicframe == '1') {
	    drawing_callbackdata->index_graphicframe++;
	    //chunk_%d_%d_chart.json
	    int len_index_graphicframe = snprintf(NULL, 0, "%d", drawing_callbackdata->index_graphicframe);
	    int len_chart_json_file_name = len_index_sheet + len_index_graphicframe + 18;
	    char *chart_json_file_name = malloc(len_chart_json_file_name + 1);
	    snprintf(
              chart_json_file_name, len_chart_json_file_name + 1,
	      "chunk_%d_%d_chart.json", drawing_callbackdata->index_sheet,
	      drawing_callbackdata->index_graphicframe
	    );

            int len_chart_json_file_path = strlen(OUTPUT_DIR) + strlen(CHUNKS_DIR_NAME) + len_chart_json_file_name + 1;
            char *chart_json_file_path = malloc(len_chart_json_file_path + 1);
            snprintf(chart_json_file_path, len_chart_json_file_path + 1, "%s/%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME, chart_json_file_name);
	    free(chart_json_file_name);
	    XML_Parser xmlparser_chart;
	    xmlparser_chart = XML_ParserCreate(NULL);
            struct ChartCallBackData chart_callbackdata;
            chart_callbackdata_initialize(&chart_callbackdata, chart_json_file_path, drawing_callbackdata->index_sheet, &xmlparser_chart);
	    int status_chart = load_chart(drawing_callbackdata->zip, _tmp_target, &chart_callbackdata, &xmlparser_chart);
	    free(chart_json_file_path);
	    if (status_chart != 1) {
	      //fprintf(stderr, "%s\n", strerror(errno));
	    }

	  }

	  free(_tmp_target);

	}
      }
      free(drawing_callbackdata->twocellanchor.pic.name);
      free(drawing_callbackdata->twocellanchor.pic.blip_embed);
      free(drawing_callbackdata->twocellanchor.pic.hlinkClick_id);
    }
    if (drawing_callbackdata->twocellanchor.editAs != NULL) {
      free(drawing_callbackdata->twocellanchor.editAs);
    }
    drawing_callbackdata->twocellanchor = new_twocellanchor();
  }
  drawing_callbackdata->is_pic = '0';
  drawing_callbackdata->is_graphicframe = '0';
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
    drawing_callbackdata->is_pic = '1';
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  } else if (strcmp(name, "xdr:sp") == 0) {
    free(drawing_callbackdata->twocellanchor.editAs);
    drawing_callbackdata->twocellanchor = new_twocellanchor();
    drawing_callbackdata->skiptag = strdup(name);
    drawing_callbackdata->skiptagcount = 1;
    drawing_callbackdata->skip_start = NULL;
    drawing_callbackdata->skip_end = drawings_lv1_end_element;
    drawing_callbackdata->skip_data = NULL;
    XML_SetElementHandler(xmlparser, drawings_skip_tag_start_element, drawings_skip_tag_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (strcmp(name, "xdr:graphicFrame") == 0) {
    drawing_callbackdata->is_graphicframe = '1';
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, NULL);
  }
}

void drawings_lv1_end_element(void *callbackdata, const XML_Char *name) {
  printf("LV111111111: </%s>\n", name);
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (strcmp(name, "xdr:from") == 0) {
    if (drawing_callbackdata->_tmp_offset.col != 0) {
      drawing_callbackdata->twocellanchor.from.col = drawing_callbackdata->_tmp_offset.col;
      drawing_callbackdata->twocellanchor.from.colOff = drawing_callbackdata->_tmp_offset.colOff;
      drawing_callbackdata->twocellanchor.from.row = drawing_callbackdata->_tmp_offset.row;
      drawing_callbackdata->twocellanchor.from.rowOff = drawing_callbackdata->_tmp_offset.rowOff;
      drawing_callbackdata->_tmp_offset = new_offset();
    }
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, drawings_lv1_end_element);
  } else if (strcmp(name, "xdr:to") == 0) {
    if (drawing_callbackdata->_tmp_offset.col != 0) {
      drawing_callbackdata->twocellanchor.to.col = drawing_callbackdata->_tmp_offset.col;
      drawing_callbackdata->twocellanchor.to.colOff = drawing_callbackdata->_tmp_offset.colOff;
      drawing_callbackdata->twocellanchor.to.row = drawing_callbackdata->_tmp_offset.row;
      drawing_callbackdata->twocellanchor.to.rowOff = drawing_callbackdata->_tmp_offset.rowOff;
      drawing_callbackdata->_tmp_offset = new_offset();
    }
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, drawings_lv1_end_element);
  } else if (strcmp(name, "xdr:pic") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, drawings_lv1_end_element);
  } else if (strcmp(name, "xdr:clientData") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, drawings_end_element);
  } else if (strcmp(name, "xdr:graphicFrame") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv1_start_element, drawings_lv1_end_element);
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
  } else if (strcmp(name, "xdr:nvGraphicFramePr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  } else if (strcmp(name, "a:graphic") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv3_start_element, NULL);
  }
}

void drawings_lv2_end_element(void *callbackdata, const XML_Char *name) {
  printf("LV222222222: </%s>\n", name);
  struct DrawingCallbackData *drawing_callbackdata = callbackdata;
  if (strcmp(name, "xdr:col") == 0) {
    if (drawing_callbackdata->text != NULL) {
      drawing_callbackdata->_tmp_offset.col = strtol(drawing_callbackdata->text, NULL, 10) + 1;
      free(drawing_callbackdata->text);
      drawing_callbackdata->text = NULL;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:colOff") == 0) {
    if (drawing_callbackdata->text != NULL) {
      drawing_callbackdata->_tmp_offset.colOff = strtol(drawing_callbackdata->text, NULL, 10);
      free(drawing_callbackdata->text);
      drawing_callbackdata->text = NULL;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:row") == 0) {
    if (drawing_callbackdata->text != NULL) {
      drawing_callbackdata->_tmp_offset.row = strtol(drawing_callbackdata->text, NULL, 10) + 1;
      free(drawing_callbackdata->text);
      drawing_callbackdata->text = NULL;
    }
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if(strcmp(name, "xdr:rowOff") == 0) {
    if (drawing_callbackdata->text != NULL) {
      drawing_callbackdata->_tmp_offset.rowOff = strtol(drawing_callbackdata->text, NULL, 10);
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
  } else if (strcmp(name, "xdr:nvGraphicFramePr") == 0) {
    XML_SetElementHandler(xmlparser, drawings_lv2_start_element, drawings_lv1_end_element);
  } else if (strcmp(name, "a:graphic") == 0) {
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
	if (drawing_callbackdata->is_pic == '1') {
          drawing_callbackdata->twocellanchor.pic.name = malloc(len_name + 1);
          memcpy(drawing_callbackdata->twocellanchor.pic.name, attrs[i + 1], len_name + 1);
	} else if (drawing_callbackdata->is_graphicframe == '1') {
          drawing_callbackdata->twocellanchor.graphic_frame.name = malloc(len_name + 1);
          memcpy(drawing_callbackdata->twocellanchor.graphic_frame.name, attrs[i + 1], len_name + 1);
	}
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
  } else if (strcmp(name, "a:graphicData") == 0) {
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
  } else if (strcmp(name, "a:graphicData") == 0) {
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
  } else if (strcmp(name, "c:chart") == 0) {
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r:id") == 0) {
	drawing_callbackdata->twocellanchor.graphic_frame.chart_id = strdup(attrs[i + 1]);
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
