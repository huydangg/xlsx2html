#include <private.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sheet.h>
#include <const.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <style.h>
#include <sharedstring.h>
#include <errno.h>


unsigned int START_CELL_IN_NUMBER_BY_ROW; //default is 1
unsigned int CURRENT_CELL_IN_NUMBER_BY_ROW;
unsigned short INDEX_CURRENT_SHEET;
unsigned long CURRENT_SIZE_IN_CHUNK = 0;


void reversed(char *input) {
  int length = XML_Char_len(input);
  int last_pos = length - 1;
  for (int i = 0; i < length/2; i++) {
    char tmp = input[i];
    input[i] = input[last_pos - i];
    input[last_pos - i] = tmp;
  }
}

char *int_to_column_name(unsigned int n) {
  char *column_name = XML_Char_malloc(4);
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
  column_name[XML_Char_len(column_name)] = '\0';
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
  for (i = 0, j = XML_Char_len(col_name) - 1; i < XML_Char_len(col_name); i += 1, j -= 1) {
    const char *ptr = strchr(base, col_name[i]);
    if (!ptr) {
      return -1;
    }
    int index = ptr - base;
    result += (int)pow((double)XML_Char_len(base), (double)j) * (index + 1);
  }
  return result;
}

int generate_columns(struct ArrayCols array_cols, unsigned short end_col_number, unsigned short index_worksheet) {
  int len_index_worksheet = snprintf(NULL, 0, "%d", index_worksheet);
  int LEN_CHUNKS_DIR_PATH = XML_Char_len(CHUNKS_DIR_PATH);
  int LEN_THE_FIRST_CHUNK_PATH = LEN_CHUNKS_DIR_PATH + len_index_worksheet + 15;
  char *THE_FIRST_CHUNK_PATH = XML_Char_malloc(LEN_THE_FIRST_CHUNK_PATH + 1);
  snprintf(THE_FIRST_CHUNK_PATH, LEN_THE_FIRST_CHUNK_PATH + 1, "%s/chunk_%d_0.chunk", CHUNKS_DIR_PATH, index_worksheet);
  FILE *fchunk0;
  fchunk0 = fopen(THE_FIRST_CHUNK_PATH, "ab+");
  if (fchunk0 == NULL) {
    debug_print("%s: %s\n", strerror(errno), THE_FIRST_CHUNK_PATH);
    free(THE_FIRST_CHUNK_PATH);
    return -1;
  }
  if (end_col_number == -1) {
    debug_print("End col number is -1\n");
    free(THE_FIRST_CHUNK_PATH);
    return -1;
  }
  free(THE_FIRST_CHUNK_PATH);
  fputs("<tr>", fchunk0);
  fputs("<th style=\"width:35px;height:15px;\"></th>", fchunk0);
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
  if (XML_Char_icmp(name, "row") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    int len_row_number = snprintf(NULL, 0, "%d", worksheet_callbackdata->ROW_NUMBER);
    int len_index_sheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
    for (int i = 1; i <= worksheet_callbackdata->end_col_number; i++) {
      //id: 0_B2
      char *col_name = int_to_column_name(i);
      int len_col_name = XML_Char_len(col_name);
      int len_id = len_index_sheet + len_col_name + len_row_number + 1;
      int len_td_str = 44 + len_id;
      char *td = XML_Char_malloc(len_td_str + 1);
      snprintf(
	td, len_td_str + 1,
	"<td id=\"%d_%s%d\" style=\"border-style:hidden;\"></td>",
	INDEX_CURRENT_SHEET, col_name,
	worksheet_callbackdata->ROW_NUMBER
      );
      fputs(td, worksheet_callbackdata->worksheet_file);
      free(col_name);
      free(td);
    }

    fputs("</tr>", worksheet_callbackdata->worksheet_file);
    if (ftell(worksheet_callbackdata->worksheet_file) >= CHUNK_SIZE_LIMIT) {
      worksheet_callbackdata->num_of_chunks++;
      int LEN_CHUNKS_DIR_PATH = XML_Char_len(OUTPUT_DIR) + XML_Char_len(CHUNKS_DIR_NAME) + 1 + 1;
      char *CHUNKS_DIR_PATH = XML_Char_malloc(LEN_CHUNKS_DIR_PATH);
      snprintf(CHUNKS_DIR_PATH, LEN_CHUNKS_DIR_PATH, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);
      //12: chunk_%d_%d.html
      int len_chunk_file_path = LEN_CHUNKS_DIR_PATH + snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET) + snprintf(NULL, 0, "%d", worksheet_callbackdata->num_of_chunks) + 13;
      char *CHUNK_FILE_PATH = XML_Char_malloc(len_chunk_file_path + 1);
      snprintf(CHUNK_FILE_PATH, len_chunk_file_path + 1, "%s/chunk_%d_%d.chunk", CHUNKS_DIR_PATH, INDEX_CURRENT_SHEET, worksheet_callbackdata->num_of_chunks);
      free(CHUNKS_DIR_PATH);
      if (worksheet_callbackdata->worksheet_file != NULL)
	fclose(worksheet_callbackdata->worksheet_file);
      worksheet_callbackdata->worksheet_file = fopen(CHUNK_FILE_PATH, "w");
      if (worksheet_callbackdata->worksheet_file == NULL) {
	debug_print("%s: %s\n", strerror(errno), CHUNK_FILE_PATH);
	exit(-1);
      }
      free(CHUNK_FILE_PATH);
    }
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
  int length = (int)XML_Char_len(_tmp_end_cell);
  char _tmp_end_col[4];
  int count_col_char = 0;
  while (*_tmp_end_cell) {
    if (*_tmp_end_cell >= 'A' && *_tmp_end_cell <= 'Z') {
      count_col_char++; 
      _tmp_end_col[count_col_char - 1] = *_tmp_end_cell;
    } else {
      _tmp_end_col[count_col_char] = '\0';
      *end_col = XML_Char_malloc((count_col_char + 1) * sizeof(char));
      memcpy(*end_col, _tmp_end_col, (count_col_char + 1) * sizeof(char));
      *end_row = XML_Char_malloc(sizeof(char) * (length - count_col_char + 1));
      memcpy(*end_row, _tmp_end_cell, (length - count_col_char + 1) * sizeof(char));
      return;
    }
    _tmp_end_cell++;
  }
}

