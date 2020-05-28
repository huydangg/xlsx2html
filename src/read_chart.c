#include <read_chart.h>
#include <string.h>
#include <stdio.h>


int chart_callbackdata_initialize (struct ChartCallBackData *data, char* json_path, int index_sheet) {
  data->fchart = fopen(json_path, "wb");
  if (data->fchart == NULL) {
    fprintf(stderr, "Cannot open %s to write\n", json_path);
    return -1;
  }
  data->text = NULL;
  data->textlen = 0;
  data->skiptag = NULL;
  data->skiptagcount = 0;
  data->skip_start = NULL;
  data->skip_end = NULL;
  data->skip_data = NULL;
  data->index_sheet = index_sheet;
  data->mark_to_insert_commas = '0';
  data->array_charts_length = 0;
  data->array_sers_length = 0;
  data->array_vals_length = 0;
  data->array_cats_length = 0;
  data->is_val = '0';
  data->has_tx = '0'; // check tx tag is visiable or not
  data->f = NULL;
  return 1;
}

void chart_skip_tag_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (name && strcmp(name, chart_callbackdata->skiptag) == 0) {
    chart_callbackdata->skiptagcount++;
  }
}

void chart_skip_tag_end_element(void *callbackdata, const XML_Char *name) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (!name || strcmp(name, chart_callbackdata->skiptag) == 0) {
    if (--chart_callbackdata->skiptagcount == 0) {
      XML_SetElementHandler(xmlparser, chart_callbackdata->skip_start, chart_callbackdata->skip_end);
      XML_SetCharacterDataHandler(xmlparser, chart_callbackdata->skip_data);
      free(chart_callbackdata->skiptag);
      chart_callbackdata->skiptag = NULL;
    }
  }
}

void chart_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:chart") == 0) {
    fputs("{", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_lv1_start_element, NULL);
  }
}

void chart_end_element(void *callbackdata, const XML_Char *name) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:chart") == 0) {
    fputs("}", chart_callbackdata->fchart);
  }
}

void chart_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:title") == 0) {
    chart_callbackdata->mark_to_insert_commas = '1';
    fputs("\"title\": {", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_title_item_start_element, chart_title_item_end_element);
  } else if (strcmp(name, "c:autoTitleDeleted") == 0){
    XML_SetElementHandler(xmlparser, NULL, chart_lv1_end_element);
  } else if (strcmp(name, "c:plotArea") == 0) {
    if (chart_callbackdata->mark_to_insert_commas == '1') {
      fputs(",", chart_callbackdata->fchart);
      chart_callbackdata->mark_to_insert_commas = '0';
    }
    fputs("\"charts\":", chart_callbackdata->fchart);
    fputs("[", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_plotArea_item_start_element, chart_plotArea_item_end_element);
  }
}

void chart_lv1_end_element(void *callbackdata, const XML_Char *name) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:title") == 0) {
    fputs("}", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_lv1_start_element, chart_end_element);
  } else if (strcmp(name, "c:autoTitleDeleted") == 0){
    XML_SetElementHandler(xmlparser, chart_lv1_start_element, chart_end_element);
  } else if (strcmp(name, "c:plotArea") == 0) {
    fputs("]", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_lv1_start_element, chart_end_element);
  }
}

void chart_title_item_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "a:pPr") == 0) {
    chart_callbackdata->skiptag = strdup(name);
    chart_callbackdata->skiptagcount = 1;
    chart_callbackdata->skip_start = NULL;
    chart_callbackdata->skip_end = chart_title_item_end_element;
    chart_callbackdata->skip_data = NULL;
    XML_SetElementHandler(xmlparser, chart_skip_tag_start_element, chart_skip_tag_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (strcmp(name, "a:latin") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "typeface") == 0) {
	fputs("\"font_name\":", chart_callbackdata->fchart);
	fputs("\"", chart_callbackdata->fchart);
	fputs(attrs[i + 1], chart_callbackdata->fchart);
	fputs("\"", chart_callbackdata->fchart);
      }
    }
  } else if (strcmp(name, "a:t") == 0) {
    fputs(",", chart_callbackdata->fchart);
    fputs("\"text\":", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_title_item_start_element, chart_title_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, chart_content_handler);
  }
}

