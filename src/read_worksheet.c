#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <read_worksheet.h>
#include <const.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <read_styles.h>
#include <read_sharedstrings.h>

unsigned int NUM_OF_CELLS;
unsigned int START_CELL_IN_NUMBER_BY_ROW; //default is 1
unsigned int CURRENT_CELL_IN_NUMBER_BY_ROW;
unsigned int CURRENT_CHUNK = 0;
unsigned short NUM_OF_CHUNKS;
unsigned int COUNT_CELLS = 0;
unsigned short INDEX_CURRENT_SHEET;


void reversed(char *input) {
  int length = strlen(input);
  int last_pos = length - 1;
  for (int i = 0; i < length/2; i++) {
    char tmp = input[i];
    input[i] = input[last_pos - i];
    input[last_pos - i] = tmp;
  }
}

char *int_to_column_name(unsigned int n) {
  char *column_name = malloc(4);
  column_name[0] = '\0';
  while (n > 0) {
    n--;
    char _tmp_column_name[2];
    _tmp_column_name[0] = (char)('A' + n%26);
    _tmp_column_name[1] = '\0';
    strcat(column_name, _tmp_column_name);
    n /= 26;
  }
  reversed(column_name);
  column_name[strlen(column_name)] = '\0';
  return column_name;
}

size_t get_col_nr (const XML_Char* A1col) {
  const XML_Char* p = A1col;
  size_t result = 0;
  if (p) {
    while (*p) {
      if (*p >= 'A' && *p <= 'Z')
        result = result * 26 + (*p - 'A') + 1;
      else if (*p >= 'a' && *p <= 'z')
        result = result * 26 + (*p - 'a') + 1;
      else if (*p >= '0' && *p <= '9' && p != A1col)
        return result;
      else
        break;
      p++;
    }
  }
  return 0;
}

//determine row number based on cell coordinate (e.g. "A1"), returns 1-based row number or 0 on error
size_t get_row_nr (const XML_Char* A1col) {
  const XML_Char* p = A1col;
  size_t result = 0;
  if (p) {
    while (*p) {
      if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z'))
        ;
      else if (*p >= '0' && *p <= '9' && p != A1col)
        result = result * 10 + (*p - '0');
      else
        return 0;
      p++;
    }
  }
  return result;
}

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
  int LEN_CHUNKS_DIR_PATH = strlen(OUTPUT_DIR) + strlen(CHUNKS_DIR_NAME) + 1 + 1;
  char *CHUNKS_DIR_PATH = malloc(LEN_CHUNKS_DIR_PATH);
  snprintf(CHUNKS_DIR_PATH, LEN_CHUNKS_DIR_PATH, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);
  struct stat st = {0};
  printf("%s\n", CHUNKS_DIR_PATH);
  if (stat(CHUNKS_DIR_PATH, &st) == -1) {
    int status = mkdir(CHUNKS_DIR_PATH, 0777);
    if (status != 0) {
      fprintf(stderr, "Error when create a chunk dir with status is %d\n", status);
      free(CHUNKS_DIR_PATH);
      return -1; 
    }
  }
  int len_index_worksheet = snprintf(NULL, 0, "%d", index_worksheet);
  int LEN_THE_FIRST_CHUNK_PATH = LEN_CHUNKS_DIR_PATH + len_index_worksheet + 14;
  char *THE_FIRST_CHUNK_PATH = malloc(LEN_THE_FIRST_CHUNK_PATH);
  snprintf(THE_FIRST_CHUNK_PATH, LEN_THE_FIRST_CHUNK_PATH, "%s/chunk_%d_0.html", CHUNKS_DIR_PATH, index_worksheet);
  FILE *fchunk0;
  fchunk0 = fopen(THE_FIRST_CHUNK_PATH, "ab+");
  if (fchunk0 == NULL) {
    fprintf(stderr, "Cannot open chunk0 file to read\n");
    free(THE_FIRST_CHUNK_PATH);
    return -1;
  }
  if (end_col_number == -1) {
    fprintf(stderr, "Error when convert column name to number\n");
    free(THE_FIRST_CHUNK_PATH);
    return -1;
  }
  free(CHUNKS_DIR_PATH);
  free(THE_FIRST_CHUNK_PATH);
  fputs("<tr>", fchunk0);
  fputs("<th style=\"width:35px;height:15px;\"></th>", fchunk0);
  fputs("\n", fchunk0);
  for (int i = 1; i <= end_col_number; i++) {
    char TH_STRING[256];
    for (int index_col = 0; index_col < array_cols.length; index_col++) {
      if (i >= array_cols.cols[index_col]->min && i <= array_cols.cols[index_col]->max) {
	float column_width_in_px = array_cols.cols[index_col]->width * (64 * 1.0 / 8.43);
	snprintf(TH_STRING, sizeof(TH_STRING), "<th style=\"width:%gpx;\">", column_width_in_px);
        break;
      }
    }
    fputs(TH_STRING, fchunk0);
    fputs("\n", fchunk0);
    char *column_name = int_to_column_name((unsigned int)i);
    printf("COLUMN_NAME: %s\n", column_name);
    fputs(column_name, fchunk0);
    fputs("\n", fchunk0);
    free(column_name);
    fputs("</th>", fchunk0);
    fputs("\n", fchunk0);
  }
  fputs("</tr>", fchunk0);
  fclose(fchunk0);
  return 1;
}


