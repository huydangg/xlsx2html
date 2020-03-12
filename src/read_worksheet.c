#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <read_worksheet.h>
#include <const.h>
#include <sys/stat.h>
#include <sys/types.h>

unsigned int NUM_OF_CELLS;
unsigned int START_CELL_IN_NUMBER_BY_ROW; //default is 1
unsigned int CURRENT_CELL_IN_NUMBER_BY_ROW;
unsigned int CURRENT_CHUNK;
unsigned short NUM_OF_CHUNKS;
unsigned int COUNT_CELLS = 0;


unsigned short column_name_to_number(const char *column_name) {
  const char *col_name = column_name;
  char *base = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int i, j, result = 0;
  for (i = 0, j = strlen(col_name) - 1; i < strlen(col_name); i += 1, j -= 1) {
    const char *ptr = strchr(base, col_name[i]);
    if (!ptr) {
      return -1;
    }
    int index = ptr - base;
    result += (int)pow((double)strlen(base), (double)j) * (index + 1);
  }
  return result;
}

int generate_columns(struct ArrayCols array_cols, unsigned short end_col_number, unsigned short index_worksheet) {
  const char *OUTPUT_ROOT_DIR = "/media/huydang/HuyDang1/xlsxmagic/output";
  const char *CHUNKS_DIR_NAME = "chunks";
  char THE_FIRST_CHUNK_DIR[256];
  snprintf(THE_FIRST_CHUNK_DIR, sizeof(THE_FIRST_CHUNK_DIR), "%s/%s", OUTPUT_ROOT_DIR, CHUNKS_DIR_NAME);
  struct stat st = {0};
  printf("%s\n", THE_FIRST_CHUNK_DIR);
  if (stat(THE_FIRST_CHUNK_DIR, &st) == -1) {
    int status = mkdir(THE_FIRST_CHUNK_DIR, 0777);
    if (status != 0) {
      fprintf(stderr, "Error when create a chunk dir with status is %d\n", status);
      return -1; 
    }
  }
  char THE_FIRST_CHUNK_PATH[256];
  snprintf(THE_FIRST_CHUNK_PATH, sizeof(THE_FIRST_CHUNK_PATH), "%s/chunk_%d_0.html", THE_FIRST_CHUNK_DIR, index_worksheet);
  FILE *fchunk0;
  fchunk0 = fopen(THE_FIRST_CHUNK_PATH, "ab+");
  if (fchunk0 == NULL) {
    fprintf(stderr, "Cannot open chunk0 file to read\n");
    return -1;
  }
  if (end_col_number == -1) {
    fprintf(stderr, "Error when convert column name to number\n");
    return -1;
  }
  fputs("<th width: 35px; height: 15px;></th>", fchunk0);
  for (int i = 1; i <= end_col_number; i++) {
    char TH_STRING[256];
    for (int index_col = 0; index_col < array_cols.length; index_col++) {
      if (i >= array_cols.cols[index_col]->min && i <= array_cols.cols[index_col]->max) {
	snprintf(TH_STRING, sizeof(TH_STRING), "<th style=\"width: %gpx;\">", array_cols.cols[index_col]->width);
        break;
      }
    }
    fputs(TH_STRING, fchunk0);
    char *column_name = int_to_column_name(i);
    printf("COLUMN_NAME: %s\n", column_name);
    fputs(column_name, fchunk0);
    free(column_name);
    fputs("</th>", fchunk0);
  }
  fclose(fchunk0);
  return 1;
}
struct SheetData {
  unsigned short current_cell_remain;
  FILE *worksheet_file;
  unsigned short current_sheet;
  unsigned short end_col_number;
};
// When tag <row> is empty.
void generate_cells(void *callbackdata, const XML_Char *name) {
  printf("NAMEEEEEEEEEEEEEEEEEEEEE CELLLLLLLLLLLLLLLLLLLLL: %s\n", name);
  struct SheetData *sheetData_callbackdata = callbackdata;
  for (int i = 1; i <= sheetData_callbackdata->end_col_number; i++) {
    fputs("<td></td>", sheetData_callbackdata->worksheet_file);
    COUNT_CELLS++;
  }
}
void get_end_col_end_row_from_range(const XML_Char *range, char **end_row, char **end_col) {
  // ex: A1:Q109 or I28
  int pos_colon = 0;
  char *check_colon = strchr(range, ':');
  if (check_colon){
    pos_colon = check_colon - range + 1;
  }
  const XML_Char *_tmp_end_cell = range + pos_colon;
  int length = (int)strlen(_tmp_end_cell);
  char _tmp_end_col[4];
  int count_col_char = 0;
  while (*_tmp_end_cell) {
    if (*_tmp_end_cell >= 'A' && *_tmp_end_cell <= 'Z') {
      count_col_char++; 
      _tmp_end_col[count_col_char - 1] = *_tmp_end_cell;
    } else {
      _tmp_end_col[count_col_char] = '\0';
      *end_col = malloc((count_col_char + 1) * sizeof(char));
      memcpy(*end_col, _tmp_end_col, (count_col_char + 1) * sizeof(char));
      *end_row = malloc(sizeof(char) * (length - count_col_char + 1));
      memcpy(*end_row, _tmp_end_cell, (length - count_col_char + 1) * sizeof(char));
      return;
    }
    _tmp_end_cell++;
  }
}



