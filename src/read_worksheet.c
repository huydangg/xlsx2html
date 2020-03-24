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
unsigned int CELLS_REMAIN;


char *int_to_column_name(int n) {
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
  for (i = 0, j = strlen(col_name) - 1; i < strlen(col_name); i += 1, j -= 1) { const char *ptr = strchr(base, col_name[i]); if (!ptr) {
      return -1;
    }
    int index = ptr - base;
    result += (int)pow((double)strlen(base), (double)j) * (index + 1);
  }
  return result;
}

int generate_columns(struct ArrayCols array_cols, unsigned short end_col_number, unsigned short index_worksheet) {
  const char *OUTPUT_ROOT_DIR = "/media/huydang/HuyDang/xlsxmagic/output";
  const char *CHUNKS_DIR_NAME = "chunks";
  int LEN_THE_FIRST_CHUNK_DIR = strlen(OUTPUT_ROOT_DIR) + strlen(CHUNKS_DIR_NAME) + 1 + 1;
  char *THE_FIRST_CHUNK_DIR = malloc(LEN_THE_FIRST_CHUNK_DIR);
  snprintf(THE_FIRST_CHUNK_DIR, LEN_THE_FIRST_CHUNK_DIR, "%s/%s", OUTPUT_ROOT_DIR, CHUNKS_DIR_NAME);
  struct stat st = {0};
  printf("%s\n", THE_FIRST_CHUNK_DIR);
  if (stat(THE_FIRST_CHUNK_DIR, &st) == -1) {
    int status = mkdir(THE_FIRST_CHUNK_DIR, 0777);
    if (status != 0) {
      fprintf(stderr, "Error when create a chunk dir with status is %d\n", status);
      free(THE_FIRST_CHUNK_DIR);
      return -1; 
    }
  }
  int len_index_worksheet = snprintf(NULL, 0, "%d", index_worksheet);
  int LEN_THE_FIRST_CHUNK_PATH = LEN_THE_FIRST_CHUNK_DIR + len_index_worksheet + 14;
  char *THE_FIRST_CHUNK_PATH = malloc(LEN_THE_FIRST_CHUNK_PATH);
  snprintf(THE_FIRST_CHUNK_PATH, LEN_THE_FIRST_CHUNK_PATH, "%s/chunk_%d_0.html", THE_FIRST_CHUNK_DIR, index_worksheet);
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
  free(THE_FIRST_CHUNK_DIR);
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
    char *column_name = int_to_column_name(i);
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

struct SheetData {
  unsigned short current_cell_remain;
  FILE *worksheet_file;
  unsigned short current_sheet;
  unsigned short end_col_number;
  char *cell_name;
  char *type_content;
  int index_style;
};

// When tag <row> is empty.
void generate_cells(void *callbackdata, const XML_Char *name) {
  struct SheetData *sheetData_callbackdata = callbackdata;
  for (int i = 1; i <= sheetData_callbackdata->end_col_number; i++) {
    fputs("<td></td>", sheetData_callbackdata->worksheet_file);
    fputs("\n", sheetData_callbackdata->worksheet_file);
    COUNT_CELLS++;
  }
  fputs("</tr>", sheetData_callbackdata->worksheet_file);
  fputs("\n", sheetData_callbackdata->worksheet_file);
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
    struct SheetData *sheetData_callbackdata = malloc(sizeof(struct SheetData));
    sheetData_callbackdata->current_sheet = worksheet_callbackdata->index_sheet;
    sheetData_callbackdata->end_col_number = worksheet_callbackdata->end_col_number;
    char CHUNK_PATH[256];
    snprintf(CHUNK_PATH, sizeof(CHUNK_PATH), "/media/huydang/HuyDang/xlsxmagic/output/chunks/chunk_%d_%d.html", sheetData_callbackdata->current_sheet, CURRENT_CHUNK);
    sheetData_callbackdata->worksheet_file = fopen(CHUNK_PATH, "wb+");
    XML_SetUserData(xmlparser, sheetData_callbackdata);
    XML_SetElementHandler(xmlparser, col_row_start_element, NULL);
  }
}