// When tag <row> is empty.
void generate_cells(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "row") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    int len_row_number = strlen(worksheet_callbackdata->ROW_NUMBER);
    int len_index_sheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);

    for (int i = 1; i <= worksheet_callbackdata->end_col_number; i++) {
      //id: 0_B2
      char *col_name = int_to_column_name(i);
      int len_col_name = strlen(col_name);
      int len_id = len_index_sheet + len_col_name + len_row_number + 1;
      int len_td_str = 44 + len_id;
      char *td = malloc(len_td_str + 1);
      snprintf(
        td, len_td_str + 1,
        "<td id=\"%d_%s%s\" style=\"border-style:hidden;\"></td>",
        INDEX_CURRENT_SHEET, col_name,
        worksheet_callbackdata->ROW_NUMBER
      );
      fputs(td, worksheet_callbackdata->worksheet_file);
      fputs("\n", worksheet_callbackdata->worksheet_file);
      free(col_name);
      free(td);
      COUNT_CELLS++;
    }
    fputs("</tr>", worksheet_callbackdata->worksheet_file);
    fputs("\n", worksheet_callbackdata->worksheet_file);
    free(worksheet_callbackdata->ROW_NUMBER);
    worksheet_callbackdata->ROW_NUMBER = NULL;
  }
  XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
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
    INDEX_CURRENT_SHEET = worksheet_callbackdata->index_sheet;
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
          NUM_OF_CHUNKS += 1;
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
    CURRENT_CHUNK = 1;
    int LEN_CHUNKS_DIR_PATH = strlen(OUTPUT_DIR) + strlen(CHUNKS_DIR_NAME) + 1 + 1;
    char *CHUNKS_DIR_PATH = malloc(LEN_CHUNKS_DIR_PATH);
    snprintf(CHUNKS_DIR_PATH, LEN_CHUNKS_DIR_PATH, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);
    //12: chunk_%d_%d.html
    int len_chunk_file_path = LEN_CHUNKS_DIR_PATH + snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET) + snprintf(NULL, 0, "%d", CURRENT_CHUNK) + 12;
    char *CHUNK_FILE_PATH = malloc(len_chunk_file_path + 1);
    snprintf(CHUNK_FILE_PATH, len_chunk_file_path + 1, "%s/chunk_%d_%d.html", CHUNKS_DIR_PATH, INDEX_CURRENT_SHEET, CURRENT_CHUNK);
    free(CHUNKS_DIR_PATH);
    worksheet_callbackdata->worksheet_file = fopen(CHUNK_FILE_PATH, "wb+");
    free(CHUNK_FILE_PATH);
    XML_SetElementHandler(xmlparser, col_row_start_element, NULL);
  } else if (strcmp(name, "mergeCells") == 0) {
    worksheet_callbackdata->hasMergedCells = '1';
    int LEN_CHUNKS_DIR_PATH = strlen(OUTPUT_DIR) + strlen(CHUNKS_DIR_NAME) + 1 + 1;
    char *CHUNKS_DIR_PATH = malloc(LEN_CHUNKS_DIR_PATH);
    snprintf(CHUNKS_DIR_PATH, LEN_CHUNKS_DIR_PATH, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);

    int len_index_worksheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
    //1: /  14: chunk_%d_mc.json
    int len_json_file_path = LEN_CHUNKS_DIR_PATH + 1 + 14 + len_index_worksheet;
    char JSON_FILE_PATH[len_json_file_path + 1];
    snprintf(JSON_FILE_PATH, len_json_file_path + 1, "%s/chunk_%d_mc.json", CHUNKS_DIR_PATH, INDEX_CURRENT_SHEET);
    free(CHUNKS_DIR_PATH);
    worksheet_callbackdata->fmergecell = fopen(JSON_FILE_PATH, "wb");
    if (worksheet_callbackdata->fmergecell == NULL) {
      fprintf(stderr, "Can not open chunk_%d_mc.json for write", INDEX_CURRENT_SHEET);
      return;
    }
    fputs("{", worksheet_callbackdata->fmergecell);
    XML_SetElementHandler(xmlparser, col_row_start_element, NULL);
  } else if (strcmp(name, "drawing") == 0) {
    worksheet_callbackdata->array_drawingids.length++;
    worksheet_callbackdata->array_drawingids.drawing_ids = realloc(
      worksheet_callbackdata->array_drawingids.drawing_ids,
      worksheet_callbackdata->array_drawingids.length * sizeof(char *)
    );
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r:id") == 0) {
        worksheet_callbackdata->array_drawingids.drawing_ids[worksheet_callbackdata->array_drawingids.length - 1] = strdup(attrs[i + 1]);
      }
      printf("ID DRAWING: %s\n", attrs[i]);
    }
    XML_SetElementHandler(xmlparser, NULL, worksheet_end_element);
  }
}

