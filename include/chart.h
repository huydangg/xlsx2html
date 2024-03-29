#ifndef INCLUDED_CHART_H
#define INCLUDED_CHART_H

#include <expat.h>
#include <stdio.h>

extern XML_Parser xmlparser;

struct ChartCallBackData {
  XML_Char *text;
  size_t textlen;
  XML_Char *skiptag;                  // tag to skip
  size_t skiptagcount;                // nesting level for current tag to skip
  XML_StartElementHandler skip_start; // start handler to set after skipping
  XML_EndElementHandler skip_end;     // end handler to set after skipping
  XML_CharacterDataHandler skip_data; // data handler to set after skipping
  int index_sheet;
  FILE *fchart;
  unsigned short array_charts_length;
  unsigned short array_sers_length;
  unsigned short array_vals_length;
  unsigned short array_cats_length;
  char is_val;
  XML_Char *f;
  char has_tx;
  char has_title;
  char has_cat;
  char has_f;
};

void chart_start_element(void *, const XML_Char *, const XML_Char **);
void chart_end_element(void *, const XML_Char *);

void chart_lv1_start_element(void *, const XML_Char *, const XML_Char **);
void chart_lv1_end_element(void *, const XML_Char *);

void chart_title_item_start_element(void *, const XML_Char *,
                                    const XML_Char **);
void chart_title_item_end_element(void *, const XML_Char *);
void chart_plotArea_item_start_element(void *, const XML_Char *,
                                       const XML_Char **);
void chart_plotArea_item_end_element(void *, const XML_Char *);
void chart_barChart_item_start_element(void *, const XML_Char *,
                                       const XML_Char **);
void chart_barChart_item_end_element(void *, const XML_Char *);
void chart_content_handler(void *, const XML_Char *, int);

int chart_callbackdata_initialize(struct ChartCallBackData *, char *, int);

#endif