void worksheet_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "sheetData") == 0) {
    struct SheetData *sheetData_callbackdata = callbackdata;
    fclose(sheetData_callbackdata->worksheet_file);
    free(sheetData_callbackdata);
  } else if (strcmp(name, "cols") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    int status = generate_columns(worksheet_callbackdata->array_cols, worksheet_callbackdata->end_col_number, worksheet_callbackdata->index_sheet);
    if (status == -1) {
      fprintf(stderr, "Error when generated chunk0 of  Sheet\n");
      return;
    }
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
    char *ROW_NUMBER = NULL;
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r") == 0) {
	int len_row_number = strlen(attrs[i + 1]);
	ROW_NUMBER = realloc(ROW_NUMBER, len_row_number + 1);
	memcpy(ROW_NUMBER, attrs[i + 1], len_row_number + 1);
	int LEN_TR_TAG = 11 + len_row_number;
        char TR_TAG[LEN_TR_TAG];
        snprintf(TR_TAG, LEN_TR_TAG, "<tr id=\"%s\">", ROW_NUMBER);
        fputs(TR_TAG, sheetData_callbackdata->worksheet_file);
        fputs("\n", sheetData_callbackdata->worksheet_file);  
      } else if (strcmp(attrs[i], "ht") == 0) {
	//<th style="height:px;"
	float row_height_in_px = strtof((char *)attrs[i + 1], NULL) * (20 * 1.0 / 15);
	int len_row_height_in_px = snprintf(NULL, 0, "%.2f", row_height_in_px);
        int LEN_TH_TAG = 29 + len_row_height_in_px + strlen(ROW_NUMBER);
	char TH_TAG[LEN_TH_TAG];
	snprintf(TH_TAG, LEN_TH_TAG, "<th style=\"height:%gpx;\">%s</th>", row_height_in_px, ROW_NUMBER);
	fputs(TH_TAG, sheetData_callbackdata->worksheet_file);
        fputs("\n", sheetData_callbackdata->worksheet_file);  
      }
    }
    START_CELL_IN_NUMBER_BY_ROW = 1;
    free(ROW_NUMBER);
    XML_SetElementHandler(xmlparser, cell_start_element, generate_cells);
  }
}

void col_row_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "col") == 0) {
    XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
  } else if (strcmp(name, "row") == 0) {
    struct SheetData *sheetData_callbackdata = callbackdata;
    while (CURRENT_CELL_IN_NUMBER_BY_ROW < sheetData_callbackdata->end_col_number) {
      fputs("<td></td>", sheetData_callbackdata->worksheet_file);
      fputs("\n", sheetData_callbackdata->worksheet_file);
      CURRENT_CELL_IN_NUMBER_BY_ROW++;
      COUNT_CELLS++;
    }
    fputs("</tr>", sheetData_callbackdata->worksheet_file);
    fputs("\n", sheetData_callbackdata->worksheet_file);  
    XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
  }
}

void cell_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) { 
  (void)attrs;
  if (strcmp(name, "c") == 0) {
    struct SheetData *sheetData_callbackdata = callbackdata;
    sheetData_callbackdata->cell_name = NULL;
    sheetData_callbackdata->type_content = NULL;
    for (int i = 0; attrs[i]; i+=2) {
      if (strcmp(attrs[i], "r") == 0) {
	sheetData_callbackdata->cell_name = realloc(sheetData_callbackdata->cell_name, 1 + strlen(attrs[i + 1]));
        memcpy(sheetData_callbackdata->cell_name, attrs[i + 1], 1 + strlen(attrs[i + 1]));
        CURRENT_CELL_IN_NUMBER_BY_ROW = (unsigned int)get_col_nr(attrs[i + 1]);
      } else if (strcmp(attrs[i], "s") == 0) {
        sheetData_callbackdata->index_style = (int)strtol(attrs[i + 1], NULL, 10);
      } else if (strcmp(attrs[i], "t") == 0) {
        sheetData_callbackdata->type_content = realloc(sheetData_callbackdata->type_content, 1 + strlen(attrs[i + 1]));
	memcpy(sheetData_callbackdata->type_content, attrs[i + 1], 1 + strlen(attrs[i + 1]));
      }
    }
    while (START_CELL_IN_NUMBER_BY_ROW < CURRENT_CELL_IN_NUMBER_BY_ROW) {
      fputs("<td></td>", sheetData_callbackdata->worksheet_file);
      fputs("\n", sheetData_callbackdata->worksheet_file);
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
    char wrapText;
    if (array_cellXfs.Xfs[sheetData_callbackdata->index_style].isApplyAlignment == '1') {
      int len_horizontal = strlen(array_cellXfs.Xfs[sheetData_callbackdata->index_style].alignment.horizontal);
      horizontal = realloc(horizontal, 1 + len_horizontal);
      memcpy(horizontal, array_cellXfs.Xfs[sheetData_callbackdata->index_style].alignment.horizontal, 1 + len_horizontal);
      int len_vertical = strlen(array_cellXfs.Xfs[sheetData_callbackdata->index_style].alignment.vertical);
      vertical = realloc(vertical, 1 + len_vertical);
      memcpy(vertical, array_cellXfs.Xfs[sheetData_callbackdata->index_style].alignment.vertical, 1 + len_vertical);
      wrapText = array_cellXfs.Xfs[sheetData_callbackdata->index_style].alignment.isWrapText;
    } else if (array_cellXfs.Xfs[sheetData_callbackdata->index_style].isApplyAlignment == '0') {
      int id_cellXfs = array_cellXfs.Xfs[sheetData_callbackdata->index_style].xfId;
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
    if (array_cellXfs.Xfs[sheetData_callbackdata->index_style].isApplyBorder == '1') {
      border_id = array_cellXfs.Xfs[sheetData_callbackdata->index_style].borderId;
    } else if (array_cellXfs.Xfs[sheetData_callbackdata->index_style].isApplyBorder == '0') {
      int id_cellXfs = array_cellXfs.Xfs[sheetData_callbackdata->index_style].xfId;
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
    int len_styles = strlen(horizontal_style) + strlen(vertical_style) + strlen(wraptext_style) + strlen(border_style);
    styles = realloc(styles, len_styles + 1);
    snprintf(styles, len_styles + 1, "%s%s%s%s", horizontal_style, vertical_style, wraptext_style, border_style);
    free(horizontal_style);
    free(vertical_style);
    free(wraptext_style);
    free(border_style);
    char *TD_TAG = NULL;
    //24: <td id="" style=""></td>
    int len_cellname = strlen(sheetData_callbackdata->cell_name);
    TD_TAG = realloc(TD_TAG, 24 + len_styles + len_cellname + 1);
    snprintf(TD_TAG, 24 + len_styles + len_cellname + 1, "<td id=\"%s\" style=\"%s\">", sheetData_callbackdata->cell_name, styles);
    fputs(TD_TAG, sheetData_callbackdata->worksheet_file);
    fputs("\n", sheetData_callbackdata->worksheet_file);
    START_CELL_IN_NUMBER_BY_ROW++;
    COUNT_CELLS++;
    free(styles);
    free(TD_TAG);
    XML_SetElementHandler(xmlparser, cell_item_start_element, cell_end_element);
  }
}

void cell_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "c") == 0) {
    struct SheetData *sheetData_callbackdata = callbackdata;
    fputs("</td>", sheetData_callbackdata->worksheet_file);
    fputs("\n", sheetData_callbackdata->worksheet_file);
    free(sheetData_callbackdata->cell_name);
    sheetData_callbackdata->cell_name = NULL;
    if (sheetData_callbackdata->type_content != NULL) {
      free(sheetData_callbackdata->type_content);
      sheetData_callbackdata->type_content = NULL;
    }
    XML_SetElementHandler(xmlparser, cell_start_element, col_row_end_element);
  }
}