void worksheet_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "sheetData") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    fclose(worksheet_callbackdata->worksheet_file);
  } else if (strcmp(name, "cols") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    int status = generate_columns(worksheet_callbackdata->array_cols, worksheet_callbackdata->end_col_number, INDEX_CURRENT_SHEET);
    if (status == -1) {
      fprintf(stderr, "Error when generated chunk0 of  Sheet\n");
      return;
    }
    for (int index_col = 0; index_col < worksheet_callbackdata->array_cols.length; index_col++) {
      free(worksheet_callbackdata->array_cols.cols[index_col]);
    }
    free(worksheet_callbackdata->array_cols.cols);

  } else if (strcmp(name, "mergeCells") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    fseek(worksheet_callbackdata->fmergecell, -1, SEEK_CUR);
    fputs("}", worksheet_callbackdata->fmergecell);
    fclose(worksheet_callbackdata->fmergecell);
  } else if (strcmp(name, "drawing") == 0) {

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
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    worksheet_callbackdata->ROW_NUMBER = NULL;
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r") == 0) {
	int len_row_number = strlen(attrs[i + 1]);
	worksheet_callbackdata->ROW_NUMBER = realloc(worksheet_callbackdata->ROW_NUMBER, len_row_number + 1);
	memcpy(worksheet_callbackdata->ROW_NUMBER, attrs[i + 1], len_row_number + 1);
	int LEN_TR_TAG = 11 + len_row_number;
        char TR_TAG[LEN_TR_TAG]; snprintf(TR_TAG, LEN_TR_TAG, "<tr id=\"%s\">", worksheet_callbackdata->ROW_NUMBER); fputs(TR_TAG, worksheet_callbackdata->worksheet_file);
        fputs("\n", worksheet_callbackdata->worksheet_file);
      } else if (strcmp(attrs[i], "ht") == 0) {
	//<th style="height:px;"
	float row_height_in_px = strtof((char *)attrs[i + 1], NULL) * (20 * 1.0 / 15);
	int len_row_height_in_px = snprintf(NULL, 0, "%.2f", row_height_in_px);
        int LEN_TH_TAG = 29 + len_row_height_in_px + strlen(worksheet_callbackdata->ROW_NUMBER);
	char TH_TAG[LEN_TH_TAG];
	snprintf(TH_TAG, LEN_TH_TAG, "<th style=\"height:%.2fpx;\">%s</th>", row_height_in_px, worksheet_callbackdata->ROW_NUMBER);
	fputs(TH_TAG, worksheet_callbackdata->worksheet_file);
        fputs("\n", worksheet_callbackdata->worksheet_file);
      }
    }
    START_CELL_IN_NUMBER_BY_ROW = 1;
    XML_SetElementHandler(xmlparser, cell_start_element, generate_cells);
  } else if (strcmp(name, "mergeCell") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "ref") == 0) {
	char **mergecell_range = malloc(2 * sizeof(char *));
	int count = 0;
	char *token, *_tmp_range, *tofree;
	tofree = _tmp_range = strdup(attrs[i + 1]);
	while ((token = strsep(&_tmp_range, ":"))) {
	  int len_token = strlen(token);
	  mergecell_range[count] = malloc(len_token + 1);
	  memcpy(mergecell_range[count], token, len_token + 1);
	  mergecell_range[count][len_token] = '\0';
	  count++;
	}
	printf("MERGE CELL RANGEEEEEEEEEE: %s | %s\n", mergecell_range[0], mergecell_range[1]);
        //You can't free _tmp_range because its value can be changed by calls to strsep().
	//The value of tofree consistently points to the start of the memory you want to free.
	free(tofree);
	size_t index_col_start = get_col_nr(mergecell_range[0]);
	size_t index_col_end = get_col_nr(mergecell_range[1]);
	size_t index_row_start = get_row_nr(mergecell_range[0]);
	size_t index_row_end = get_row_nr(mergecell_range[1]);
	unsigned short colspan = index_col_end - index_col_start + 1;
	unsigned short rowspan = index_row_end - index_row_start + 1;
	int len_start_cell = strlen(mergecell_range[0]);
	int len_colspan = snprintf(NULL, 0, "%d", colspan);
	int len_rowspan = snprintf(NULL, 0, "%d", rowspan);
	//30:  "":{colspan:"",rowspan:""}
	int len_mergecell_json_str = len_start_cell + len_colspan + len_rowspan + 30;
        char mergecell_json_str[len_mergecell_json_str + 1];
        snprintf(mergecell_json_str, len_mergecell_json_str + 1, "\"%s\":{\"colspan\":\"%d\",\"rowspan\":\"%d\"}", mergecell_range[0], colspan, rowspan);
	fputs(mergecell_json_str, worksheet_callbackdata->fmergecell);
        fputs(",", worksheet_callbackdata->fmergecell);
	free(mergecell_range[0]);
	free(mergecell_range[1]);
	free(mergecell_range);
      }
    }
    XML_SetElementHandler(xmlparser, NULL, col_row_end_element);
  }
}

