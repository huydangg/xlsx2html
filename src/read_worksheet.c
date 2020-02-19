#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <read_worksheet.h>

struct ArrayCols array_cols;
char start_col = 'A'; // A
char *end_col;
char start_row  = '1';// 1
char *end_row;


void get_end_col_end_row_from_range(XML_Char *range, char *end_row, char *end_col) {
  // ex: A1:Q109
  int pos_colon = 0;
  char *check_colon = strchr(range, ':');
  if (check_colon){
    pos_colon = check_colon - range + 1;
  }
  char *_tmp_end_cell = range + pos_colon;
  int length = (int)strlen(_tmp_end_cell);
  char _tmp_end_col[3];
  int count_col_char = 0;
  while (*_tmp_end_cell) {
    if (*_tmp_end_cell >= 'A' && *_tmp_end_cell <= 'Z') {
      count_col_char++; 
      _tmp_end_col[count_col_char - 1] = *_tmp_end_cell;
    } else {
      end_row = malloc(sizeof(_tmp_end_cell));
      memcpy(end_row, _tmp_end_cell, sizeof(_tmp_end_cell));
      end_row[strlen(_tmp_end_cell)] = '\0';
      break;
    }
    _tmp_end_cell++;
  }
  end_col = malloc((count_col_char + 1) * sizeof(char));
  end_col = _tmp_end_col;
  memcpy(end_col, _tmp_end_col, sizeof(_tmp_end_col));
  end_col[count_col_char] = '\0';
  printf("END COL: %s\n", end_col);
  printf("END ROW: %s\n", end_row);
}

void worksheet_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name, "sheetPr") == 0) {
    // TODO: tabColor
  } else if (strcmp(name, "dimension") == 0) {
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "ref") == 0) {
        get_end_col_end_row_from_range(attrs[i + 1], end_row, end_col);
      }
    }
  } else if (strcmp(name, "cols") == 0) {
    array_cols.length = 0;
    array_cols.cols = malloc(sizeof(struct Col *));
    if (array_cols.cols == NULL) {
	fprintf(stderr, "Error when allocted array_cols.cols");
	// TODO: Handle Error
    } else {
      XML_SetElementHandler(xmlparser, col_start_element, NULL);
    }
  }
}

void worksheet_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, worksheet_start_element, NULL);
}

void col_start_element(void *userData, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name, "col") == 0) {
    struct Col **_tmp_cols;
    array_cols.length++;
    if (array_cols.length > 1) {
      _tmp_cols = realloc(array_cols.cols, array_cols.length * sizeof(struct Col *));
      if (_tmp_cols) {
	array_cols.cols = _tmp_cols;
      } else {
	fprintf(stderr, "Error when allocted array_cols.cols");
	// TODO: Handle Error
      }
    }
    array_cols.cols[array_cols.length - 1] = malloc(sizeof(struct Col));
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "hidden") == 0) {
        array_cols.cols[array_cols.length - 1]->isHidden = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0'; 
      } else if (strcmp(attrs[i], "min") == 0) {
	array_cols.cols[array_cols.length - 1]->min = (unsigned short int)strtol((char *)attrs[i + 1], NULL, 10);
      } else if (strcmp(attrs[i], "max") == 0) {
	array_cols.cols[array_cols.length - 1]->max = (unsigned short int)strtol((char *)attrs[i + 1], NULL, 10);
      } else if (strcmp(attrs[i], "width") == 0) {
	array_cols.cols[array_cols.length - 1]->width = strtof((char *)attrs[i + 1], NULL);
      }
    }
  }
  XML_SetElementHandler(xmlparser, NULL, col_end_element);
}

void col_end_element(void *userData, const XML_Char *name) {
  XML_SetElementHandler(xmlparser, col_start_element, worksheet_end_element);
}

void content_handler(void *userData, const XML_Char *s, int len) {
  if (len == 0){
    return;
  }
  char *value = malloc((len + 1) * sizeof(XML_Char));
  strncpy(value, s, len);
  free(value);
}