void worksheet_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  struct WorkSheet *worksheet_callbackdata = callbackdata;
  if (XML_Char_icmp(name, "worksheet") == 0) {
    worksheet_callbackdata->has_dimension = '0';
  } else if (XML_Char_icmp(name, "dimension") == 0) {
    INDEX_CURRENT_SHEET = worksheet_callbackdata->index_sheet;
    worksheet_callbackdata->has_dimension = '1';
    char *_tmp_end_row = worksheet_callbackdata->end_row;
    char *_tmp_end_col = worksheet_callbackdata->end_col;
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "ref") == 0) {
        get_end_col_end_row_from_range(attrs[i + 1], &_tmp_end_row, &_tmp_end_col);
        worksheet_callbackdata->end_row = _tmp_end_row;
	worksheet_callbackdata->end_col = _tmp_end_col;
	worksheet_callbackdata->end_col_number = column_name_to_number(_tmp_end_col);
	break;
      }
    }
    XML_SetElementHandler(xmlparser, NULL, worksheet_end_element);
  } else if (XML_Char_icmp(name, "cols") == 0) {
    worksheet_callbackdata->array_cols.length = 0;
    worksheet_callbackdata->array_cols.cols = XML_Char_malloc(sizeof(struct Col *));
    if (worksheet_callbackdata->array_cols.cols == NULL) {
      debug_print("%s\n", strerror(errno));
      // TODO: Handle Error
    } else {
      XML_SetElementHandler(xmlparser, col_row_start_element, NULL);
    }
  } else if (XML_Char_icmp(name, "sheetData") == 0) {
    worksheet_callbackdata->ROW_NUMBER = 0;
    int LEN_CHUNKS_DIR_PATH = XML_Char_len(OUTPUT_DIR) + XML_Char_len(CHUNKS_DIR_NAME) + 1 + 1;
    char *CHUNKS_DIR_PATH = XML_Char_malloc(LEN_CHUNKS_DIR_PATH);
    snprintf(CHUNKS_DIR_PATH, LEN_CHUNKS_DIR_PATH, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);
    //12: chunk_%d_%d.html
    int len_chunk_file_path = LEN_CHUNKS_DIR_PATH + snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET) + snprintf(NULL, 0, "%d", worksheet_callbackdata->num_of_chunks) + 13;
    char *CHUNK_FILE_PATH = XML_Char_malloc(len_chunk_file_path + 1);
    snprintf(CHUNK_FILE_PATH, len_chunk_file_path + 1, "%s/chunk_%d_%d.chunk", CHUNKS_DIR_PATH, INDEX_CURRENT_SHEET, worksheet_callbackdata->num_of_chunks);
    free(CHUNKS_DIR_PATH);
    worksheet_callbackdata->worksheet_file = fopen(CHUNK_FILE_PATH, "w");
    if (worksheet_callbackdata->worksheet_file == NULL) {
      debug_print("%s: %s\n", strerror(errno), CHUNK_FILE_PATH);
      exit(-1);
    }
    free(CHUNK_FILE_PATH);
    XML_SetElementHandler(xmlparser, col_row_start_element, NULL);
  } else if (XML_Char_icmp(name, "mergeCells") == 0) {
    worksheet_callbackdata->hasMergedCells = '1';
    int LEN_CHUNKS_DIR_PATH = XML_Char_len(OUTPUT_DIR) + XML_Char_len(CHUNKS_DIR_NAME) + 1 + 1;
    char *CHUNKS_DIR_PATH = XML_Char_malloc(LEN_CHUNKS_DIR_PATH);
    snprintf(CHUNKS_DIR_PATH, LEN_CHUNKS_DIR_PATH, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);

    int len_index_worksheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
    //1: /  14: chunk_%d_mc.json
    int len_json_file_path = LEN_CHUNKS_DIR_PATH + 1 + 14 + len_index_worksheet;
    char JSON_FILE_PATH[len_json_file_path + 1];
    snprintf(JSON_FILE_PATH, len_json_file_path + 1, "%s/chunk_%d_mc.json", CHUNKS_DIR_PATH, INDEX_CURRENT_SHEET);
    free(CHUNKS_DIR_PATH);
    worksheet_callbackdata->fmergecell = fopen(JSON_FILE_PATH, "wb");
    if (worksheet_callbackdata->fmergecell == NULL) {
      debug_print("%s: %s\n", strerror(errno), JSON_FILE_PATH);
      return;
    }
    fputs("{", worksheet_callbackdata->fmergecell);
    XML_SetElementHandler(xmlparser, col_row_start_element, NULL);
  } else if (XML_Char_icmp(name, "drawing") == 0) {
    worksheet_callbackdata->array_drawingids.length++;
    worksheet_callbackdata->array_drawingids.drawing_ids = XML_Char_realloc(
      worksheet_callbackdata->array_drawingids.drawing_ids,
      worksheet_callbackdata->array_drawingids.length * sizeof(char *)
    );
    for (int i = 0; attrs[i]; i+=2) {
      if (XML_Char_icmp(attrs[i], "r:id") == 0) {
        worksheet_callbackdata->array_drawingids.drawing_ids[worksheet_callbackdata->array_drawingids.length - 1] = strdup(attrs[i + 1]);
      }
    }
    XML_SetElementHandler(xmlparser, NULL, worksheet_end_element);
  }
}