void col_row_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "col") == 0) {
    XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
  } else if (strcmp(name, "row") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    int len_row_number = strlen(worksheet_callbackdata->ROW_NUMBER);
    int len_index_sheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
    while (START_CELL_IN_NUMBER_BY_ROW <= worksheet_callbackdata->end_col_number) {
      char *col_name = int_to_column_name(START_CELL_IN_NUMBER_BY_ROW);
      int len_col_name = strlen(col_name);
      int len_id = len_index_sheet + len_col_name + len_row_number + 1;
      int len_td_str = 44 + len_id;
      char *td = malloc(len_td_str + 1);
      snprintf(
        td, len_td_str + 1,
        "<td id=\"%d_%s%s\" style=\"border-style:hidden;\"></td>",
        INDEX_CURRENT_SHEET, col_name,
        worksheet_callbackdata->ROW_NUMBER
      );
      fputs(td, worksheet_callbackdata->worksheet_file);
      fputs("\n", worksheet_callbackdata->worksheet_file);
      free(col_name);
      free(td);
      START_CELL_IN_NUMBER_BY_ROW++;
      COUNT_CELLS++;
    }
    fputs("</tr>", worksheet_callbackdata->worksheet_file);
    fputs("\n", worksheet_callbackdata->worksheet_file);
    free(worksheet_callbackdata->ROW_NUMBER);
    worksheet_callbackdata->ROW_NUMBER = NULL;
    XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
  } else if (strcmp(name, "mergeCell") == 0) {
    XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
  }
}