void chart_title_item_end_element(void *callbackdata, const XML_Char *name) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "a:t") == 0) {
    fputs("\"", chart_callbackdata->fchart);
    chart_callbackdata->text[chart_callbackdata->textlen] = '\0';
    fputs(chart_callbackdata->text, chart_callbackdata->fchart);
    fputs("\"", chart_callbackdata->fchart);
    free(chart_callbackdata->text);
    chart_callbackdata->text = NULL;
    chart_callbackdata->textlen = 0;
  } else if (strcmp(name, "c:title") == 0) {
    chart_lv1_end_element(callbackdata, name);
  } else {
    XML_SetElementHandler(xmlparser, chart_title_item_start_element, chart_title_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void chart_plotArea_item_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:layout") == 0) {
    chart_callbackdata->skiptag = strdup(name);
    chart_callbackdata->skiptagcount = 1;
    chart_callbackdata->skip_start = chart_plotArea_item_start_element;
    chart_callbackdata->skip_end = chart_plotArea_item_end_element;
    chart_callbackdata->skip_data = NULL;
    XML_SetElementHandler(xmlparser, chart_skip_tag_start_element, chart_skip_tag_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (strcmp(name, "c:barChart") == 0) {
    chart_callbackdata->array_charts_length++;
    if (chart_callbackdata->array_charts_length > 1)
      fputs(",", chart_callbackdata->fchart);
    fputs("{", chart_callbackdata->fchart);
    fputs("\"type\":\"barChart\"", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
  } else if (strcmp(name, "c:bar3DChart") == 0) {
    chart_callbackdata->array_charts_length++;
    if (chart_callbackdata->array_charts_length > 1)
      fputs(",", chart_callbackdata->fchart);
    fputs("{", chart_callbackdata->fchart);
    fputs("\"type\":\"bar3DChart\"", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
  } else if (strcmp(name, "c:lineChart") == 0) {
    chart_callbackdata->array_charts_length++;
    if (chart_callbackdata->array_charts_length > 1)
      fputs(",", chart_callbackdata->fchart);
    fputs("{", chart_callbackdata->fchart);
    fputs("\"type\":\"lineChart\"", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
  } else if (strcmp(name, "c:pieChart") == 0) {
    chart_callbackdata->array_charts_length++;
    if (chart_callbackdata->array_charts_length > 1)
      fputs(",", chart_callbackdata->fchart);
    fputs("{", chart_callbackdata->fchart);
    fputs("\"type\":\"pieChart\"", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
  } else if (strcmp(name, "c:pie3DChart") == 0) {
    chart_callbackdata->array_charts_length++;
    if (chart_callbackdata->array_charts_length > 1)
      fputs(",", chart_callbackdata->fchart);
    fputs("{", chart_callbackdata->fchart);
    fputs("\"type\":\"pie3DChart\"", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
  } else if (strcmp(name, "c:areaChart") == 0) {
    chart_callbackdata->array_charts_length++;
    if (chart_callbackdata->array_charts_length > 1)
      fputs(",", chart_callbackdata->fchart);
    fputs("{", chart_callbackdata->fchart);
    fputs("\"type\":\"areaChart\"", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
  } else if (strcmp(name, "c:area3DChart") == 0) {
    chart_callbackdata->array_charts_length++;
    if (chart_callbackdata->array_charts_length > 1)
      fputs(",", chart_callbackdata->fchart);
    fputs("{", chart_callbackdata->fchart);
    fputs("\"type\":\"area3DChart\"", chart_callbackdata->fchart);
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
  }
}

void chart_plotArea_item_end_element(void *callbackdata, const XML_Char *name) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:plotArea") == 0) {
    chart_lv1_end_element(callbackdata, name);
  } else {
    if (strcmp(name, "c:barChart") == 0) {
      chart_callbackdata->array_sers_length = 0;
      fputs("]", chart_callbackdata->fchart);
      fputs("}", chart_callbackdata->fchart);
    } else if (strcmp(name, "c:bar3DChart") == 0) {
      chart_callbackdata->array_sers_length = 0;
      fputs("]", chart_callbackdata->fchart);
      fputs("}", chart_callbackdata->fchart);
    } else if (strcmp(name, "c:lineChart") == 0) {
      chart_callbackdata->array_sers_length = 0;
      fputs("]", chart_callbackdata->fchart);
      fputs("}", chart_callbackdata->fchart);
    } else if (strcmp(name, "c:pieChart") == 0) {
      chart_callbackdata->array_sers_length = 0;
      fputs("]", chart_callbackdata->fchart);
      fputs("}", chart_callbackdata->fchart);
    } else if (strcmp(name, "c:pie3DChart") == 0) {
      chart_callbackdata->array_sers_length = 0;
      fputs("]", chart_callbackdata->fchart);
      fputs("}", chart_callbackdata->fchart);
    } else if (strcmp(name, "c:areaChart") == 0) {
      chart_callbackdata->array_sers_length = 0;
      fputs("]", chart_callbackdata->fchart);
      fputs("}", chart_callbackdata->fchart);
    } else if (strcmp(name, "c:area3DChart") == 0) {
      chart_callbackdata->array_sers_length = 0;
      fputs("]", chart_callbackdata->fchart);
      fputs("}", chart_callbackdata->fchart);
    }
    XML_SetElementHandler(xmlparser, chart_plotArea_item_start_element, chart_plotArea_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void chart_barChart_item_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:barDir") == 0) {
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "val") == 0) {
        fputs(",", chart_callbackdata->fchart);
	fputs("\"barDir\":", chart_callbackdata->fchart);
	fputs("\"", chart_callbackdata->fchart);
	fputs(attrs[i + 1], chart_callbackdata->fchart);
	fputs("\"", chart_callbackdata->fchart);
      }
    }
  } else if (strcmp(name, "c:ser") == 0) {
    chart_callbackdata->array_sers_length++;
    if (chart_callbackdata->array_sers_length > 1)
      fputs(",", chart_callbackdata->fchart);
    else {
      fputs(",", chart_callbackdata->fchart);
      fputs("\"sers\":", chart_callbackdata->fchart);
      fputs("[", chart_callbackdata->fchart);
    }
    fputs("{", chart_callbackdata->fchart);
  } else if (strcmp(name, "c:tx") == 0) {
    chart_callbackdata->has_tx = '1';
    fputs("\"tx\":", chart_callbackdata->fchart);
  } else if (strcmp(name, "c:cat") == 0) {
    chart_callbackdata->mark_to_insert_commas = '1';
    if (chart_callbackdata->has_tx == '1') {
     fputs(",", chart_callbackdata->fchart);
     chart_callbackdata->has_tx = '0';
    }
    fputs("\"cat\": [", chart_callbackdata->fchart);
    chart_callbackdata->array_cats_length = 1;
  } else if (strcmp(name, "c:val") == 0) {
    chart_callbackdata->is_val = '1';
    if (chart_callbackdata->mark_to_insert_commas == '1')
      fputs(",", chart_callbackdata->fchart);
    fputs("\"val\": [", chart_callbackdata->fchart);
    chart_callbackdata->array_vals_length = 1;
  } else if (strcmp(name, "c:f") == 0) {
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, chart_content_handler);
  } else if (strcmp(name, "c:v") == 0) {
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, chart_content_handler);
  }
}

void chart_barChart_item_end_element(void *callbackdata, const XML_Char *name) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:barChart") == 0) {
    chart_plotArea_item_end_element(callbackdata, name);
  } else if (strcmp(name, "c:bar3DChart") == 0) {
    chart_plotArea_item_end_element(callbackdata, name);
  } else if (strcmp(name, "c:lineChart") == 0) {
    chart_plotArea_item_end_element(callbackdata, name);
  } else if (strcmp(name, "c:pieChart") == 0) {
    chart_plotArea_item_end_element(callbackdata, name);
  } else if (strcmp(name, "c:pie3DChart") == 0) {
    chart_plotArea_item_end_element(callbackdata, name);
  } else if (strcmp(name, "c:areaChart") == 0) {
    chart_plotArea_item_end_element(callbackdata, name);
  } else if (strcmp(name, "c:area3DChart") == 0) {
    chart_plotArea_item_end_element(callbackdata, name);
  }
  else {
    if (strcmp(name, "c:ser") == 0) {
      fputs("}", chart_callbackdata->fchart);
    } else if (strcmp(name, "c:tx") == 0) {

    } else if (strcmp(name, "c:cat") == 0) {
      fputs("]", chart_callbackdata->fchart);
      chart_callbackdata->array_cats_length = 0;

    } else if (strcmp(name, "c:val") == 0) {
      chart_callbackdata->is_val = '0';
      fputs("]", chart_callbackdata->fchart);
      chart_callbackdata->array_vals_length = 0;
      fputs(",", chart_callbackdata->fchart);
      fputs("\"f\":", chart_callbackdata->fchart);
      fputs("\"", chart_callbackdata->fchart);
      fputs(chart_callbackdata->f, chart_callbackdata->fchart);
      free(chart_callbackdata->f);
      chart_callbackdata->f = NULL;
      fputs("\"", chart_callbackdata->fchart);
    } else if (strcmp(name, "c:f") == 0) {
      chart_callbackdata->text[chart_callbackdata->textlen] = '\0';
      chart_callbackdata->f = strdup(chart_callbackdata->text);
      free(chart_callbackdata->text);
      chart_callbackdata->text = NULL;
      chart_callbackdata->textlen = 0;
    } else if (strcmp(name, "c:v") == 0) {
      if (chart_callbackdata->array_cats_length > 1 || chart_callbackdata->array_vals_length > 1)
        fputs(",", chart_callbackdata->fchart);
      if (chart_callbackdata->array_cats_length > 0)
        chart_callbackdata->array_cats_length++;
      if (chart_callbackdata->array_vals_length > 0)
	chart_callbackdata->array_vals_length++;
      if (chart_callbackdata->is_val == '0')
        fputs("\"", chart_callbackdata->fchart);
      if (chart_callbackdata->text != NULL) {
	chart_callbackdata->text[chart_callbackdata->textlen] = '\0';
        fputs(chart_callbackdata->text, chart_callbackdata->fchart);
	free(chart_callbackdata->text);
	chart_callbackdata->text = NULL;
	chart_callbackdata->textlen = 0;
      } else {
	if (chart_callbackdata->is_val == '1') {
          fputs("null", chart_callbackdata->fchart);
	}
      }
      if (chart_callbackdata->is_val == '0')
        fputs("\"", chart_callbackdata->fchart);
    }
    XML_SetElementHandler(xmlparser, chart_barChart_item_start_element, chart_barChart_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void chart_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if ((chart_callbackdata->text = realloc(chart_callbackdata->text, chart_callbackdata->textlen + len + 1)) == NULL) {
    chart_callbackdata->textlen = 0;
  } else {
    memcpy(chart_callbackdata->text + chart_callbackdata->textlen, buf, len);
    chart_callbackdata->textlen += len;
  }
}
