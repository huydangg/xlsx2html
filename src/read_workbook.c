#include <private.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <read_workbook.h>
struct ArraySheets array_sheets;


XML_Char *insert_substr_to_str_at_pos(XML_Char *des, const XML_Char *substr, int pos) {
  const XML_Char *_substr = substr;
  XML_Char *_tmp_sheet_id = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(_substr) + 1));
  memcpy(_tmp_sheet_id, _substr, sizeof(XML_Char) * (XML_Char_len(_substr) + 1));
  XML_Char *_tmp_path_name = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(_tmp_sheet_id) + XML_Char_len(des) + 1));
  memcpy(_tmp_path_name, des, pos);
  _tmp_path_name[pos] = '\0';
  memcpy(_tmp_path_name + pos, _tmp_sheet_id, sizeof(XML_Char) * (XML_Char_len(_tmp_sheet_id) + 1));
  memcpy(_tmp_path_name + pos + XML_Char_len(_tmp_sheet_id), des + pos, sizeof(XML_Char) * (XML_Char_len(des + pos) + 1));
  free(_tmp_sheet_id);
  return _tmp_path_name;
}

void workbook_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (XML_Char_icmp(name, "sheets") == 0) {
    array_sheets.length = 0;
    array_sheets.sheets = XML_Char_malloc(sizeof(struct Sheet *));
    if (array_sheets.sheets == NULL) {
      fprintf(stderr, "Error when XML_Char_malloc sheets_data");
      // TODO: Handle error
    } else {
      XML_SetElementHandler(xmlparser, sheet_main_start_element, NULL);
    }
  }
}

void workbook_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, workbook_start_element, NULL);
}

void sheet_main_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  if (XML_Char_icmp(name, "sheet") == 0){
    struct Sheet **_tmp_sheets_callbackdata;
    array_sheets.length++;
    if (array_sheets.length > 1) {
      _tmp_sheets_callbackdata = realloc(array_sheets.sheets, sizeof(struct Sheet *) * array_sheets.length);
      if (_tmp_sheets_callbackdata) {
	array_sheets.sheets = _tmp_sheets_callbackdata;
      } else {
	fprintf(stderr, "Error when resize sheets");
	array_sheets.length--;
	// TODO: Handle error
      }
    }
    array_sheets.sheets[array_sheets.length - 1] = XML_Char_malloc(sizeof(struct Sheet));
    for(int i = 0; attrs[i]; i += 2){
      if(XML_Char_icmp(attrs[i], "state") == 0){
        array_sheets.sheets[array_sheets.length - 1]->isHidden = XML_Char_icmp(attrs[i + 1], "hidden") == 0 ? '1' : '0';
      }
      if (XML_Char_icmp(attrs[i], "name") == 0){
	array_sheets.sheets[array_sheets.length - 1]->name = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	memcpy(array_sheets.sheets[array_sheets.length - 1]->name, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
      }
      if (XML_Char_icmp(attrs[i], "sheetId") == 0){
	array_sheets.sheets[array_sheets.length - 1]->sheetId = XML_Char_malloc(sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	memcpy(array_sheets.sheets[array_sheets.length - 1]->sheetId, attrs[i + 1], sizeof(XML_Char) * (XML_Char_len(attrs[i + 1]) + 1));
	char *pattern_name = "xl/worksheets/sheet.xml";
	array_sheets.sheets[array_sheets.length - 1]->path_name = insert_substr_to_str_at_pos(pattern_name, attrs[i + 1], 19);
      }
    } 
  }
  XML_SetElementHandler(xmlparser, NULL, sheet_main_end_element);
}

void sheet_main_end_element(void *callbackdata, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, sheet_main_start_element, workbook_end_element);
}