void cell_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) { 
  (void)attrs;
  if (strcmp(name, "c") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    worksheet_callbackdata->cell_name = NULL;
    worksheet_callbackdata->type_content = NULL;
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r") == 0) {
	worksheet_callbackdata->cell_name = realloc(worksheet_callbackdata->cell_name, 1 + strlen(attrs[i + 1]));
        memcpy(worksheet_callbackdata->cell_name, attrs[i + 1], 1 + strlen(attrs[i + 1]));
        CURRENT_CELL_IN_NUMBER_BY_ROW = (unsigned int)get_col_nr(attrs[i + 1]);
      } else if (strcmp(attrs[i], "s") == 0) {
        worksheet_callbackdata->index_style = (int)strtol(attrs[i + 1], NULL, 10);
      } else if (strcmp(attrs[i], "t") == 0) {
        worksheet_callbackdata->type_content = realloc(worksheet_callbackdata->type_content, 1 + strlen(attrs[i + 1]));
	memcpy(worksheet_callbackdata->type_content, attrs[i + 1], 1 + strlen(attrs[i + 1]));
      }
    }

    int len_row_number = strlen(worksheet_callbackdata->ROW_NUMBER);
    int len_index_sheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
    while (START_CELL_IN_NUMBER_BY_ROW < CURRENT_CELL_IN_NUMBER_BY_ROW) {
      char *col_name = int_to_column_name(START_CELL_IN_NUMBER_BY_ROW);
      int len_col_name = strlen(col_name);
      int len_id = len_index_sheet + len_col_name + len_row_number + 1;
      int len_td_str = 44 + len_id;
      char *td = malloc(len_td_str + 1);
      snprintf(
        td, len_td_str + 1,
        "<td id=\"%d_%s%s\" style=\"border-style:hidden;\"></td>",
        INDEX_CURRENT_SHEET, col_name,
        worksheet_callbackdata->ROW_NUMBER
      );
      fputs(td, worksheet_callbackdata->worksheet_file);
      fputs("\n", worksheet_callbackdata->worksheet_file);
      free(col_name);
      free(td);

      START_CELL_IN_NUMBER_BY_ROW++;
      COUNT_CELLS++;
    }
    char *styles = NULL;
    char *horizontal_style = NULL;
    char *vertical_style = NULL;
    char *border_style = NULL;
    char *wraptext_style = NULL;
    char *horizontal = NULL;
    char *vertical = NULL;
    char *font_style = NULL;
    char *fill_style = NULL;
    char wrapText;
    if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyAlignment == '1') {
      int len_horizontal = strlen(array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.horizontal);
      horizontal = realloc(horizontal, 1 + len_horizontal);
      memcpy(horizontal, array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.horizontal, 1 + len_horizontal);
      int len_vertical = strlen(array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.vertical);
      vertical = realloc(vertical, 1 + len_vertical);
      memcpy(vertical, array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.vertical, 1 + len_vertical);
      wrapText = array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.isWrapText;
    } else if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyAlignment == '0') {
      int id_cellXfs = array_cellXfs.Xfs[worksheet_callbackdata->index_style].xfId;
      int len_horizontal = strlen(array_cellStyleXfs.Xfs[id_cellXfs].alignment.horizontal);
      horizontal = realloc(horizontal, 1 + len_horizontal);
      memcpy(horizontal, array_cellStyleXfs.Xfs[id_cellXfs].alignment.horizontal, 1 + len_horizontal);
      int len_vertical = strlen(array_cellStyleXfs.Xfs[id_cellXfs].alignment.vertical);
      vertical = realloc(vertical, 1 + len_vertical);
      memcpy(vertical, array_cellStyleXfs.Xfs[id_cellXfs].alignment.vertical, 1 + len_vertical);
      wrapText = array_cellStyleXfs.Xfs[id_cellXfs].alignment.isWrapText;
    }
    if (horizontal != NULL) {
      if (strcmp(horizontal, "center") == 0) {
        //18: text-align:center;
        horizontal_style = realloc(horizontal_style, 18 + 1);
        snprintf(horizontal_style, 18 + 1, "text-align:center;");
      } else if (strcmp(horizontal, "general") == 0) {
        //16: text-align:left;
        horizontal_style = realloc(horizontal_style, 16 + 1);
        snprintf(horizontal_style, 16 + 1, "text-align:left;");
      } else {
        //12: text-align:;
        horizontal_style = realloc(horizontal_style, strlen(horizontal) + 12 + 1);
        snprintf(horizontal_style, strlen(horizontal) + 12 + 1, "text-align:%s;", horizontal);
      }
      free(horizontal);
    }
    if (vertical != NULL) {
      if (strcmp(vertical, "center") == 0) {
        //22: vertical-align:middle;
        vertical_style = realloc(vertical_style, 22 + 1);
        snprintf(vertical_style, 22 + 1, "vertical-align:middle;");
      } else {
        //16: vertical-align:;
        vertical_style = realloc(vertical_style, strlen(vertical) + 16 + 1);
        snprintf(vertical_style, strlen(vertical) + 16 + 1, "vertical-align:%s;", vertical);
      }
      free(vertical);
    }
    if (wrapText == '1') {
      //19: white-space:normal;
      wraptext_style = realloc(wraptext_style, 19 + 1);
      snprintf(wraptext_style, 19 + 1, "white-space:normal;");
    } else {
      //19: white-space:nowrap;
      wraptext_style = realloc(wraptext_style, 19 + 1);
      snprintf(wraptext_style, 19 + 1, "white-space:nowrap;");
    }
    char *border_left = NULL;
    char *border_right = NULL;
    char *border_top = NULL;
    char *border_bottom = NULL;
    int border_id = -1;
    if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyBorder == '1') {
      border_id = array_cellXfs.Xfs[worksheet_callbackdata->index_style].borderId;
    } else if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyBorder == '0') {
      int id_cellXfs = array_cellXfs.Xfs[worksheet_callbackdata->index_style].xfId;
      border_id = array_cellStyleXfs.Xfs[id_cellXfs].borderId;
    }
    if (border_id != -1) {
      if (array_borders.borders[border_id].left.style != NULL) {
        //25: border-left-style:solid;
        border_left = realloc(border_left, 25 + 1);
        snprintf(border_left, 25 + 1, "border-left-style:solid;");
        //TODO: If border_left == 'medium', then set border_width to 2px.
      } else {
        border_left = realloc(border_left, 26 + 1);
        snprintf(border_left, 26 + 1, "border-left-style:hidden;");
      }
      if (array_borders.borders[border_id].right.style != NULL) {
        //26: border-right-style:solid;
        border_right = realloc(border_right, 26 + 1);
        snprintf(border_right, 26 + 1, "border-right-style:solid;");
        //TODO: If border_right == 'medium', then set border_width to 2px.
      } else {
        border_right = realloc(border_right, 27 + 1);
        snprintf(border_right, 27 + 1, "border-right-style:hidden;");
      }
      if (array_borders.borders[border_id].top.style != NULL) {
        //24: border-top-style:solid;
        border_top = realloc(border_top, 24 + 1);
        snprintf(border_top, 24 + 1, "border-top-style:solid;");
        //TODO: If border_top == 'medium', then set border_width to 2px.
      } else {
        border_top = realloc(border_top, 25 + 1);
        snprintf(border_top, 25 + 1, "border-top-style:hidden;");
      }
      if (array_borders.borders[border_id].bottom.style != NULL) {
        //27: border-bottom-style:solid;
        border_bottom = realloc(border_bottom, 27 + 1);
        snprintf(border_bottom, 27 + 1, "border-bottom-style:solid;");
        //TODO: If border_bottom == 'medium', then set border_width to 2px.
      } else {
        border_bottom = realloc(border_bottom, 28 + 1);
        snprintf(border_bottom, 28 + 1, "border-bottom-style:hidden;");
      }
    }
    int len_borders = strlen(border_left) + strlen(border_right) + strlen(border_top) + strlen(border_bottom);
    border_style = realloc(border_style, len_borders + 1);
    snprintf(border_style, len_borders + 1, "%s%s%s%s", border_left, border_right, border_top, border_bottom);
    free(border_left);
    free(border_right);
    free(border_top);
    free(border_bottom);

    unsigned short font_id = -1;
    if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyFont == '1') {
      font_id = array_cellXfs.Xfs[worksheet_callbackdata->index_style].fontId;
    } else if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyFont == '0') {
      int id_cellXfs = array_cellXfs.Xfs[worksheet_callbackdata->index_style].xfId;
      font_id = array_cellStyleXfs.Xfs[id_cellXfs].fontId;
    }
    if (font_id != -1 && array_fonts.fonts[font_id].name != NULL) {
      //12: "font-family:" | 1: ';'
      const int LEN_FONT_NAME = 14 + strlen(array_fonts.fonts[font_id].name); //ex
      char font_name[LEN_FONT_NAME];
      snprintf(font_name, LEN_FONT_NAME, "font-family:%s;", array_fonts.fonts[font_id].name);
      font_style = strdup(font_name);
      if (array_fonts.fonts[font_id].sz != 0.0) {
        // 13: "font-size:px;" + 1: '\0' = 14
        const int LEN_FONT_SIZE = snprintf(NULL, 0, "%.2f", array_fonts.fonts[font_id].sz) + 14;
        char font_size[LEN_FONT_SIZE];
        snprintf(font_size, LEN_FONT_SIZE, "font-size:%.2fpx;", array_fonts.fonts[font_id].sz);
        char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, font_size);
	free(tmp_font_style);
      }
      if (array_fonts.fonts[font_id].isBold != '0') {
        char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, "font-weight:bold;");
	free(tmp_font_style);
      }
      if (array_fonts.fonts[font_id].isItalic != '0') {
	char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, "font-style:italic;");
	free(tmp_font_style);
      }
      if (array_fonts.fonts[font_id].color.rgb != NULL) {
	const int LEN_FONT_COLOR_RGB = 8 + strlen(array_fonts.fonts[font_id].color.rgb);
        char font_color_rgb[LEN_FONT_COLOR_RGB];
        // 6: "color:" | 1: ';'
        snprintf(font_color_rgb, LEN_FONT_COLOR_RGB, "color:%s;", array_fonts.fonts[font_id].color.rgb);
	char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, font_color_rgb);
	free(tmp_font_style);
      }
      if (array_fonts.fonts[font_id].underline != NULL && strcmp(array_fonts.fonts[font_id].underline, "none") != 0) {
	const int LEN_FONT_TEXT_DECORATION_LINE = 32;
        char font_text_decoration_line[LEN_FONT_TEXT_DECORATION_LINE];
	memcpy(font_text_decoration_line, "text-decoration-line:underline;", LEN_FONT_TEXT_DECORATION_LINE);
	char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, font_text_decoration_line);
	free(tmp_font_style);
        if (strcmp(array_fonts.fonts[font_id].underline, "single") == 0) {
	  const int LEN_FONT_TEXT_DECORATION_STYLE = 30;
          char font_text_decoration_style[LEN_FONT_TEXT_DECORATION_STYLE];
	  memcpy(font_text_decoration_style, "text-decoration-style:single;", LEN_FONT_TEXT_DECORATION_STYLE);
	  char *tmp_font_style = strdup(font_style);
	  free(font_style);
	  font_style = concat(tmp_font_style, font_text_decoration_style);
	  free(tmp_font_style);
        } else if(strcmp(array_fonts.fonts[font_id].underline, "double") == 0) {
	  const int LEN_FONT_TEXT_DECORATION_STYLE = 30;
          char font_text_decoration_style[LEN_FONT_TEXT_DECORATION_STYLE];
	  memcpy(font_text_decoration_style, "text-decoration-style:double;", LEN_FONT_TEXT_DECORATION_STYLE);
	  char *tmp_font_style = strdup(font_style);
	  free(font_style);
	  font_style = concat(tmp_font_style, font_text_decoration_style);
	  free(tmp_font_style);
        }
      }
      unsigned short fill_id = array_cellXfs.Xfs[worksheet_callbackdata->index_style].fillId;
      int LEN_FILL_FGCOLOR_RGB = 0;
      if (array_fills.fills[fill_id].patternFill.fgColor.rgb != NULL) {
        // 18: "background-color:;"
	LEN_FILL_FGCOLOR_RGB = 18 + strlen(array_fills.fills[fill_id].patternFill.fgColor.rgb);
	fill_style = realloc(fill_style, LEN_FILL_FGCOLOR_RGB + 1);
        snprintf(fill_style, LEN_FILL_FGCOLOR_RGB + 1, "background-color:%s;", array_fills.fills[fill_id].patternFill.fgColor.rgb);
      }
      int len_styles = strlen(horizontal_style) + strlen(vertical_style) + strlen(wraptext_style) + strlen(border_style) + strlen(font_style) + LEN_FILL_FGCOLOR_RGB;
      styles = realloc(styles, len_styles + 1);
      if (LEN_FILL_FGCOLOR_RGB == 0) {
        snprintf(styles, len_styles + 1, "%s%s%s%s%s", horizontal_style, vertical_style, wraptext_style, border_style, font_style);
      } else {
        snprintf(styles, len_styles + 1, "%s%s%s%s%s%s", horizontal_style, vertical_style, wraptext_style, border_style, font_style, fill_style);
        free(fill_style);
      }
      free(horizontal_style);
      free(vertical_style);
      free(wraptext_style);
      free(border_style);
      free(font_style);
      char *TD_TAG = NULL;
      int len_cellname = strlen(worksheet_callbackdata->cell_name);
      int len_index_sheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
      //24: <td id="" style=""></td>
      //1: _
      int len_td_tag = 24 + len_styles + len_cellname + len_index_sheet + 1;
      TD_TAG = realloc(TD_TAG, len_td_tag + 1);
      snprintf(TD_TAG, len_td_tag + 1, "<td id=\"%d_%s\" style=\"%s\">", INDEX_CURRENT_SHEET, worksheet_callbackdata->cell_name, styles);
      fputs(TD_TAG, worksheet_callbackdata->worksheet_file);
      fputs("\n", worksheet_callbackdata->worksheet_file);
      START_CELL_IN_NUMBER_BY_ROW++;
      COUNT_CELLS++;
      free(styles);
      free(TD_TAG);
      XML_SetElementHandler(xmlparser, cell_item_start_element, cell_end_element);
    }
  }
}