void worksheet_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "sheetData") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    fclose(worksheet_callbackdata->worksheet_file);
  } else if (XML_Char_icmp(name, "cols") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    int  status = generate_columns(worksheet_callbackdata->array_cols, worksheet_callbackdata->end_col_number, INDEX_CURRENT_SHEET);
    if (status == -1) {
      debug_print("%s\n", strerror(errno));
      return;
    }
    for (int index_col = 0; index_col < worksheet_callbackdata->array_cols.length; index_col++) {
      free(worksheet_callbackdata->array_cols.cols[index_col]);
    }
    free(worksheet_callbackdata->array_cols.cols);

  } else if (XML_Char_icmp(name, "mergeCells") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    fseek(worksheet_callbackdata->fmergecell, -1, SEEK_CUR);
    fputs("}", worksheet_callbackdata->fmergecell);
    fclose(worksheet_callbackdata->fmergecell);
  } else if (XML_Char_icmp(name, "drawing") == 0) {

  }

  XML_SetElementHandler(xmlparser, worksheet_start_element, NULL);
}

void col_row_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) {
  (void)attrs;
  if (XML_Char_icmp(name, "col") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    struct Col **_tmp_cols;
    worksheet_callbackdata->array_cols.length++;
    if (worksheet_callbackdata->array_cols.length > 1) {
      _tmp_cols = XML_Char_realloc(worksheet_callbackdata->array_cols.cols, worksheet_callbackdata->array_cols.length * sizeof(struct Col *));
      if (_tmp_cols) {
	worksheet_callbackdata->array_cols.cols = _tmp_cols;
      } else {
        debug_print("%s\n", strerror(errno));
	// TODO: Handle Error
      }
    }
    worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1] = XML_Char_malloc(sizeof(struct Col));
    worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->isHidden = '0';
    for (int i = 0; attrs[i]; i += 2) {
      if (XML_Char_icmp(attrs[i], "hidden") == 0) {
        worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->isHidden = XML_Char_icmp(attrs[i + 1], "true") == 0 ? '1' : '0'; 
      } else if (XML_Char_icmp(attrs[i], "min") == 0) {
	worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->min = (unsigned short)XML_Char_tol((char *)attrs[i + 1]);
      } else if (XML_Char_icmp(attrs[i], "max") == 0) {
	if (worksheet_callbackdata->has_dimension != '1')
	  worksheet_callbackdata->end_col_number = (unsigned short)XML_Char_tol((char *)attrs[i + 1]);
	worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->max = (unsigned short)XML_Char_tol((char *)attrs[i + 1]);
      } else if (XML_Char_icmp(attrs[i], "width") == 0) {
	worksheet_callbackdata->array_cols.cols[worksheet_callbackdata->array_cols.length - 1]->width = strtof((char *)attrs[i + 1], NULL);
      }
    }
    XML_SetElementHandler(xmlparser, NULL, col_row_end_element);
  } else if(XML_Char_icmp(name, "row") == 0) {
    //TODO: Need to calculate number of chunks first.
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    float row_height_in_px  = 0.0;
    unsigned short pre_row_number = 0;
    int len_row_number = 0;
    for (int i = 0; attrs[i]; i+=2) {
      if (XML_Char_icmp(attrs[i], "r") == 0) {
        if (worksheet_callbackdata->ROW_NUMBER != 0) {
	  pre_row_number = worksheet_callbackdata->ROW_NUMBER + 1;
        }
	worksheet_callbackdata->ROW_NUMBER = (unsigned short)XML_Char_tol(attrs[i + 1]);
	len_row_number = snprintf(NULL, 0, "%d", worksheet_callbackdata->ROW_NUMBER);
      } else if (XML_Char_icmp(attrs[i], "ht") == 0) {
	//<th style="height:px;"
	row_height_in_px = strtof((char *)attrs[i + 1], NULL) * (20 * 1.0 / 15);
      }
    }
    while (pre_row_number != 0 && worksheet_callbackdata->ROW_NUMBER != 0 && pre_row_number < worksheet_callbackdata->ROW_NUMBER) {
      int len_pre_row_number = snprintf(NULL, 0, "%d", pre_row_number);
      int LEN_TR_TAG = 11 + len_pre_row_number;
      char *TR_TAG = XML_Char_malloc(LEN_TR_TAG);
      snprintf(TR_TAG, LEN_TR_TAG, "<tr id=\"%d\">", pre_row_number);
      fputs(TR_TAG, worksheet_callbackdata->worksheet_file);
      free(TR_TAG);
      int LEN_TH_TAG = 20 + len_pre_row_number;
      char TH_TAG[LEN_TH_TAG];
      snprintf(TH_TAG, LEN_TH_TAG, "<th style=\"\">%d</th>", pre_row_number);
      fputs(TH_TAG, worksheet_callbackdata->worksheet_file);

      int len_index_sheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
      for (int i = 1; i <= worksheet_callbackdata->end_col_number; i++) {
	//id: 0_B2
	char *col_name = int_to_column_name(i);
	int len_col_name = XML_Char_len(col_name);
	int len_id = len_index_sheet + len_col_name + len_pre_row_number + 1;
	int len_td_str = 44 + len_id;
	char *td = XML_Char_malloc(len_td_str + 1);
	snprintf(
	  td, len_td_str + 1,
	  "<td id=\"%d_%s%d\" style=\"border-style:hidden;\"></td>",
	  INDEX_CURRENT_SHEET, col_name,
	  pre_row_number
	);
	fputs(td, worksheet_callbackdata->worksheet_file);
	free(col_name);
	free(td);
      }
      fputs("</tr>", worksheet_callbackdata->worksheet_file);
      if (ftell(worksheet_callbackdata->worksheet_file) >= CHUNK_SIZE_LIMIT) {
	worksheet_callbackdata->num_of_chunks++;
	int LEN_CHUNKS_DIR_PATH = XML_Char_len(OUTPUT_DIR) + XML_Char_len(CHUNKS_DIR_NAME) + 1 + 1;
	char *CHUNKS_DIR_PATH = XML_Char_malloc(LEN_CHUNKS_DIR_PATH);
	snprintf(CHUNKS_DIR_PATH, LEN_CHUNKS_DIR_PATH, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);
	//12: chunk_%d_%d.html
	int len_chunk_file_path = LEN_CHUNKS_DIR_PATH + snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET) + snprintf(NULL, 0, "%d", worksheet_callbackdata->num_of_chunks) + 13;
	char *CHUNK_FILE_PATH = XML_Char_malloc(len_chunk_file_path + 1);
	snprintf(CHUNK_FILE_PATH, len_chunk_file_path + 1, "%s/chunk_%d_%d.chunk", CHUNKS_DIR_PATH, INDEX_CURRENT_SHEET, worksheet_callbackdata->num_of_chunks);
	free(CHUNKS_DIR_PATH);
	if (worksheet_callbackdata->worksheet_file != NULL)
	  fclose(worksheet_callbackdata->worksheet_file);
	worksheet_callbackdata->worksheet_file = fopen(CHUNK_FILE_PATH, "w");
	if (worksheet_callbackdata->worksheet_file == NULL) {
	  debug_print("%s: %s\n", strerror(errno), CHUNK_FILE_PATH);
	  exit(-1);
	}
	free(CHUNK_FILE_PATH);
      }
      pre_row_number++;
    }
    int LEN_TR_TAG = 11 + len_row_number;
    char *TR_TAG = XML_Char_malloc(LEN_TR_TAG);
    snprintf(TR_TAG, LEN_TR_TAG, "<tr id=\"%d\">", worksheet_callbackdata->ROW_NUMBER);
    fputs(TR_TAG, worksheet_callbackdata->worksheet_file);
    free(TR_TAG);
    int len_row_height_in_px = snprintf(NULL, 0, "%.2f", row_height_in_px);

    int LEN_TH_TAG = 29 + len_row_height_in_px + len_row_number;
    char TH_TAG[LEN_TH_TAG];
    snprintf(TH_TAG, LEN_TH_TAG, "<th style=\"height:%.2fpx;\">%d</th>", row_height_in_px, worksheet_callbackdata->ROW_NUMBER);
    fputs(TH_TAG, worksheet_callbackdata->worksheet_file);
    START_CELL_IN_NUMBER_BY_ROW = 1;
    XML_SetElementHandler(xmlparser, cell_start_element, generate_cells);
  } else if (XML_Char_icmp(name, "mergeCell") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    for (int i = 0; attrs[i]; i+=2) {
      if (XML_Char_icmp(attrs[i], "ref") == 0) {
	char **mergecell_range = XML_Char_malloc(2 * sizeof(char *));
	int count = 0;
	char *token, *_tmp_range, *tofree;
	tofree = _tmp_range = strdup(attrs[i + 1]);
	while ((token = strsep(&_tmp_range, ":"))) {
	  int len_token = XML_Char_len(token);
	  mergecell_range[count] = XML_Char_malloc(len_token + 1);
	  memcpy(mergecell_range[count], token, len_token + 1);
	  mergecell_range[count][len_token] = '\0';
	  count++;
	}
        //You can't free _tmp_range because its value can be changed by calls to strsep().
	//The value of tofree consistently points to the start of the memory you want to free.
	free(tofree);
	size_t index_col_start = get_col_nr(mergecell_range[0]);
	size_t index_col_end = get_col_nr(mergecell_range[1]);
	size_t index_row_start = get_row_nr(mergecell_range[0]);
	size_t index_row_end = get_row_nr(mergecell_range[1]);
	unsigned short colspan = index_col_end - index_col_start + 1;
	unsigned short rowspan = index_row_end - index_row_start + 1;
	int len_start_cell = XML_Char_len(mergecell_range[0]);
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
  if (XML_Char_icmp(name, "col") == 0) {
    XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
  } else if (XML_Char_icmp(name, "row") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    int len_row_number = snprintf(NULL, 0, "%d", worksheet_callbackdata->ROW_NUMBER);
    int len_index_sheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
    while (START_CELL_IN_NUMBER_BY_ROW <= worksheet_callbackdata->end_col_number) {
      char *col_name = int_to_column_name(START_CELL_IN_NUMBER_BY_ROW);
      int len_col_name = XML_Char_len(col_name);
      int len_id = len_index_sheet + len_col_name + len_row_number + 1;
      int len_td_str = 44 + len_id;
      char *td = XML_Char_malloc(len_td_str + 1);
      snprintf(
	td, len_td_str + 1,
	"<td id=\"%d_%s%d\" style=\"border-style:hidden;\"></td>",
	INDEX_CURRENT_SHEET, col_name,
	worksheet_callbackdata->ROW_NUMBER
      );
      fputs(td, worksheet_callbackdata->worksheet_file);
      free(col_name);
      free(td);
      START_CELL_IN_NUMBER_BY_ROW++;
    }

    fputs("</tr>", worksheet_callbackdata->worksheet_file);
    if (ftell(worksheet_callbackdata->worksheet_file) >= CHUNK_SIZE_LIMIT) {
      worksheet_callbackdata->num_of_chunks++;
      int LEN_CHUNKS_DIR_PATH = XML_Char_len(OUTPUT_DIR) + XML_Char_len(CHUNKS_DIR_NAME) + 1 + 1;
      char *CHUNKS_DIR_PATH = XML_Char_malloc(LEN_CHUNKS_DIR_PATH);
      snprintf(CHUNKS_DIR_PATH, LEN_CHUNKS_DIR_PATH, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);
      //12: chunk_%d_%d.html
      int len_chunk_file_path = LEN_CHUNKS_DIR_PATH + snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET) + snprintf(NULL, 0, "%d", worksheet_callbackdata->num_of_chunks) + 13;
      char *CHUNK_FILE_PATH = XML_Char_malloc(len_chunk_file_path + 1);
      snprintf(CHUNK_FILE_PATH, len_chunk_file_path + 1, "%s/chunk_%d_%d.chunk", CHUNKS_DIR_PATH, INDEX_CURRENT_SHEET, worksheet_callbackdata->num_of_chunks);
      free(CHUNKS_DIR_PATH);

      if (worksheet_callbackdata->worksheet_file != NULL)
	fclose(worksheet_callbackdata->worksheet_file);
      worksheet_callbackdata->worksheet_file = fopen(CHUNK_FILE_PATH, "w");
      if (worksheet_callbackdata->worksheet_file == NULL) {
	debug_print("%s: %s\n", strerror(errno), CHUNK_FILE_PATH);
	exit(-1);
      }
      free(CHUNK_FILE_PATH);
    }
    XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
  } else if (XML_Char_icmp(name, "mergeCell") == 0) {
    XML_SetElementHandler(xmlparser, col_row_start_element, worksheet_end_element);
  }
}

void cell_start_element(void *callbackdata, const XML_Char *name, const XML_Char **attrs) { 
  (void)attrs;
  if (XML_Char_icmp(name, "c") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    worksheet_callbackdata->cell_name = NULL;
    worksheet_callbackdata->type_content = NULL;
    for (int i = 0; attrs[i]; i+=2) {
      if (XML_Char_icmp(attrs[i], "r") == 0) {
	worksheet_callbackdata->cell_name = XML_Char_realloc(worksheet_callbackdata->cell_name, 1 + XML_Char_len(attrs[i + 1]));
        memcpy(worksheet_callbackdata->cell_name, attrs[i + 1], 1 + XML_Char_len(attrs[i + 1]));
        CURRENT_CELL_IN_NUMBER_BY_ROW = (unsigned int)get_col_nr(attrs[i + 1]);
      } else if (XML_Char_icmp(attrs[i], "s") == 0) {
        worksheet_callbackdata->index_style = (int)XML_Char_tol(attrs[i + 1]);
      } else if (XML_Char_icmp(attrs[i], "t") == 0) {
        worksheet_callbackdata->type_content = XML_Char_realloc(worksheet_callbackdata->type_content, 1 + XML_Char_len(attrs[i + 1]));
	memcpy(worksheet_callbackdata->type_content, attrs[i + 1], 1 + XML_Char_len(attrs[i + 1]));
      }
    }

    int len_row_number = snprintf(NULL, 0, "%d", worksheet_callbackdata->ROW_NUMBER);
    int len_index_sheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
    while (START_CELL_IN_NUMBER_BY_ROW < CURRENT_CELL_IN_NUMBER_BY_ROW) {
      char *col_name = int_to_column_name(START_CELL_IN_NUMBER_BY_ROW);
      int len_col_name = XML_Char_len(col_name);
      int len_id = len_index_sheet + len_col_name + len_row_number + 1;
      int len_td_str = 44 + len_id;
      char *td = XML_Char_malloc(len_td_str + 1);
      snprintf(
        td, len_td_str + 1,
        "<td id=\"%d_%s%d\" style=\"border-style:hidden;\"></td>",
        INDEX_CURRENT_SHEET, col_name,
        worksheet_callbackdata->ROW_NUMBER
      );
      fputs(td, worksheet_callbackdata->worksheet_file);
      free(col_name);
      free(td);
      START_CELL_IN_NUMBER_BY_ROW++;
    }
    char *styles = NULL;
    char *horizontal_style = NULL;
    char *vertical_style =NULL;
    char *border_style = NULL;
    char *wraptext_style = NULL;
    char *horizontal = NULL;
    char *vertical = NULL;
    char *font_style = NULL;
    char *fill_style = NULL;
    char wrapText = '\0';
    if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyAlignment == '1') {
      if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.horizontal != NULL) {
	int len_horizontal = XML_Char_len(array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.horizontal);
	horizontal = XML_Char_realloc(horizontal, 1 + len_horizontal);
	memcpy(horizontal, array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.horizontal, 1 + len_horizontal);
	int len_vertical = XML_Char_len(array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.vertical);
	vertical = XML_Char_realloc(vertical, 1 + len_vertical);
	memcpy(vertical, array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.vertical, 1 + len_vertical);
	wrapText = array_cellXfs.Xfs[worksheet_callbackdata->index_style].alignment.isWrapText;
      }
    } else if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyAlignment == '0') {
      int id_cellXfs = array_cellXfs.Xfs[worksheet_callbackdata->index_style].xfId;
      if (array_cellStyleXfs.Xfs[id_cellXfs].alignment.horizontal != NULL) {
	int len_horizontal = XML_Char_len(array_cellStyleXfs.Xfs[id_cellXfs].alignment.horizontal);
	horizontal = XML_Char_realloc(horizontal, 1 + len_horizontal);
	memcpy(horizontal, array_cellStyleXfs.Xfs[id_cellXfs].alignment.horizontal, 1 + len_horizontal);
	int len_vertical = XML_Char_len(array_cellStyleXfs.Xfs[id_cellXfs].alignment.vertical);
	vertical = XML_Char_realloc(vertical, 1 + len_vertical);
	memcpy(vertical, array_cellStyleXfs.Xfs[id_cellXfs].alignment.vertical, 1 + len_vertical);
	wrapText = array_cellStyleXfs.Xfs[id_cellXfs].alignment.isWrapText;
      }
    }
    if (horizontal != NULL) {
      if (XML_Char_icmp(horizontal, "center") == 0) {
        //18: text-align:center;
        horizontal_style = XML_Char_realloc(horizontal_style, 18 + 1);
        snprintf(horizontal_style, 18 + 1, "text-align:center;");
      } else if (XML_Char_icmp(horizontal, "general") == 0) {
        //16: text-align:left;
        horizontal_style = XML_Char_realloc(horizontal_style, 16 + 1);
        snprintf(horizontal_style, 16 + 1, "text-align:left;");
      } else {
        //12: text-align:;
        horizontal_style = XML_Char_realloc(horizontal_style, XML_Char_len(horizontal) + 12 + 1);
        snprintf(horizontal_style, XML_Char_len(horizontal) + 12 + 1, "text-align:%s;", horizontal);
      }
      free(horizontal);
    }
    if (vertical != NULL) {
      if (XML_Char_icmp(vertical, "center") == 0) {
        //22: vertical-align:middle;
        vertical_style = XML_Char_realloc(vertical_style, 22 + 1);
        snprintf(vertical_style, 22 + 1, "vertical-align:middle;");
      } else {
        //16: vertical-align:;
        vertical_style = XML_Char_realloc(vertical_style, XML_Char_len(vertical) + 16 + 1);
        snprintf(vertical_style, XML_Char_len(vertical) + 16 + 1, "vertical-align:%s;", vertical);
      }
      free(vertical);
    }
    if (wrapText == '1') {
      //19: white-space:normal;
      wraptext_style = XML_Char_realloc(wraptext_style, 19 + 1);
      snprintf(wraptext_style, 19 + 1, "white-space:normal;");
    } else {
      //19: white-space:nowrap;
      wraptext_style = XML_Char_realloc(wraptext_style, 19 + 1);
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
        border_left = XML_Char_realloc(border_left, 25 + 1);
        snprintf(border_left, 25 + 1, "border-left-style:solid;");
        //TODO: If border_left == 'medium', then set border_width to 2px.
      } else {
        border_left = XML_Char_realloc(border_left, 26 + 1);
        snprintf(border_left, 26 + 1, "border-left-style:hidden;");
      }
      if (array_borders.borders[border_id].right.style != NULL) {
        //26: border-right-style:solid;
        border_right = XML_Char_realloc(border_right, 26 + 1);
        snprintf(border_right, 26 + 1, "border-right-style:solid;");
        //TODO: If border_right == 'medium', then set border_width to 2px.
      } else {
        border_right = XML_Char_realloc(border_right, 27 + 1);
        snprintf(border_right, 27 + 1, "border-right-style:hidden;");
      }
      if (array_borders.borders[border_id].top.style != NULL) {
        //24: border-top-style:solid;
        border_top = XML_Char_realloc(border_top, 24 + 1);
        snprintf(border_top, 24 + 1, "border-top-style:solid;");
        //TODO: If border_top == 'medium', then set border_width to 2px.
      } else {
        border_top = XML_Char_realloc(border_top, 25 + 1);
        snprintf(border_top, 25 + 1, "border-top-style:hidden;");
      }
      if (array_borders.borders[border_id].bottom.style != NULL) {
        //27: border-bottom-style:solid;
        border_bottom = XML_Char_realloc(border_bottom, 27 + 1);
        snprintf(border_bottom, 27 + 1, "border-bottom-style:solid;");
        //TODO: If border_bottom == 'medium', then set border_width to 2px.
      } else {
        border_bottom = XML_Char_realloc(border_bottom, 28 + 1);
        snprintf(border_bottom, 28 + 1, "border-bottom-style:hidden;");
      }
    }
    int len_borders = XML_Char_len(border_left) + XML_Char_len(border_right) + XML_Char_len(border_top) + XML_Char_len(border_bottom);
    border_style = XML_Char_realloc(border_style, len_borders + 1);
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
      const int LEN_FONT_NAME = 14 + XML_Char_len(array_fonts.fonts[font_id].name); //ex
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
	const int LEN_FONT_COLOR_RGB = 8 + XML_Char_len(array_fonts.fonts[font_id].color.rgb);
        char font_color_rgb[LEN_FONT_COLOR_RGB];
        // 6: "color:" | 1: ';'
        snprintf(font_color_rgb, LEN_FONT_COLOR_RGB, "color:%s;", array_fonts.fonts[font_id].color.rgb);
	char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, font_color_rgb);
	free(tmp_font_style);
      }
      if (array_fonts.fonts[font_id].underline != NULL && XML_Char_icmp(array_fonts.fonts[font_id].underline, "none") != 0) {
	const int LEN_FONT_TEXT_DECORATION_LINE = 32;
        char font_text_decoration_line[LEN_FONT_TEXT_DECORATION_LINE];
	memcpy(font_text_decoration_line, "text-decoration-line:underline;", LEN_FONT_TEXT_DECORATION_LINE);
	char *tmp_font_style = strdup(font_style);
	free(font_style);
	font_style = concat(tmp_font_style, font_text_decoration_line);
	free(tmp_font_style);
        if (XML_Char_icmp(array_fonts.fonts[font_id].underline, "single") == 0) {
	  const int LEN_FONT_TEXT_DECORATION_STYLE = 30;
          char font_text_decoration_style[LEN_FONT_TEXT_DECORATION_STYLE];
	  memcpy(font_text_decoration_style, "text-decoration-style:single;", LEN_FONT_TEXT_DECORATION_STYLE);
	  char *tmp_font_style = strdup(font_style);
	  free(font_style);
	  font_style = concat(tmp_font_style, font_text_decoration_style);
	  free(tmp_font_style);
        } else if(XML_Char_icmp(array_fonts.fonts[font_id].underline, "double") == 0) {
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
	LEN_FILL_FGCOLOR_RGB = 18 + XML_Char_len(array_fills.fills[fill_id].patternFill.fgColor.rgb);
	fill_style = XML_Char_realloc(fill_style, LEN_FILL_FGCOLOR_RGB + 1);
        snprintf(fill_style, LEN_FILL_FGCOLOR_RGB + 1, "background-color:%s;", array_fills.fills[fill_id].patternFill.fgColor.rgb);
      } else {
	fill_style = calloc(1, sizeof(char));
      }
      if (horizontal_style == NULL) {
	horizontal_style = calloc(1, sizeof(char));
      }
      if (vertical_style == NULL) {
	vertical_style = calloc(1, sizeof(char));
      }
      if (wraptext_style == NULL) {
	wraptext_style = calloc(1, sizeof(char));
      }
      if (border_style == NULL) {
	border_style = calloc(1, sizeof(char));
      }
      if (font_style == NULL) {
	font_style = calloc(1, sizeof(char));
      }

      int len_styles = XML_Char_len(horizontal_style) + XML_Char_len(vertical_style) + XML_Char_len(wraptext_style) + XML_Char_len(border_style) + XML_Char_len(font_style) + LEN_FILL_FGCOLOR_RGB;
      styles = XML_Char_realloc(styles, len_styles + 1);
      snprintf(styles, len_styles + 1, "%s%s%s%s%s%s", horizontal_style, vertical_style, wraptext_style, border_style, font_style, fill_style);
      free(horizontal_style);
      free(vertical_style);
      free(wraptext_style);
      free(border_style);
      free(font_style);
      free(fill_style);
      char *TD_TAG = NULL;
      int len_cellname = XML_Char_len(worksheet_callbackdata->cell_name);
      int len_index_sheet = snprintf(NULL, 0, "%d", INDEX_CURRENT_SHEET);
      //24: <td id="" style=""></td>
      //1: _
      int len_td_tag = 24 + len_styles + len_cellname + len_index_sheet + 1;
      TD_TAG = XML_Char_realloc(TD_TAG, len_td_tag + 1);
      snprintf(TD_TAG, len_td_tag + 1, "<td id=\"%d_%s\" style=\"%s\">", INDEX_CURRENT_SHEET, worksheet_callbackdata->cell_name, styles);
      fputs(TD_TAG, worksheet_callbackdata->worksheet_file);
      START_CELL_IN_NUMBER_BY_ROW++;
      free(styles);
      free(TD_TAG);
      XML_SetElementHandler(xmlparser, cell_item_start_element, cell_end_element);
    }
  }
}