void worksheet_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct WorkSheet *worksheet_callbackdata = callbackdata;
  if (strcmp(name, "dimension") == 0) {
    char *_tmp_end_row = worksheet_callbackdata->end_row;
    char *_tmp_end_col = worksheet_callbackdata->end_col;
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "ref") == 0) {
        get_end_col_end_row_from_range(attrs[i + 1], &_tmp_end_row, &_tmp_end_col);
        worksheet_callbackdata->end_row = _tmp_end_row;
	worksheet_callbackdata->end_col = _tmp_end_col;
	worksheet_callbackdata->end_col_number = column_name_to_number(_tmp_end_col);
	NUM_OF_CELLS = strtol(_tmp_end_row, NULL, 10) * worksheet_callbackdata->end_col_number;
	NUM_OF_CHUNKS = NUM_OF_CELLS / CHUNK_SIZE_LIMIT;
	if (NUM_OF_CELLS % CHUNK_SIZE_LIMIT != 0) {
          NUM_OF_CHUNKS++;
	}
	break;
      }
    }
    XML_SetElementHandler(xmlparser, NULL, worksheet_end_element);
  } else if (strcmp(name, "cols") == 0) {
    worksheet_callbackdata->array_cols.length = 0;
    worksheet_callbackdata->array_cols.cols = malloc(sizeof(struct Col *));
    if (worksheet_callbackdata->array_cols.cols == NULL) {
	fprintf(stderr, "Error when allocted array_cols.cols");
	// TODO: Handle Error
    } else {
      XML_SetElementHandler(xmlparser, col_row_start_element, NULL);
    }
  } else if (strcmp(name, "sheetData") == 0) {
    START_CELL_IN_NUMBER_BY_ROW = 1;
    CURRENT_CHUNK = 1;
    struct SheetData *sheetData_callbackdata = malloc(sizeof(struct SheetData));
    sheetData_callbackdata->current_sheet = worksheet_callbackdata->index_sheet;
    sheetData_callbackdata->end_col_number = worksheet_callbackdata->end_col_number;
    XML_SetUserData(xmlparser, sheetData_callbackdata);
    XML_SetElementHandler(xmlparser, col_row_start_element, NULL);
  }
}

void worksheet_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "sheetData") == 0) {
    struct SheetData *sheetData_callbackdata = callbackdata;
    fclose(sheetData_callbackdata->worksheet_file);
    free(sheetData_callbackdata);
  }
  XML_SetElementHandler(xmlparser, worksheet_start_element, NULL);
}