void cell_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "c") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    fputs("</td>", worksheet_callbackdata->worksheet_file);
    fputs("\n", worksheet_callbackdata->worksheet_file);
    free(worksheet_callbackdata->cell_name);
    worksheet_callbackdata->cell_name = NULL;
    if (worksheet_callbackdata->type_content != NULL) {
      free(worksheet_callbackdata->type_content);
      worksheet_callbackdata->type_content = NULL;
    }
    XML_SetElementHandler(xmlparser, cell_start_element, col_row_end_element);
  }
}

void cell_item_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  if (strcmp(name, "v") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    worksheet_callbackdata->worksheet_content = NULL;
    worksheet_callbackdata->len_worksheet_content = 0;
    XML_SetElementHandler(xmlparser, NULL, cell_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, worksheet_content_handler);
  } else if (strcmp(name, "f") == 0) {
    XML_SetElementHandler(xmlparser, NULL, cell_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void cell_item_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "v") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    if (worksheet_callbackdata->worksheet_content != NULL) {
      if (strcmp(worksheet_callbackdata->type_content, "s") == 0) {
        int len_sharedStrings_html_file_name = strlen(OUTPUT_FILE_NAME) + strlen(SHAREDSTRINGS_HTML_FILE_SUFFIX);
        char *SHAREDSTRINGS_HTML_FILE_NAME = malloc(len_sharedStrings_html_file_name + 1);
        snprintf(SHAREDSTRINGS_HTML_FILE_NAME, len_sharedStrings_html_file_name + 1, "%s%s", OUTPUT_FILE_NAME, SHAREDSTRINGS_HTML_FILE_SUFFIX);
        int len_sharedStrings_file_path = strlen(TEMP_DIR) + 1 + len_sharedStrings_html_file_name;
        char *SHAREDSTRINGS_HTML_FILE_PATH = malloc(len_sharedStrings_file_path + 1);
        snprintf(SHAREDSTRINGS_HTML_FILE_PATH, len_sharedStrings_file_path + 1, "%s/%s", TEMP_DIR, SHAREDSTRINGS_HTML_FILE_NAME);
	free(SHAREDSTRINGS_HTML_FILE_NAME);
        FILE *sharedStrings_file = fopen(SHAREDSTRINGS_HTML_FILE_PATH, "rb");
        if (sharedStrings_file == NULL) {
          fprintf(stderr, "Cannot open %s file to read\n", SHAREDSTRINGS_HTML_FILE_PATH);
	  free(SHAREDSTRINGS_HTML_FILE_PATH);
          return;
        }
	free(SHAREDSTRINGS_HTML_FILE_PATH);
        int len_pos_arr = sharedStrings_position.length;
        int index_sharedStrings_current = (int)strtol(worksheet_callbackdata->worksheet_content, NULL, 10);
        unsigned long start_pos = sharedStrings_position.positions[index_sharedStrings_current];
        unsigned long end_pos = -1;
        if (index_sharedStrings_current + 1 <= len_pos_arr) {
          end_pos = sharedStrings_position.positions[index_sharedStrings_current + 1];
        }
        fseek(sharedStrings_file, start_pos, SEEK_SET);
        char c;
        while((c = fgetc(sharedStrings_file)) != EOF) {
          fputc(c, worksheet_callbackdata->worksheet_file);
          fflush(sharedStrings_file);
          if (end_pos != -1 && ftell(sharedStrings_file) == end_pos) {
	    break;
          }
        }
        fclose(sharedStrings_file);
      } else if (strcmp(worksheet_callbackdata->type_content, "n") == 0) {
        unsigned short numFmt_id = -1;
	if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyNumberFormat == '1') {
	  numFmt_id = array_cellXfs.Xfs[worksheet_callbackdata->index_style].numFmtId;
	} else if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyNumberFormat  == '0') {
	  int id_cellXfs = array_cellXfs.Xfs[worksheet_callbackdata->index_style].xfId;
	  numFmt_id = array_cellStyleXfs.Xfs[id_cellXfs].numFmtId;
	}
	int index_numFmt;
	for (index_numFmt = 0; index_numFmt < array_numfmts.length; index_numFmt++) {
	  int numFmtIdInt = strtol(array_numfmts.numfmts[index_numFmt].numFmtId, NULL, 10);
	  if (numFmtIdInt == numFmt_id) {
	    break;
	  }
	}
        fputs("<span>", worksheet_callbackdata->worksheet_file);
	if (strcmp(array_numfmts.numfmts[index_numFmt].formatCode, "General") == 0) {
	  fputs(worksheet_callbackdata->worksheet_content, worksheet_callbackdata->worksheet_file);
	} else {
	  FILE *fp;
	  char formated_content[1035];
	  /* Open the command for reading. */
	  int len_ssf_bin_path = strlen(WORKING_DIR) + strlen(THIRD_PARTY_DIR_NAME) + strlen(SSF_BIN_DIR_NAME) + 3;
	  char *ssf_bin_path = malloc(len_ssf_bin_path + 1);
	  snprintf(ssf_bin_path, len_ssf_bin_path + 1, "%s%s/%s", WORKING_DIR, THIRD_PARTY_DIR_NAME, SSF_BIN_DIR_NAME);
	  char *option_format = "--format";

	  //3: blank
	  //2: ""
	  int len_cmd = len_ssf_bin_path
	    + strlen(worksheet_callbackdata->worksheet_content)
	    + strlen(array_numfmts.numfmts[index_numFmt].formatCode)
	    + strlen(option_format) + 3 + 2;
	  char *cmd = malloc(len_cmd + 1);
	  snprintf(
	    cmd, len_cmd + 1, "%s %s \"%s\" %s", ssf_bin_path, option_format,
	    array_numfmts.numfmts[index_numFmt].formatCode,
	    worksheet_callbackdata->worksheet_content
	  );
	  free(ssf_bin_path);
	  fp = popen(cmd, "r");
	  free(cmd);
	  if (fp == NULL) {
	    fprintf(stderr, "Failed to run ssf\n");
	    fputs(worksheet_callbackdata->worksheet_content, worksheet_callbackdata->worksheet_file);
	  } else {
	    /* Read the output a line at a time - output it. */
	    fgets(formated_content, sizeof(formated_content), fp);
	    /* close */
	    pclose(fp);
	    fputs(formated_content, worksheet_callbackdata->worksheet_file);
	  }
	}
        fputs("</span>", worksheet_callbackdata->worksheet_file);
        fputs("\n", worksheet_callbackdata->worksheet_file);
      } else {
        fputs("<span>", worksheet_callbackdata->worksheet_file);
        fputs(worksheet_callbackdata->worksheet_content, worksheet_callbackdata->worksheet_file);
        fputs("</span>", worksheet_callbackdata->worksheet_file);
        fputs("\n", worksheet_callbackdata->worksheet_file);
      }
      free(worksheet_callbackdata->worksheet_content);
      worksheet_callbackdata->worksheet_content = NULL;
      worksheet_callbackdata->len_worksheet_content = 0;
    }
    XML_SetElementHandler(xmlparser, NULL, cell_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (strcmp(name, "f") == 0) {
    XML_SetElementHandler(xmlparser, cell_item_start_element, cell_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void worksheet_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  //The character data handler may be called multiple times with partial character data for a single element. 
  struct WorkSheet *worksheet_callbackdata = callbackdata;
  if (worksheet_callbackdata->worksheet_content == NULL && worksheet_callbackdata->len_worksheet_content == 0) {
    if ((worksheet_callbackdata->worksheet_content = realloc(worksheet_callbackdata->worksheet_content, len + 1)) == NULL) {
      return;
    }
    memcpy(worksheet_callbackdata->worksheet_content, buf, len);
    worksheet_callbackdata->worksheet_content[len] = '\0';
    worksheet_callbackdata->len_worksheet_content += len + 1;
  } else {
    int len_worksheet_content = worksheet_callbackdata->len_worksheet_content;
    if ((worksheet_callbackdata->worksheet_content = realloc(worksheet_callbackdata->worksheet_content, len_worksheet_content + len)) == NULL) {
      return;
    }
    memcpy(worksheet_callbackdata->worksheet_content + len_worksheet_content - 1, buf, len);
    worksheet_callbackdata->worksheet_content[len_worksheet_content + len - 1] = '\0';
    worksheet_callbackdata->len_worksheet_content += len;
  }
}
