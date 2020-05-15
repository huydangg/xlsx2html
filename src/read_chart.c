#include <read_chart.h>
#include <string.h>
#include <stdio.h>


int chart_callbackdata_initialize (struct ChartCallBackData *data, char* json_path, int index_sheet, XML_Parser *xmlparser_chart) {
  FILE *fchart;
  fchart = fopen(json_path, "wb");
  if (fchart == NULL) {
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
  data->xmlparser = xmlparser_chart;
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
      XML_SetElementHandler(chart_callbackdata ->xmlparser, chart_callbackdata->skip_start, chart_callbackdata->skip_end);
      XML_SetCharacterDataHandler(chart_callbackdata->xmlparser, chart_callbackdata->skip_data);
      free(chart_callbackdata->skiptag);
      chart_callbackdata->skiptag = NULL;
    }
  }
}

void chart_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  printf("%s\n", name);

  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:chart") == 0) {
    XML_SetElementHandler(chart_callbackdata->xmlparser, chart_lv1_start_element, NULL);
  }

}

void chart_end_element(void *callbackdata, const XML_Char *name) {

}

void chart_lv1_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:chart") == 0) {
    XML_SetElementHandler(chart_callbackdata->xmlparser, chart_lv1_start_element, NULL);
  }

}

void chart_lv1_end_element(void *callbackdata, const XML_Char *name) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (strcmp(name, "c:title") == 0) {
    XML_SetElementHandler(chart_callbackdata->xmlparser, chart_lv1_start_element, NULL);
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
    XML_SetElementHandler(chart_callbackdata->xmlparser, chart_skip_tag_start_element, chart_skip_tag_end_element);
    XML_SetCharacterDataHandler(chart_callbackdata->xmlparser, NULL);
  }
}

void chart_title_item_end_element(void *callbackdata, const XML_Char *name) {
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  XML_SetElementHandler(chart_callbackdata ->xmlparser, chart_title_item_start_element, chart_lv1_end_element);
}

void chart_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  struct ChartCallBackData *chart_callbackdata = callbackdata;
  if (chart_callbackdata->text == NULL) {
    if ((chart_callbackdata->text = realloc(chart_callbackdata->text, len + 1)) == NULL) {
      return;
    }
    memcpy(chart_callbackdata->text, buf, len);
    chart_callbackdata->text[len] = '\0';
  } else {
    int len_text = strlen(chart_callbackdata->text);
    if ((chart_callbackdata->text = realloc(chart_callbackdata->text, len_text + len)) == NULL) {
      return;
    }
    memcpy(chart_callbackdata->text + len_text - 1, buf, len);
    chart_callbackdata->text[len_text + len - 1] = '\0';
  }
}