void col_row_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (strcmp(name, "col") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    struct Col **_tmp_cols;
    worksheet_callbackdata->array_cols.length++;
    if (worksheet_callbackdata->array_cols.length > 1) {
      _tmp_cols = realloc(worksheet_callbackdata->array_cols.cols, worksheet_callbackdata->array_cols.length * sizeof(struct Col *));
      if (_tmp_cols) {
	worksheet_callbackdata->array_cols.cols = _tmp_cols;
      } else {
	fprintf(stderr, "Error when allocted array_cols.cols");
	// TODO: Handle Error
      }
    }
    worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1] = malloc(sizeof(struct Col));
    worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->isHidden = '0';
    for (int i = 0; attrs[i]; i += 2) {
      if (strcmp(attrs[i], "hidden") == 0) {
        worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->isHidden = strcmp(attrs[i + 1], "true") == 0 ? '1' : '0'; 
      } else if (strcmp(attrs[i], "min") == 0) {
	worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->min = (unsigned short int)strtol((char *)attrs[i + 1], NULL, 10);
      } else if (strcmp(attrs[i], "max") == 0) {
	worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->max = (unsigned short int)strtol((char *)attrs[i + 1], NULL, 10);
      } else if (strcmp(attrs[i], "width") == 0) {
	worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->width = strtof((char *)attrs[i + 1], NULL);
      }
    }
    XML_SetElementHandler(xmlparser, NULL, col_row_end_element);
  } else if(strcmp(name, "row") == 0) {
    //TODO: Need to calculate number of chunks first.
    struct SheetData *sheetData_callbackdata = callbackdata;
    printf("sheetData_callbackdata->current_sheet: %d\n", sheetData_callbackdata->current_sheet);
    char CHUNK_PATH[256];
    snprintf(CHUNK_PATH, sizeof(CHUNK_PATH), "/media/huydang/HuyDang1/xlsxmagic/output/chunks/chunk_%d_%d.html", sheetData_callbackdata->current_sheet, CURRENT_CHUNK);
    sheetData_callbackdata->worksheet_file = fopen(CHUNK_PATH, "wb+");
    char *ROW_NUMBER = NULL;
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r")) {
	int len_row_number = strlen(attrs[i + 1]);
	ROW_NUMBER = realloc(ROW_NUMBER, len_row_number + 1);
	memcpy(ROW_NUMBER, attrs[i + 1], len_row_number + 1);
	int LEN_TR_TAG = 11 + len_row_number;
        char TR_TAG[LEN_TR_TAG];
        snprintf(TR_TAG, LEN_TR_TAG, "<tr id=\"%s\">", ROW_NUMBER);
        fputs(TR_TAG, sheetData_callbackdata->worksheet_file);
      } else if (strcmp(attrs[i], "ht") == 0) {
	//<th style="height:px;"
        int LEN_TH_TAG = 29 + strlen(attrs[i + 1]) + strlen(ROW_NUMBER);
	char TH_TAG[LEN_TH_TAG];
	snprintf(TH_TAG, LEN_TH_TAG, "<th style=\"height:%spx;\">%s</th>", attrs[i + 1], ROW_NUMBER);
	fputs(TH_TAG, sheetData_callbackdata->worksheet_file);
      }
    }
    printf("COUNTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT: %d\n", XML_GetSpecifiedAttributeCount(xmlparser));
    XML_SetElementHandler(xmlparser, cell_start_element, generate_cells);
  }
}


void col_row_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "col") == 0) {

    struct WorkSheet *worksheet_callbackdata = callbackdata;
    int status = generate_columns(worksheet_callbackdata->array_cols, worksheet_callbackdata->end_col_number, worksheet_callbackdata->index_sheet);
    if (status == -1) {
      fprintf(stderr, "Error when generated chunk0 of  Sheet\n");
      return;
    }
    XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
  } else if (strcmp(name, "row") == 0) {
  }
}

void cell_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) { 
  /*XML_AttrInfo *test_info =  XML_GetAttributeInfo(xmlparser);
  printf("ATTR INFO name start: %d\n", test_info[0].nameStart);
  printf("ATTR INFO name end: %d\n", test_info[0].nameEnd);
  printf("ATTR INFO value start: %d\n", test_info[0].valueStart);
  printf("ATTR INFO value end: %d\n", test_info[0].valueEnd);*/
  
  struct SheetData *sheetData_callbackdata = callbackdata;
  if (sheetData_callbackdata->current_sheet == 0) {
  }
}

void cell_end_element(void *callbackdata, const XML_Char *name) {

}

void content_handler(void *callbackdata, const XML_Char *s, int len) {
  if (len == 0){
    return;
  }
  char *value = malloc((len + 1) * sizeof(XML_Char));
  strncpy(value, s, len);
  free(value);
}