void cell_item_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  if (strcmp(name, "v") == 0) {
    XML_SetElementHandler(xmlparser, NULL, cell_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, worksheet_content_handler);
  } else if (strcmp(name, "f") == 0) {
    XML_SetElementHandler(xmlparser, NULL, cell_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void cell_item_end_element(void *callbackdata, const XML_Char *name) {
  if (strcmp(name, "v") == 0) {
    XML_SetElementHandler(xmlparser, NULL, cell_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (strcmp(name, "f") == 0) {
    XML_SetElementHandler(xmlparser, NULL, cell_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void worksheet_content_handler(void *callbackdata, const XML_Char *buf, int len) {
  if (len == 0){
    return;
  }
  XML_Char *value;
  if ((value = malloc(len + 1)) == NULL) {
    return;
  }
  memcpy(value, buf, len);
  value[len] = '\0';

  struct SheetData *sheetData_callbackdata = callbackdata;
  if (strcmp(sheetData_callbackdata->type_content, "s") == 0) {
    FILE *sharedStrings_file = fopen("/media/huydang/HuyDang/xlsxmagic/output/sharedStrings.html", "rb");
    if (sharedStrings_file == NULL) {
      fprintf(stderr, "Cannot open sharedStrings.html file to read");
      return;
    }
    int len_pos_arr = sharedStrings_position.length;
    int index_sharedStrings_current = (int)strtol(value, NULL, 10);
    unsigned long start_pos = sharedStrings_position.positions[index_sharedStrings_current];
    unsigned long end_pos = -1;
    if (index_sharedStrings_current + 1 <= len_pos_arr) {
      end_pos = sharedStrings_position.positions[index_sharedStrings_current + 1];
    }
    fseek(sharedStrings_file, start_pos, SEEK_SET);
    char c;
    while((c = fgetc(sharedStrings_file)) != EOF) {
      fputc(c, sheetData_callbackdata->worksheet_file);
      if (end_pos != -1 && ftell(sharedStrings_file) == end_pos) {
	break;
      }
    }
    fclose(sharedStrings_file);
  } else {
    fputs("<p>", sheetData_callbackdata->worksheet_file);
    fputs(value, sheetData_callbackdata->worksheet_file);
    fputs("</p>", sheetData_callbackdata->worksheet_file);
    fputs("\n", sheetData_callbackdata->worksheet_file);
  }
  free(value);
}