void cell_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "c") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    fputs("</td>", worksheet_callbackdata->worksheet_file);
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
  if (XML_Char_icmp(name, "v") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    worksheet_callbackdata->worksheet_content = NULL;
    worksheet_callbackdata->len_worksheet_content = 0;
    XML_SetElementHandler(xmlparser, NULL, cell_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, worksheet_content_handler);
  } else if (XML_Char_icmp(name, "f") == 0) {
    XML_SetElementHandler(xmlparser, NULL, cell_item_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  }
}

void cell_item_end_element(void *callbackdata, const XML_Char *name) {
  if (XML_Char_icmp(name, "v") == 0) {
    struct WorkSheet *worksheet_callbackdata = callbackdata;
    if (worksheet_callbackdata->worksheet_content != NULL && worksheet_callbackdata->type_content != NULL) {
      if (XML_Char_icmp(worksheet_callbackdata->type_content, "s") == 0) {
        FILE *sharedStrings_file = fopen(SHAREDSTRINGS_HTML_FILE_PATH, "rb");
        if (sharedStrings_file == NULL) {
          debug_print("%s: %s\n", strerror(errno), SHAREDSTRINGS_HTML_FILE_PATH);
          return;
        }
        int len_pos_arr = sharedStrings_position.length;
        int index_sharedStrings_current = (int)XML_Char_tol(worksheet_callbackdata->worksheet_content);
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
      } else if (XML_Char_icmp(worksheet_callbackdata->type_content, "n") == 0) {
        unsigned short numFmt_id = -1;
	if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyNumberFormat == '1') {
	  numFmt_id = array_cellXfs.Xfs[worksheet_callbackdata->index_style].numFmtId;
	} else if (array_cellXfs.Xfs[worksheet_callbackdata->index_style].isApplyNumberFormat  == '0') {
	  int id_cellXfs = array_cellXfs.Xfs[worksheet_callbackdata->index_style].xfId;
	  numFmt_id = array_cellStyleXfs.Xfs[id_cellXfs].numFmtId;
	}
	int index_numFmt;
	for (index_numFmt = 0; index_numFmt < array_numfmts.length; index_numFmt++) {
	  int numFmtIdInt = XML_Char_tol(array_numfmts.numfmts[index_numFmt].numFmtId);
	  if (numFmtIdInt == numFmt_id) {
	    break;
	  }
	}
	if (XML_Char_icmp(array_numfmts.numfmts[index_numFmt].formatCode, "General") == 0) {
          fputs("<span>", worksheet_callbackdata->worksheet_file);
	} else {
	  // <span class="n" data-format-code="">
	  int len_span_html = XML_Char_len(array_numfmts.numfmts[index_numFmt].formatCode) + 36;
	  char *span_html = XML_Char_malloc(len_span_html + 1);
	  snprintf(span_html, len_span_html + 1, "<span class=\"n\" data-format-code=\"%s\">", array_numfmts.numfmts[index_numFmt].formatCode);
	  fputs(span_html, worksheet_callbackdata->worksheet_file);
	  free(span_html);
	}  
	fputs(worksheet_callbackdata->worksheet_content, worksheet_callbackdata->worksheet_file);
	fputs("</span>", worksheet_callbackdata->worksheet_file);
      } else {
        fputs("<span>", worksheet_callbackdata->worksheet_file);
        fputs(worksheet_callbackdata->worksheet_content, worksheet_callbackdata->worksheet_file);
        fputs("</span>", worksheet_callbackdata->worksheet_file);
      }
      free(worksheet_callbackdata->worksheet_content);
      worksheet_callbackdata->worksheet_content = NULL;
      worksheet_callbackdata->len_worksheet_content = 0;
    }
    XML_SetElementHandler(xmlparser, NULL, cell_end_element);
    XML_SetCharacterDataHandler(xmlparser, NULL);
  } else if (XML_Char_icmp(name, "f") == 0) {
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
    if ((worksheet_callbackdata->worksheet_content = XML_Char_realloc(worksheet_callbackdata->worksheet_content, len + 1)) == NULL) {
      return;
    }
    memcpy(worksheet_callbackdata->worksheet_content, buf, len);
    worksheet_callbackdata->worksheet_content[len] = '\0';
    worksheet_callbackdata->len_worksheet_content += len + 1;
  } else {
    int len_worksheet_content = worksheet_callbackdata->len_worksheet_content;
    if ((worksheet_callbackdata->worksheet_content = XML_Char_realloc(worksheet_callbackdata->worksheet_content, len_worksheet_content + len)) == NULL) {
      return;
    }
    memcpy(worksheet_callbackdata->worksheet_content + len_worksheet_content - 1, buf, len);
    worksheet_callbackdata->worksheet_content[len_worksheet_content + len - 1] = '\0';
    worksheet_callbackdata->len_worksheet_content += len;
  }
}
