#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <main.h>
#include <private.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <version.h>
#include <whereami.h> // wai_getExecutablePath()

XML_Parser xmlparser;
const char *ORIGIN_FILE_PATH;
const char *OUTPUT_DIR;
const char *OUTPUT_FILE_NAME;
const char *TEMP_DIR;
const char *RESOURCE_URL;
const char *WORKING_DIR;
const char *CHUNKS_DIR_PATH;
const char *SHAREDSTRINGS_HTML_FILE_PATH;

int err;

int mkdir_p(const char *path, mode_t mode) {
  /* Adapted from http://stackoverflow.com/a/2336245/119527 */
  const size_t len = XML_Char_len(path);
  char _path[PATH_MAX];
  char *p;

  errno = 0;

  /* Copy string so its mutable */
  if (len > sizeof(_path) - 1) {
    errno = ENAMETOOLONG;
    return -1;
  }
  strcpy(_path, path);

  /* Iterate the string */
  for (p = _path + 1; *p; p++) {
    if (*p == '/') {
      /* Temporarily truncate */
      *p = '\0';

      if (mkdir(_path, mode) != 0) {
        if (errno != EEXIST)
          return -1;
      }

      *p = '/';
    }
  }

  if (mkdir(_path, mode) != 0) {
    if (errno != EEXIST)
      return -1;
  }

  return 0;
}

zip_t *open_zip(const char *file_name) {
  return zip_open(file_name, ZIP_RDONLY, &err);
}

zip_file_t *open_zip_file(zip_t *zip, const char *zip_file_name) {
  return zip_fopen(zip, zip_file_name, ZIP_FL_UNCHANGED);
}

int load_relationships(zip_t *zip, char *zip_file_name, void *callbackdata) {
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  zip_error_t *err_zip = zip_get_error(zip);
  if (archive == NULL) {
    debug_print("%s: %s\n", zip_error_strerror(err_zip), zip_file_name);
    return -1;
  }
  int status = process_zip_file(archive, callbackdata, NULL, rels_start_element,
                                rels_end_element);
  return status;
}

int load_drawings(zip_t *zip, char *zip_file_name, void *callbackdata) {
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  zip_error_t *err_zip = zip_get_error(zip);
  if (archive == NULL) {
    debug_print("%s: %s\n", zip_error_strerror(err_zip), zip_file_name);
    return -1;
  }
  int status = process_zip_file(archive, callbackdata, NULL,
                                drawings_start_element, drawings_end_element);
  return status;
}

int load_chart(zip_t *zip, char *zip_file_name, void *callbackdata) {
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  zip_error_t *err_zip = zip_get_error(zip);
  if (archive == NULL) {
    debug_print("%s: %s\n", zip_error_strerror(err_zip), zip_file_name);
    return -1;
  }
  int status = process_zip_file(archive, callbackdata, NULL,
                                chart_start_element, chart_end_element);
  return status;
}

int load_workbook(zip_t *zip) {
  const char *zip_file_name = "xl/workbook.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  int status = process_zip_file(archive, NULL, NULL, workbook_start_element,
                                workbook_end_element);
  for (int i = 0; i < array_sheets.length; i++) {
    // 35: xl/worksheets/_rels/sheet%d.xml.rels
    int len_zip_sheet_rels_file_name =
        XML_Char_len(array_sheets.sheets[i]->sheetId) + 35;
    char *zip_sheet_rels_file_name =
        (char *)XML_Char_malloc(len_zip_sheet_rels_file_name + 1);
    snprintf(zip_sheet_rels_file_name, len_zip_sheet_rels_file_name + 1,
             "xl/worksheets/_rels/sheet%s.xml.rels",
             array_sheets.sheets[i]->sheetId);
    array_sheets.sheets[i]->array_worksheet_rels.length = 0;
    array_sheets.sheets[i]->array_worksheet_rels.relationships = NULL;
    int status_sheet_rels =
        load_relationships(zip, zip_sheet_rels_file_name,
                           &array_sheets.sheets[i]->array_worksheet_rels);
    free(zip_sheet_rels_file_name);
    if (status_sheet_rels != 1) {
      continue;
    }
  }
  return status;
}

void destroy_styles() {
  for (int i = 0; i < array_numfmts.length; i++) {
    free(array_numfmts.numfmts[i].formatCode);
    free(array_numfmts.numfmts[i].numFmtId);
  }
  free(array_numfmts.numfmts);
  for (int i = 0; i < array_fonts.length; i++) {
    free(array_fonts.fonts[i].name);
    free(array_fonts.fonts[i].underline);
    free(array_fonts.fonts[i].color.rgb);
  }
  free(array_fonts.fonts);
  for (int i = 0; i < array_fills.length; i++) {
    free(array_fills.fills[i].patternFill.patternType);
    free(array_fills.fills[i].patternFill.bgColor.rgb);
    free(array_fills.fills[i].patternFill.fgColor.rgb);
  }
  free(array_fills.fills);
  for (int i = 0; i < array_borders.length; i++) {
    free(array_borders.borders[i].left.style);
    free(array_borders.borders[i].left.color.rgb);
    free(array_borders.borders[i].right.style);
    free(array_borders.borders[i].right.color.rgb);
    free(array_borders.borders[i].top.style);
    free(array_borders.borders[i].top.color.rgb);
    free(array_borders.borders[i].bottom.style);
    free(array_borders.borders[i].bottom.color.rgb);
  }
  free(array_borders.borders);
  for (int i = 0; i < array_cellStyleXfs.length; i++) {
    free(array_cellStyleXfs.Xfs[i].alignment.horizontal);
    free(array_cellStyleXfs.Xfs[i].alignment.vertical);
    free(array_cellStyleXfs.Xfs[i].alignment.textRotation);
  }
  free(array_cellStyleXfs.Xfs);
  for (int i = 0; i < array_cellXfs.length; i++) {
    free(array_cellXfs.Xfs[i].alignment.horizontal);
    free(array_cellXfs.Xfs[i].alignment.vertical);
    free(array_cellXfs.Xfs[i].alignment.textRotation);
  }
  free(array_cellXfs.Xfs);
}

int load_styles(zip_t *zip) {
  const char *zip_file_name = "xl/styles.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  zip_error_t *err_zip = zip_get_error(zip);
  if (archive == NULL) {
    debug_print("%s: %s\n", zip_error_strerror(err_zip), zip_file_name);
    return -1;
  }
  // Load NumFMT first
  int status = process_zip_file(archive, NULL, NULL, styles_start_element,
                                styles_end_element);
  return status;
}

int load_worksheets(zip_t *zip) {
  for (int i = 0; i < array_sheets.length; i++) {
    zip_file_t *archive = open_zip_file(zip, array_sheets.sheets[i]->path_name);
    zip_error_t *err_zip = zip_get_error(zip);
    if (archive == NULL) {
      debug_print("%s: %s\n", zip_error_strerror(err_zip),
                  array_sheets.sheets[i]->path_name);
      return -1;
    }
    struct WorkSheet worksheet;
    worksheet.start_col = 'A';
    worksheet.start_row = '1';
    worksheet.end_col = NULL;
    worksheet.end_row = NULL;
    worksheet.index_sheet = i;
    worksheet.hasMergedCells = '0';
    worksheet.array_drawingids.length = 0;
    worksheet.array_drawingids.drawing_ids = NULL;
    worksheet.num_of_chunks = 1;
    worksheet.has_cols = '0';
    worksheet.array_cols.length = 0;

    int status_worksheet =
        process_zip_file(archive, &worksheet, NULL, worksheet_start_element,
                         worksheet_end_element);
    if (status_worksheet != 1) {
      return status_worksheet;
    }
    array_sheets.sheets[i]->hasMergedCells = worksheet.hasMergedCells;
    array_sheets.sheets[i]->array_drawing_rels.length = 0;
    array_sheets.sheets[i]->array_drawing_rels.relationships = NULL;
    array_sheets.sheets[i]->num_of_chunks = worksheet.num_of_chunks;
    array_sheets.sheets[i]->max_row = worksheet.ROW_NUMBER;
    array_sheets.sheets[i]->max_col_number = worksheet.end_col_number;

    for (int index_rels = 0;
         index_rels < array_sheets.sheets[i]->array_worksheet_rels.length;
         index_rels++) {
      for (int index_drawingid = 0;
           index_drawingid < worksheet.array_drawingids.length;
           index_drawingid++) {
        if (XML_Char_icmp(
                array_sheets.sheets[i]
                    ->array_worksheet_rels.relationships[index_rels]
                    ->id,
                worksheet.array_drawingids.drawing_ids[index_drawingid]) == 0) {
          if (XML_Char_icmp(
                  array_sheets.sheets[i]
                      ->array_worksheet_rels.relationships[index_rels]
                      ->type,
                  TYPE_DRAWING) == 0) {
            // 23: xl/drawings/_rels/<token>.rels
            int count = 0;
            char *_tmp_target =
                strdup(array_sheets.sheets[i]
                           ->array_worksheet_rels.relationships[index_rels]
                           ->target);
            char *token = strtok(_tmp_target, "/");
            count++;
            while (count <= 2) {
              token = strtok(NULL, "/");
              count++;
            }
            int len_zip_drawing_rels = XML_Char_len(token) + 23;
            char *zip_drawing_rels_file_name =
                (char *)XML_Char_malloc(len_zip_drawing_rels + 23 + 1);
            snprintf(zip_drawing_rels_file_name, len_zip_drawing_rels + 1,
                     "xl/drawings/_rels/%s.rels", token);
            int status_drawing_rels =
                load_relationships(zip, zip_drawing_rels_file_name,
                                   &array_sheets.sheets[i]->array_drawing_rels);
            if (status_drawing_rels == -1) {
              // TODO: Handle error
            }
            free(zip_drawing_rels_file_name);
            free(_tmp_target);
            break;
          } else {
            int _tmp_length =
                array_sheets.sheets[i]->array_worksheet_rels.length;
            if (index_rels < _tmp_length - 1) {
              memmove(
                  array_sheets.sheets[i]->array_worksheet_rels.relationships +
                      index_rels,
                  array_sheets.sheets[i]->array_worksheet_rels.relationships +
                      index_rels + 1,
                  (_tmp_length - index_rels - 1) *
                      sizeof(struct Relationship *));
              index_rels--;
            } else {
              free(array_sheets.sheets[i]
                       ->array_worksheet_rels.relationships[_tmp_length - 1]
                       ->id);
              free(array_sheets.sheets[i]
                       ->array_worksheet_rels.relationships[_tmp_length - 1]
                       ->target);
              free(array_sheets.sheets[i]
                       ->array_worksheet_rels.relationships[_tmp_length - 1]
                       ->type);
              free(array_sheets.sheets[i]
                       ->array_worksheet_rels.relationships[_tmp_length - 1]);
              array_sheets.sheets[i]->array_worksheet_rels.relationships =
                  (struct Relationship **)XML_Char_realloc(
                      array_sheets.sheets[i]
                          ->array_worksheet_rels.relationships,
                      (_tmp_length - 1) * sizeof(struct Relationship *));
            }
            array_sheets.sheets[i]->array_worksheet_rels.length--;
            break;
          }
        }
        if (index_drawingid == worksheet.array_drawingids.length - 1) {
          int _tmp_length = array_sheets.sheets[i]->array_worksheet_rels.length;
          if (index_rels < _tmp_length - 1) {
            free(array_sheets.sheets[i]
                     ->array_worksheet_rels.relationships[index_rels]
                     ->id);
            free(array_sheets.sheets[i]
                     ->array_worksheet_rels.relationships[index_rels]
                     ->target);
            free(array_sheets.sheets[i]
                     ->array_worksheet_rels.relationships[index_rels]
                     ->type);
            free(array_sheets.sheets[i]
                     ->array_worksheet_rels.relationships[index_rels]);
            memmove(array_sheets.sheets[i]->array_worksheet_rels.relationships +
                        index_rels,
                    array_sheets.sheets[i]->array_worksheet_rels.relationships +
                        index_rels + 1,
                    (_tmp_length - index_rels - 1) *
                        sizeof(struct Relationship *));
            index_rels--;
          } else {
            free(array_sheets.sheets[i]
                     ->array_worksheet_rels.relationships[_tmp_length - 1]
                     ->id);
            free(array_sheets.sheets[i]
                     ->array_worksheet_rels.relationships[_tmp_length - 1]
                     ->target);
            free(array_sheets.sheets[i]
                     ->array_worksheet_rels.relationships[_tmp_length - 1]
                     ->type);
            free(array_sheets.sheets[i]
                     ->array_worksheet_rels.relationships[_tmp_length - 1]);
            array_sheets.sheets[i]->array_worksheet_rels.relationships =
                (struct Relationship **)XML_Char_realloc(
                    array_sheets.sheets[i]->array_worksheet_rels.relationships,
                    (_tmp_length - 1) * sizeof(struct Relationship *));
          }
          array_sheets.sheets[i]->array_worksheet_rels.length--;
        }
      }
    }

    for (int index_drawingid = 0;
         index_drawingid < worksheet.array_drawingids.length;
         index_drawingid++) {
      free(worksheet.array_drawingids.drawing_ids[index_drawingid]);
    }
    free(worksheet.array_drawingids.drawing_ids);
    free(worksheet.end_row);
    free(worksheet.end_col);
  }
  free(sharedStrings_position.positions);
  return 1;
}

int load_sharedStrings(zip_t *zip) {
  const char *zip_file_name = "xl/sharedStrings.xml";
  int len_sharedStrings_html_file_name =
      XML_Char_len(OUTPUT_FILE_NAME) +
      XML_Char_len(SHAREDSTRINGS_HTML_FILE_SUFFIX);
  char *SHAREDSTRINGS_HTML_FILE_NAME =
      (char *)XML_Char_malloc(len_sharedStrings_html_file_name + 1);
  snprintf(SHAREDSTRINGS_HTML_FILE_NAME, len_sharedStrings_html_file_name + 1,
           "%s%s", OUTPUT_FILE_NAME, SHAREDSTRINGS_HTML_FILE_SUFFIX);
  int len_sharedStrings_file_path =
      XML_Char_len(TEMP_DIR) + 1 + len_sharedStrings_html_file_name;
  char *_SHAREDSTRINGS_HTML_FILE_PATH =
      (char *)XML_Char_malloc(len_sharedStrings_file_path + 1);
  snprintf(_SHAREDSTRINGS_HTML_FILE_PATH, len_sharedStrings_file_path + 1,
           "%s/%s", TEMP_DIR, SHAREDSTRINGS_HTML_FILE_NAME);
  free(SHAREDSTRINGS_HTML_FILE_NAME);
  SHAREDSTRINGS_HTML_FILE_PATH = strdup(_SHAREDSTRINGS_HTML_FILE_PATH);
  free(_SHAREDSTRINGS_HTML_FILE_PATH);
  FILE *sharedStrings_file;
  sharedStrings_file = fopen(SHAREDSTRINGS_HTML_FILE_PATH, "wb+");
  if (sharedStrings_file == NULL) {
    debug_print("%s: %s\n", strerror(errno), SHAREDSTRINGS_HTML_FILE_PATH);
    return -1;
  }
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  zip_error_t *err_zip = zip_get_error(zip);
  if (archive == NULL) {
    debug_print("%s: %s\n", zip_error_strerror(err_zip), zip_file_name);
    fclose(sharedStrings_file);
    return -1;
  }
  int status_sharedStrings = process_zip_file(archive, sharedStrings_file, NULL,
                                              sharedStrings_main_start_element,
                                              sharedStrings_main_end_element);
  if (status_sharedStrings == -1) {
    debug_print("%s\n", strerror(errno));
    fclose(sharedStrings_file);
    return -1;
  }
  fclose(sharedStrings_file);
  return 1;
}

int process_zip_file(zip_file_t *archive, void *callbackdata,
                     XML_CharacterDataHandler content_handler,
                     XML_StartElementHandler start_element,
                     XML_EndElementHandler end_element) {
  void *buf;
  zip_int64_t buflen;
  xmlparser = XML_ParserCreate(NULL);
  int done;
  enum XML_Status status = XML_STATUS_ERROR;
  XML_SetUserData(xmlparser, callbackdata);
  XML_SetElementHandler(xmlparser, start_element, end_element);
  XML_SetCharacterDataHandler(xmlparser, content_handler);
  buf = XML_GetBuffer(xmlparser, PARSE_BUFFER_SIZE);
  while (buf && (buflen = zip_fread(archive, buf, PARSE_BUFFER_SIZE)) >= 0) {
    done = buflen < PARSE_BUFFER_SIZE;
    if ((status = XML_ParseBuffer(xmlparser, (int)buflen, (done ? 1 : 0))) ==
        XML_STATUS_ERROR) {
      debug_print("%" XML_FMT_STR " at line %" XML_FMT_INT_MOD "u\n",
                  XML_ErrorString(XML_GetErrorCode(xmlparser)),
                  XML_GetCurrentLineNumber(xmlparser));
      XML_ParserFree(xmlparser);
      zip_fclose(archive);
      return 0;
    }
    if (done) {
      break;
    }
    buf = XML_GetBuffer(xmlparser, PARSE_BUFFER_SIZE);
  }
  XML_ParserFree(xmlparser);
  zip_fclose(archive);
  return 1;
}

void embed_css(FILE *f, const char *css_path) {
  const char *_css_path = css_path;
  FILE *fcss;
  fcss = fopen(_css_path, "rb");
  if (fcss == NULL) {
    debug_print("%s: %s\n", strerror(errno), css_path);
  }
  char c;
  while ((c = fgetc(fcss)) != EOF) {
    fputc(c, f);
  }
  fclose(fcss);
}

void embed_js(FILE *f, const char *js_path) {
  const char *_js_path = js_path;
  FILE *fjs;
  fjs = fopen(_js_path, "rb");
  if (fjs == NULL) {
    debug_print("%s: %s\n", strerror(errno), js_path);
  }
  char c;
  while ((c = fgetc(fjs)) != EOF) {
    fputc(c, f);
  }
  fclose(fjs);
}

void destroy_workbook() {
  for (int index_sheet = 0; index_sheet < array_sheets.length; index_sheet++) {
    free(array_sheets.sheets[index_sheet]->name);
    free(array_sheets.sheets[index_sheet]->sheetId);
    free(array_sheets.sheets[index_sheet]->path_name);
    free(array_sheets.sheets[index_sheet]);
  }
  free(array_sheets.sheets);
}

// Generate index html file
void pre_process(zip_t *zip) {
  int len_templates_dir_path =
      XML_Char_len(WORKING_DIR) + XML_Char_len(TEMPLATES_DIR_NAME) + 1;
  char *TEMPLATES_DIR_PATH =
      (char *)XML_Char_malloc(len_templates_dir_path + 1);
  snprintf(TEMPLATES_DIR_PATH, len_templates_dir_path + 1, "%s/%s", WORKING_DIR,
           TEMPLATES_DIR_NAME);
  int len_base_css_path =
      len_templates_dir_path + XML_Char_len(BASE_CSS_FILE_NAME) + 1;
  char *BASE_CSS_PATH = (char *)XML_Char_malloc(len_base_css_path + 1);
  snprintf(BASE_CSS_PATH, len_base_css_path + 1, "%s/%s", TEMPLATES_DIR_PATH,
           BASE_CSS_FILE_NAME);
  int len_base_js_path =
      len_templates_dir_path + XML_Char_len(BASE_JS_FILE_NAME) + 1;
  char *BASE_JS_PATH = (char *)XML_Char_malloc(len_base_js_path + 1);
  snprintf(BASE_JS_PATH, len_base_js_path + 1, "%s/%s", TEMPLATES_DIR_PATH,
           BASE_JS_FILE_NAME);
  int len_manifest_path =
      len_templates_dir_path + XML_Char_len(MANIFEST_FILE_NAME) + 1;
  char *MANIFEST_PATH = (char *)XML_Char_malloc(len_manifest_path + 1);
  snprintf(MANIFEST_PATH, len_manifest_path + 1, "%s/%s", TEMPLATES_DIR_PATH,
           MANIFEST_FILE_NAME);
  // 5: .html
  int len_index_html_path =
      XML_Char_len(OUTPUT_DIR) + XML_Char_len(OUTPUT_FILE_NAME) + 5 + 1;
  char *INDEX_HTML_PATH = (char *)XML_Char_malloc(len_index_html_path + 1);
  snprintf(INDEX_HTML_PATH, len_index_html_path + 1, "%s/%s.html", OUTPUT_DIR,
           OUTPUT_FILE_NAME);
  FILE *fmanifest;
  fmanifest = fopen(MANIFEST_PATH, "rb");
  if (fmanifest == NULL) {
    debug_print("%s: %s\n", strerror(errno), MANIFEST_PATH);
    return;
  }
  FILE *findexhtml;
  findexhtml = fopen(INDEX_HTML_PATH, "ab+");
  if (findexhtml == NULL) {
    debug_print("%s: %s\n", strerror(errno), INDEX_HTML_PATH);
    return;
  }
  int len_chunks_dir_path = XML_Char_len(CHUNKS_DIR_PATH);

  char line[256];
  while (fgets(line, sizeof(line), fmanifest)) {
    if (line[0] == '\n') {
      continue;
    } else if (line[0] == '#') {
      continue;
    } else if (line[0] == '@') {
      if (XML_Char_icmp(line, "@base.min.css\n") == 0) {
        fputs("<style>", findexhtml);
        embed_css(findexhtml, BASE_CSS_PATH);
        fputs("</style>", findexhtml);
      } else if (XML_Char_icmp(line, "@xlsxmagic.min.js\n") == 0) {
        fputs("<script>", findexhtml);
        embed_js(findexhtml, BASE_JS_PATH);
        fputs("</script>", findexhtml);
      }
    } else if (line[0] == '$') {
      if (XML_Char_icmp(line, "$version\n") == 0) {
        fputs("<title>", findexhtml);
        fputs(XLSXMAGIC_FULLNAME, findexhtml);
        fputs("</title>", findexhtml);
      } else if (XML_Char_icmp(line, "$tables\n") == 0) {
        for (int index_sheet = 0; index_sheet < array_sheets.length;
             index_sheet++) {
          int len_index_sheet = snprintf(NULL, 0, "%d", index_sheet);
          int len_num_of_chunks =
              snprintf(NULL, 0, "%d",
                       array_sheets.sheets[index_sheet]->num_of_chunks + 1);
          int index_image = -1;
          int index_graphicframe = -1;
          struct ArrayDrawingCallbackData array_drawing_callbackdata;
          array_drawing_callbackdata.length = 0;
          array_drawing_callbackdata.drawing_callbackdata = NULL;
          array_drawing_callbackdata.max_row_drawing = 0;

          for (int index_rels = 0;
               index_rels <
               array_sheets.sheets[index_sheet]->array_worksheet_rels.length;
               index_rels++) {
            int len_zip_drawing_file_name = XML_Char_len(
                array_sheets.sheets[index_sheet]
                    ->array_worksheet_rels.relationships[index_rels]
                    ->target);
            char *zip_drawing_file_name =
                (char *)XML_Char_malloc(len_zip_drawing_file_name + 1);
            snprintf(zip_drawing_file_name, len_zip_drawing_file_name + 1,
                     "xl%s",
                     array_sheets.sheets[index_sheet]
                             ->array_worksheet_rels.relationships[index_rels]
                             ->target +
                         2);
            free(array_sheets.sheets[index_sheet]
                     ->array_worksheet_rels.relationships[index_rels]
                     ->id);
            free(array_sheets.sheets[index_sheet]
                     ->array_worksheet_rels.relationships[index_rels]
                     ->target);
            free(array_sheets.sheets[index_sheet]
                     ->array_worksheet_rels.relationships[index_rels]
                     ->type);
            free(array_sheets.sheets[index_sheet]
                     ->array_worksheet_rels.relationships[index_rels]);
            int status_drawings = load_drawings(zip, zip_drawing_file_name,
                                                &array_drawing_callbackdata);
            free(zip_drawing_file_name);
            if (status_drawings == -1) {
              // TODO: Handle error
              debug_print("Some thing went wrong durring drawing rendering\n");
              continue;
            }
          }
          // Occur if max row of the drawing is bigger than max row of the sheet
          if (array_sheets.sheets[index_sheet]->max_row <
              array_drawing_callbackdata.max_row_drawing) {
            array_sheets.sheets[index_sheet]->num_of_chunks++;
            int LEN_CHUNKS_DIR_PATH = XML_Char_len(OUTPUT_DIR) +
                                      XML_Char_len(CHUNKS_DIR_NAME) + 1 + 1;
            char *CHUNKS_DIR_PATH =
                (char *)XML_Char_malloc(LEN_CHUNKS_DIR_PATH);
            snprintf(CHUNKS_DIR_PATH, LEN_CHUNKS_DIR_PATH, "%s/%s", OUTPUT_DIR,
                     CHUNKS_DIR_NAME);
            // 12: chunk_%d_%d.html
            len_num_of_chunks =
                snprintf(NULL, 0, "%d",
                         array_sheets.sheets[index_sheet]->num_of_chunks + 1);
            int len_chunk_file_path =
                LEN_CHUNKS_DIR_PATH + len_index_sheet + len_num_of_chunks + 13;
            char *CHUNK_FILE_PATH =
                (char *)XML_Char_malloc(len_chunk_file_path + 1);
            snprintf(CHUNK_FILE_PATH, len_chunk_file_path + 1,
                     "%s/chunk_%d_%d.chunk", CHUNKS_DIR_PATH, index_sheet,
                     array_sheets.sheets[index_sheet]->num_of_chunks);
            free(CHUNKS_DIR_PATH);
            FILE *worksheet_file = fopen(CHUNK_FILE_PATH, "w");
            if (worksheet_file == NULL) {
              debug_print("%s: %s\n", strerror(errno), CHUNK_FILE_PATH);
              exit(-1);
            }
            free(CHUNK_FILE_PATH);
            while (array_sheets.sheets[index_sheet]->max_row++ <
                   array_drawing_callbackdata.max_row_drawing) {
              int len_row_number = snprintf(
                  NULL, 0, "%d", array_sheets.sheets[index_sheet]->max_row);
              float row_height_in_px = 15;
              int len_row_height_in_px =
                  snprintf(NULL, 0, "%.2f", row_height_in_px);
              int LEN_TR_TAG = 11 + len_row_number;
              char *TR_TAG = (char *)XML_Char_malloc(LEN_TR_TAG);
              snprintf(TR_TAG, LEN_TR_TAG, "<tr id=\"%d\">",
                       array_sheets.sheets[index_sheet]->max_row);
              fputs(TR_TAG, worksheet_file);
              free(TR_TAG);
              int LEN_TH_TAG = 29 + len_row_height_in_px + len_row_number;
              char TH_TAG[LEN_TH_TAG];
              snprintf(TH_TAG, LEN_TH_TAG,
                       "<th style=\"height:%.2fpx;\">%d</th>", row_height_in_px,
                       array_sheets.sheets[index_sheet]->max_row);
              fputs(TH_TAG, worksheet_file);
              _generate_cells(array_sheets.sheets[index_sheet]->max_row,
                              array_sheets.sheets[index_sheet]->max_col_number,
                              index_sheet, worksheet_file,
                              &array_sheets.sheets[index_sheet]->num_of_chunks);
            }
            fclose(worksheet_file);
          }
          int len_div_table =
              47 + len_index_sheet +
              XML_Char_len(array_sheets.sheets[index_sheet]->name) +
              len_num_of_chunks;
          char *DIV_TABLE = (char *)XML_Char_malloc(len_div_table + 1);
          snprintf(
              DIV_TABLE, len_div_table + 1,
              "<div id=\"sheet_%d\" name=\"%s\" data-num-of-chunks=\"%d\">",
              index_sheet, array_sheets.sheets[index_sheet]->name,
              array_sheets.sheets[index_sheet]->num_of_chunks + 1);
          fputs(DIV_TABLE, findexhtml);
          free(DIV_TABLE);
          if (array_sheets.sheets[index_sheet]->hasMergedCells == '1') {
            int len_chunk_mc_file_name = len_index_sheet + 9;
            char *CHUNK_MC_FILE_NAME =
                (char *)XML_Char_malloc(len_chunk_mc_file_name + 1);
            snprintf(CHUNK_MC_FILE_NAME, len_chunk_mc_file_name + 1,

                     "chunk_%d_mc", index_sheet);
            int len_resource_url, len_chunk_mc_url;
            char *CHUNK_MC_URL;
            if (strstr(RESOURCE_URL, "http") != NULL) {
              len_resource_url = XML_Char_len(RESOURCE_URL);
              int len_output_file_name = XML_Char_len(OUTPUT_FILE_NAME);
              len_chunk_mc_url = len_chunk_mc_file_name + len_resource_url +
                                 len_output_file_name + 8;
              CHUNK_MC_URL = (char *)XML_Char_malloc(len_chunk_mc_url + 1);
              snprintf(CHUNK_MC_URL, len_chunk_mc_url + 1, "%s/chunks/%s.json",
                       RESOURCE_URL, CHUNK_MC_FILE_NAME);
            } else {
              len_chunk_mc_url =
                  len_chunk_mc_file_name + len_chunks_dir_path + 6;
              CHUNK_MC_URL = (char *)XML_Char_malloc(len_chunk_mc_url + 1);
              snprintf(CHUNK_MC_URL, len_chunk_mc_url + 1, "%s/%s.json",
                       CHUNKS_DIR_PATH, CHUNK_MC_FILE_NAME);
            }
            int len_div_chunk = len_chunk_mc_file_name + len_chunk_mc_url + 35;
            char *DIV_CHUNK = (char *)XML_Char_malloc(len_div_chunk + 1);
            snprintf(DIV_CHUNK, len_div_chunk + 1,
                     "<div id=\"%s\" data-chunk-url=\"%s\"></div>",
                     CHUNK_MC_FILE_NAME, CHUNK_MC_URL);
            free(CHUNK_MC_URL);
            fputs(DIV_CHUNK, findexhtml);
            free(DIV_CHUNK);
            fputs("\n", findexhtml);
            free(CHUNK_MC_FILE_NAME);
          }

          // DRAWING RENDERING.
          for (int i_drawing = 0; i_drawing < array_drawing_callbackdata.length;
               i_drawing++) {
            char *drawing_id = NULL;
            if (array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                    ->is_pic == '1') {
              drawing_id = strdup(
                  array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                      ->twocellanchor.pic.blip_embed);
            } else if (array_drawing_callbackdata
                           .drawing_callbackdata[i_drawing]
                           ->is_graphicframe == '1') {
              drawing_id = strdup(
                  array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                      ->twocellanchor.graphic_frame.chart_id);
            }
            if (drawing_id == NULL)
              continue;
            int len_from_row = snprintf(
                NULL, 0, "%u",
                array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                    ->twocellanchor.from.row);
            char *from_col_name = int_to_column_name(
                array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                    ->twocellanchor.from.col);
            int len_from_col_name = XML_Char_len(from_col_name);
            int len_output_file_name = XML_Char_len(OUTPUT_FILE_NAME);

            for (int i_drawing_rel = 0;
                 i_drawing_rel <
                 array_sheets.sheets[index_sheet]->array_drawing_rels.length;
                 i_drawing_rel++) {
              int len_target = XML_Char_len(
                  array_sheets.sheets[index_sheet]
                      ->array_drawing_rels.relationships[i_drawing_rel]
                      ->target);
              char *_tmp_target = (char *)XML_Char_malloc(len_target + 1);
              snprintf(
                  _tmp_target, len_target + 1, "xl%s",
                  array_sheets.sheets[index_sheet]
                          ->array_drawing_rels.relationships[i_drawing_rel]
                          ->target +
                      2);
              if (XML_Char_icmp(drawing_id, array_sheets.sheets[index_sheet]
                                                ->array_drawing_rels
                                                .relationships[i_drawing_rel]
                                                ->id) == 0) {
                if (array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                        ->is_pic == '1') {
                  struct zip_stat sb;
                  struct zip_file *img_zf;
                  if (zip_stat(zip, _tmp_target, 0, &sb) == 0) {
                    img_zf = zip_fopen(zip, sb.name, 0);
                    if (!img_zf) {
                      debug_print("%s: %s\n", strerror(errno), sb.name);
                      continue;
                    }
                    char *img_zf_name = strdup(sb.name);
                    char *token = strtok(img_zf_name, "/");
                    int count = 0;
                    count++;
                    while (count <= 2) {
                      token = strtok(NULL, "/");
                      count++;
                    }
                    char *img_name = strdup(token);
                    token = strtok(img_name, ".");
                    char *img_ext = strdup(token);
                    while (token != NULL) {
                      free(img_ext);
                      img_ext = NULL;
                      img_ext = strdup(token);
                      token = strtok(NULL, ".");
                    }
                    free(token);
                    free(img_zf_name);
                    if (XML_Char_icmp(img_ext, "wmf") == 0) {
                      free(img_name);
                      free(img_ext);
                      free(_tmp_target);
                      free(from_col_name);
                      zip_fclose(img_zf);
                      continue;
                    }
                    index_image++;
                    int len_output_img_file_path = XML_Char_len(OUTPUT_DIR) +
                                                   XML_Char_len(img_name) +
                                                   XML_Char_len(img_ext) + 2;
                    char *OUTPUT_IMG_FILE_PATH =
                        (char *)XML_Char_malloc(len_output_img_file_path + 1);
                    snprintf(OUTPUT_IMG_FILE_PATH, len_output_img_file_path + 1,
                             "%s/%s.%s", OUTPUT_DIR, img_name, img_ext);
                    int img_fd =
                        open(OUTPUT_IMG_FILE_PATH, O_WRONLY | O_CREAT, 0644);
                    if (img_fd < 0) {
                      debug_print("%s: %s\n", strerror(errno),
                                  OUTPUT_IMG_FILE_PATH);
                      continue;
                    }
                    char buf[100];
                    int len = 0;
                    long long sum = 0;
                    while (sum != sb.size) {
                      len = zip_fread(img_zf, buf, 100);
                      if (len < 0) {
                        debug_print("%s\n", strerror(errno));
                        break;
                      }
                      write(img_fd, buf, len);
                      sum += len;
                    }
                    close(img_fd);
                    zip_fclose(img_zf);
                    char *IMG_URL = NULL;
                    int len_resource_url, len_img_name, len_img_url;
                    if (strstr(RESOURCE_URL, "http") != NULL) {
                      len_resource_url = XML_Char_len(RESOURCE_URL);
                      len_img_name = XML_Char_len(img_name);
                      int len_img_ext = XML_Char_len(img_ext);
                      // 17: /img/%s?format_img=
                      len_img_url = len_img_name + len_resource_url +
                                    len_img_ext + len_output_file_name + 18;
                      IMG_URL = (char *)XML_Char_malloc(len_img_url + 1);
                      snprintf(IMG_URL, len_img_url + 1, "%s/%s.%s",
                               RESOURCE_URL, img_name, img_ext);
                    } else {
                      IMG_URL = strdup(OUTPUT_IMG_FILE_PATH);
                      len_img_url = len_output_img_file_path;
                      free(OUTPUT_IMG_FILE_PATH);
                    }
                    free(img_name);
                    free(img_ext);
                    // http://officeopenxml.com/drwPicInSpread-oneCell.php
                    // EMUs to pixels: value / 9525 (1 pixel = 9525 EMUs)
                    size_t height = array_drawing_callbackdata
                                        .drawing_callbackdata[i_drawing]
                                        ->twocellanchor.pic.cy /
                                    9525;
                    size_t width = array_drawing_callbackdata
                                       .drawing_callbackdata[i_drawing]
                                       ->twocellanchor.pic.cx /
                                   9525;
                    int len_height = snprintf(NULL, 0, "%zu", height);
                    int len_width = snprintf(NULL, 0, "%zu", width);
                    size_t from_colOff = array_drawing_callbackdata
                                             .drawing_callbackdata[i_drawing]
                                             ->twocellanchor.from.colOff /
                                         9525;
                    size_t from_rowOff = array_drawing_callbackdata
                                             .drawing_callbackdata[i_drawing]
                                             ->twocellanchor.from.rowOff /
                                         9525;
                    int len_from_colOff = snprintf(NULL, 0, "%zu", from_colOff);
                    int len_from_rowOff = snprintf(NULL, 0, "%zu", from_rowOff);
                    int len_index_img = snprintf(NULL, 0, "%d", index_image);
                    int len_div_img = len_index_sheet + len_index_img +
                                      len_img_url + len_height + len_width +
                                      len_from_col_name + len_from_row +
                                      len_from_colOff + len_from_rowOff + 141;
                    char *DIV_IMG = (char *)XML_Char_malloc(len_div_img + 1);
                    snprintf(
                        DIV_IMG, len_div_img + 1,
                        "<div id=\"chunk_%d_%d_img\" data-img-url=\"%s\" "
                        "data-height=\"%zu\" data-width=\"%zu\" "
                        "data-from-col=\"%s\" data-from-row=\"%u\" "
                        "data-from-coloff=\"%zu\" data-from-rowoff=\"%zu\">",
                        index_sheet, index_image, IMG_URL, height, width,
                        from_col_name,
                        array_drawing_callbackdata
                            .drawing_callbackdata[i_drawing]
                            ->twocellanchor.from.row,
                        from_colOff, from_rowOff);
                    free(from_col_name);
                    fputs(DIV_IMG, findexhtml);
                    fputs("</div>", findexhtml);
                    fputs("\n", findexhtml);
                    free(DIV_IMG);
                    free(IMG_URL);
                  }
                } else if (array_drawing_callbackdata
                               .drawing_callbackdata[i_drawing]
                               ->is_graphicframe == '1') {
                  index_graphicframe++;
                  int len_index_graphicframe =
                      snprintf(NULL, 0, "%d", index_graphicframe);
                  // chunk_%d_%d_chart
                  int len_chart_json_file_name =
                      len_index_sheet + len_index_graphicframe + 18;
                  char *chart_json_file_name =
                      (char *)XML_Char_malloc(len_chart_json_file_name + 1);
                  snprintf(chart_json_file_name, len_chart_json_file_name + 1,
                           "chunk_%d_%d_chart", index_sheet,
                           index_graphicframe);
                  int len_output_chart_file_path =
                      XML_Char_len(OUTPUT_DIR) + XML_Char_len(CHUNKS_DIR_NAME) +
                      len_chart_json_file_name + 5 + 1;
                  char *OUTPUT_CHART_FILE_PATH =
                      (char *)XML_Char_malloc(len_output_chart_file_path + 1);
                  snprintf(OUTPUT_CHART_FILE_PATH,
                           len_output_chart_file_path + 1, "%s/%s/%s.json",
                           OUTPUT_DIR, CHUNKS_DIR_NAME, chart_json_file_name);
                  char *CHART_URL = NULL;
                  int len_chart_url, len_resource_url;
                  if (strstr(RESOURCE_URL, "http") != NULL) {
                    len_resource_url = XML_Char_len(RESOURCE_URL);
                    len_chart_url = len_output_chart_file_path +
                                    len_resource_url + +len_output_file_name +
                                    8;
                    CHART_URL = (char *)XML_Char_malloc(len_chart_url + 1);
                    snprintf(CHART_URL, len_chart_url + 1, "%s/chunks/%s.json",
                             RESOURCE_URL, chart_json_file_name);
                  } else {
                    CHART_URL = strdup(OUTPUT_CHART_FILE_PATH);
                    len_chart_url = len_output_chart_file_path;
                  }
                  size_t height = array_drawing_callbackdata
                                      .drawing_callbackdata[i_drawing]
                                      ->twocellanchor.graphic_frame.cy /
                                  9525;
                  size_t width = array_drawing_callbackdata
                                     .drawing_callbackdata[i_drawing]
                                     ->twocellanchor.graphic_frame.cx /
                                 9525;
                  size_t from_colOff = array_drawing_callbackdata
                                           .drawing_callbackdata[i_drawing]
                                           ->twocellanchor.from.colOff /
                                       9525;
                  size_t from_rowOff = array_drawing_callbackdata
                                           .drawing_callbackdata[i_drawing]
                                           ->twocellanchor.from.rowOff /
                                       9525;
                  int len_height = snprintf(NULL, 0, "%zu", height);
                  int len_width = snprintf(NULL, 0, "%zu", width);
                  int len_from_colOff = snprintf(NULL, 0, "%zu", from_colOff);
                  int len_from_rowOff = snprintf(NULL, 0, "%zu", from_rowOff);
                  int len_chart_name =
                      XML_Char_len(array_drawing_callbackdata
                                       .drawing_callbackdata[i_drawing]
                                       ->twocellanchor.graphic_frame.name);
                  int len_div_chart = len_chart_json_file_name + len_chart_url +
                                      len_chart_name + len_from_col_name +
                                      len_from_row + len_from_colOff +
                                      len_from_rowOff + len_height + len_width +
                                      145;
                  char *DIV_CHART = (char *)XML_Char_malloc(len_div_chart + 1);
                  snprintf(
                      DIV_CHART, len_div_chart + 1,
                      "<div id=\"%s\" data-chart-url=\"%s\" data-name=\"%s\" "
                      "data-from-col=\"%s\" data-from-row=\"%u\" "
                      "data-from-rowoff=\"%zu\" data-from-coloff=\"%zu\" "
                      "data-height=\"%zu\" data-width=\"%zu\">",
                      chart_json_file_name, CHART_URL,
                      array_drawing_callbackdata
                          .drawing_callbackdata[i_drawing]
                          ->twocellanchor.graphic_frame.name,
                      from_col_name,
                      array_drawing_callbackdata
                          .drawing_callbackdata[i_drawing]
                          ->twocellanchor.from.row,
                      from_rowOff, from_colOff, height, width);
                  free(from_col_name);
                  fputs(DIV_CHART, findexhtml);
                  fputs("</div>", findexhtml);
                  fputs("\n", findexhtml);
                  free(DIV_CHART);
                  free(chart_json_file_name);
                  free(CHART_URL);
                  struct ChartCallBackData chart_callbackdata;
                  int status_init = chart_callbackdata_initialize(
                      &chart_callbackdata, OUTPUT_CHART_FILE_PATH, index_sheet);
                  free(OUTPUT_CHART_FILE_PATH);
                  if (status_init == -1) {
                    // TODO: Handle error
                    continue;
                  }
                  int len_zip_chart_file_name = XML_Char_len(
                      array_sheets.sheets[index_sheet]
                          ->array_drawing_rels.relationships[i_drawing_rel]
                          ->target);
                  char *zip_chart_file_name =
                      (char *)XML_Char_malloc(len_zip_chart_file_name + 1);
                  snprintf(
                      zip_chart_file_name, len_zip_chart_file_name + 1, "xl%s",
                      array_sheets.sheets[index_sheet]
                              ->array_drawing_rels.relationships[i_drawing_rel]
                              ->target +
                          2);
                  int status_chart =
                      load_chart(zip, zip_chart_file_name, &chart_callbackdata);
                  if (status_chart != 1) {
                    // TODO: Handle error
                  }
                  free(zip_chart_file_name);
                  fclose(chart_callbackdata.fchart);
                }
              }
              free(_tmp_target);
            }
            free(drawing_id);
          }
          for (int i_drawing = 0; i_drawing < array_drawing_callbackdata.length;
               i_drawing++) {
            if (array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                    ->is_pic == '1') {
              if (array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                      ->twocellanchor.pic.name != NULL)
                free(array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                         ->twocellanchor.pic.name);
              free(array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                       ->twocellanchor.pic.hlinkClick_id);
              free(array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                       ->twocellanchor.pic.blip_embed);
            } else if (array_drawing_callbackdata
                           .drawing_callbackdata[i_drawing]
                           ->is_graphicframe == '1') {
              if (array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                      ->twocellanchor.graphic_frame.name != NULL)
                free(array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                         ->twocellanchor.graphic_frame.name);
              free(array_drawing_callbackdata.drawing_callbackdata[i_drawing]
                       ->twocellanchor.graphic_frame.chart_id);
            }
            free(array_drawing_callbackdata.drawing_callbackdata[i_drawing]);
          }
          free(array_drawing_callbackdata.drawing_callbackdata);
          for (int i_drawing_rel = 0;
               i_drawing_rel <
               array_sheets.sheets[index_sheet]->array_drawing_rels.length;
               i_drawing_rel++) {
            free(array_sheets.sheets[index_sheet]
                     ->array_drawing_rels.relationships[i_drawing_rel]
                     ->id);
            free(array_sheets.sheets[index_sheet]
                     ->array_drawing_rels.relationships[i_drawing_rel]
                     ->target);
            free(array_sheets.sheets[index_sheet]
                     ->array_drawing_rels.relationships[i_drawing_rel]
                     ->type);
            free(array_sheets.sheets[index_sheet]
                     ->array_drawing_rels.relationships[i_drawing_rel]);
          }
          if (array_sheets.sheets[index_sheet]->array_drawing_rels.length != 0)
            free(array_sheets.sheets[index_sheet]
                     ->array_drawing_rels.relationships);
          if (array_sheets.sheets[index_sheet]->array_worksheet_rels.length !=
              0)
            free(array_sheets.sheets[index_sheet]
                     ->array_worksheet_rels.relationships);

          for (int index_chunk = 0;
               index_chunk <= array_sheets.sheets[index_sheet]->num_of_chunks;
               index_chunk++) {
            // 7: chunk_%d_%d
            int len_chunk_html_file_name =
                snprintf(NULL, 0, "%d", index_chunk) + len_index_sheet + 7;
            char *CHUNK_HTML_FILE_NAME =
                (char *)XML_Char_malloc(len_chunk_html_file_name + 1);
            snprintf(CHUNK_HTML_FILE_NAME, len_chunk_html_file_name + 1,
                     "chunk_%d_%d", index_sheet, index_chunk);
            int len_resource_url, len_chunk_html_url;
            char *CHUNK_HTML_URL;
            if (strstr(RESOURCE_URL, "http") != NULL) {
              len_resource_url = XML_Char_len(RESOURCE_URL);
              int len_output_file_name = XML_Char_len(OUTPUT_FILE_NAME);
              len_chunk_html_url = len_chunk_html_file_name + len_resource_url +
                                   len_output_file_name + 9;
              CHUNK_HTML_URL = (char *)XML_Char_malloc(len_chunk_html_url + 1);
              snprintf(CHUNK_HTML_URL, len_chunk_html_url + 1,
                       "%s/chunks/%s.chunk", RESOURCE_URL,
                       CHUNK_HTML_FILE_NAME);
            } else {
              len_chunk_html_url =
                  len_chunk_html_file_name + len_chunks_dir_path + 7;
              CHUNK_HTML_URL = (char *)XML_Char_malloc(len_chunk_html_url + 1);
              snprintf(CHUNK_HTML_URL, len_chunk_html_url + 1, "%s/%s.chunk",
                       CHUNKS_DIR_PATH, CHUNK_HTML_FILE_NAME);
            }
            int len_div_chunk =
                len_chunk_html_file_name + len_chunk_html_url + 35;
            char *DIV_CHUNK = (char *)XML_Char_malloc(len_div_chunk + 1);
            snprintf(DIV_CHUNK, len_div_chunk + 1,
                     "<div id=\"%s\" data-chunk-url=\"%s\"></div>",
                     CHUNK_HTML_FILE_NAME, CHUNK_HTML_URL);
            free(CHUNK_HTML_URL);
            fputs(DIV_CHUNK, findexhtml);
            free(DIV_CHUNK);
            fputs("\n", findexhtml);
            free(CHUNK_HTML_FILE_NAME);
          }
          fputs("</div>", findexhtml);
          fputs("\n", findexhtml);
        }
      } else if (XML_Char_icmp(line, "$buttons\n") == 0) {
        //<button id="btn-Form Responses 1">Form Responses 1</button>
        for (int index_sheet = 0; index_sheet < array_sheets.length;
             index_sheet++) {
          char *BUTTON_HTML = NULL;
          if (index_sheet == 0) {
            int len_button_html =
                1 + XML_Char_len(array_sheets.sheets[index_sheet]->name) + 87;
            BUTTON_HTML =
                (char *)XML_Char_realloc(BUTTON_HTML, len_button_html + 1);
            snprintf(BUTTON_HTML, len_button_html + 1,
                     "<button id=\"btn_%d\" "
                     "style=\"font-weight:bold;\"onclick=\"handleButtonClick("
                     "event)\">%s</button>",
                     index_sheet, array_sheets.sheets[index_sheet]->name);
          } else {
            int len_index_sheet = snprintf(NULL, 0, "%d", index_sheet);
            int len_button_html =
                len_index_sheet +
                XML_Char_len(array_sheets.sheets[index_sheet]->name) + 62;
            BUTTON_HTML =
                (char *)XML_Char_realloc(BUTTON_HTML, len_button_html + 1);
            snprintf(BUTTON_HTML, len_button_html + 1,
                     "<button id=\"btn_%d\" "
                     "onclick=\"handleButtonClick(event)\">%s</button>",
                     index_sheet, array_sheets.sheets[index_sheet]->name);
          }
          fputs(BUTTON_HTML, findexhtml);
          free(BUTTON_HTML);
          fputs("\n", findexhtml);
          free(array_sheets.sheets[index_sheet]->name);
          free(array_sheets.sheets[index_sheet]->sheetId);
          free(array_sheets.sheets[index_sheet]->path_name);
          free(array_sheets.sheets[index_sheet]);
        }
        free(array_sheets.sheets);
      }
    } else {
      // Insert html statement
      fputs(line, findexhtml);
    }
  }
  fclose(fmanifest);
  fclose(findexhtml);
  free(MANIFEST_PATH);
  free(TEMPLATES_DIR_PATH);
  free(BASE_CSS_PATH);
  free(BASE_JS_PATH);
  free(INDEX_HTML_PATH);
}

void post_process() {
  // TODO: To handle remove redunant rows, columns, condition formating
}

int main(int argc, char **argv) {
  debug_print("%s\n", XLSXMAGIC_FULLNAME);
  int c;
  int digit_optind = 0;
  char has_origin_file_path = '0';
  char has_output_dir = '0';
  char has_output_file_name = '0';
  char has_tmp_dir = '0';
  char has_url_resource = '0';

  while (1) {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
        {"input", required_argument, 0, 0},
        {"output", required_argument, 0, 0},
        {"output-file-name", required_argument, 0, 0},
        {"tmp-dir", required_argument, 0, 0},
        {"url-resource", required_argument, 0, 0},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}};

    c = getopt_long(argc, argv, "hv", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0:
      printf("option %s", long_options[option_index].name);
      if (strcmp(long_options[option_index].name, "input") == 0) {
        if (optarg) {
          ORIGIN_FILE_PATH = strdup(optarg);
          has_origin_file_path = '1';
          printf(" with arg %s", optarg);
        }
      } else if (strcmp(long_options[option_index].name, "output") == 0) {
        if (optarg) {
          OUTPUT_DIR = strdup(optarg);
          has_output_dir = '1';
          printf(" with arg %s", optarg);
        }
      } else if (strcmp(long_options[option_index].name, "output-file-name") ==
                 0) {
        if (optarg) {
          OUTPUT_FILE_NAME = strdup(optarg);
          has_output_file_name = '1';
          printf(" with arg %s", optarg);
        }
      } else if (strcmp(long_options[option_index].name, "tmp-dir") == 0) {
        if (optarg) {
          TEMP_DIR = strdup(optarg);
          has_tmp_dir = '1';
          printf(" with arg %s", optarg);
        }
      } else if (strcmp(long_options[option_index].name, "url-resource") == 0) {
        if (optarg) {
          RESOURCE_URL = strdup(optarg);
          has_url_resource = '1';
          printf(" with arg %s", optarg);
        }
      }
      printf("\n");
      break;

    case '0':
    case '1':
    case '2':
      if (digit_optind != 0 && digit_optind != this_option_optind)
        printf("digits occur in two different argv-elements.\n");
      digit_optind = this_option_optind;
      printf("option %c\n", c);
      exit(EXIT_SUCCESS);

    case 'h':
    case '?':
      printf("%s\n", XLSXMAGIC_FULLNAME);
      printf("%s%40s\n", "--input", "Path to xlsx file");
      printf("%s%46s\n", "--output", "Path to output directory");
      printf("%s%32s\n", "--output-file-name", "Default (index.html)");
      printf("%s%42s\n", "--tmp-dir", "Path to temporary dir");
      printf("%s%64s\n", "--url-resource",
             "Url to resource (image,etc) follow by enviroment");
      printf("%s%43s\n", "-h, --help", "Print usage information");
      printf("%s%37s\n", "-v, --version", "Show current version");
      exit(EXIT_SUCCESS);
    case 'v':
      printf("%s\n", XLSXMAGIC_FULLNAME);
      exit(EXIT_SUCCESS);

    default:
      printf("?? getopt returned character code 0%o ??\n", c);
      exit(EXIT_SUCCESS);
    }
  }

  if (optind < argc) {
    printf("non-option ARGV-elements: ");
    while (optind < argc)
      printf("%s ", argv[optind++]);
    printf("\n");

    goto OPEN_ZIP_FAILED;
  }
  // Set default to test on local
  if (has_origin_file_path == '0') {
    ORIGIN_FILE_PATH = "/home/huydang/Downloads/excelsample/"
                       "Project_Management__codestringers.xlsx";
  }
  if (has_output_file_name == '0') {
    OUTPUT_FILE_NAME = "index";
  }
  char *path;
  int length = wai_getExecutablePath(NULL, 0, NULL);
  int dirname_length;
  path = (char *)malloc(length + 1);
  wai_getExecutablePath(path, length, &dirname_length);
  // Get dir name
  path[dirname_length] = '\0';
  WORKING_DIR = strdup(path);
  free(path);

  if (has_output_dir == '0') {
    char *OUTPUT_DIR_NAME = "output";
    int len_tmp_output_dir =
        XML_Char_len(WORKING_DIR) + XML_Char_len(OUTPUT_DIR_NAME) + 1;
    char *_tmp_output_dir = (char *)XML_Char_malloc(len_tmp_output_dir + 1);
    snprintf(_tmp_output_dir, len_tmp_output_dir + 1, "%s/%s", WORKING_DIR,
             OUTPUT_DIR_NAME);
    OUTPUT_DIR = strdup(_tmp_output_dir);
    free(_tmp_output_dir);
  }
  if (has_tmp_dir == '0') {
    TEMP_DIR = "/tmp";
  }
  if (has_url_resource == '0') {
    RESOURCE_URL = strdup(OUTPUT_DIR);
  }

  zip_t *zip = open_zip(ORIGIN_FILE_PATH);
  if (zip == NULL) {
    debug_print("%s: %s\n", strerror(errno), ORIGIN_FILE_PATH);
    goto LOAD_RESOURCES_FAILED;
  }

  // +1 for "/" +1 for '\0'
  struct stat st = {0};
  if (stat(OUTPUT_DIR, &st) == -1) {
    int status = mkdir_p(OUTPUT_DIR, 0755);
    if (status != 0) {
      debug_print("%s: %s\n", strerror(errno), OUTPUT_DIR);
      goto LOAD_RESOURCES_FAILED;
    }
  }
  int len_chunks_dir_path =
      XML_Char_len(OUTPUT_DIR) + XML_Char_len(CHUNKS_DIR_NAME) + 1;
  char *_tmp_chunks_dir_path = (char *)XML_Char_malloc(len_chunks_dir_path + 1);
  if (_tmp_chunks_dir_path == NULL) {
    debug_print("%s\n", strerror(errno));
  }
  snprintf(_tmp_chunks_dir_path, len_chunks_dir_path + 1, "%s/%s", OUTPUT_DIR,
           CHUNKS_DIR_NAME);
  CHUNKS_DIR_PATH = strdup(_tmp_chunks_dir_path);
  free(_tmp_chunks_dir_path);
  if (stat(CHUNKS_DIR_PATH, &st) == -1) {
    int status = mkdir_p(CHUNKS_DIR_PATH, 0755);
    if (status != 0) {
      debug_print("%s: %s\n", strerror(errno), CHUNKS_DIR_PATH);
      goto LOAD_RESOURCES_FAILED;
    }
  }
  int status_workbook = load_workbook(zip);
  if (status_workbook != 1) {
    debug_print("%s\n", strerror(errno));
    goto LOAD_RESOURCES_FAILED;
  }
  int status_styles = load_styles(zip);
  if (status_styles != 1) {
    debug_print("%s\n", strerror(errno));
    destroy_workbook();
    goto LOAD_RESOURCES_FAILED;
  }
  int status_sharedStrings = load_sharedStrings(zip);
  if (status_sharedStrings != 1) {
    debug_print("WARNING: load_sharedStrings with status is %d\n",
                status_sharedStrings);
  }
  int status_worksheets = load_worksheets(zip);
  if (status_worksheets != 1) {
    debug_print("%s\n", strerror(errno));
    destroy_styles();
    destroy_workbook();
    goto LOAD_RESOURCES_FAILED;
  }
  destroy_styles();
  pre_process(zip);
  free((char *)WORKING_DIR);

LOAD_RESOURCES_FAILED:
  zip_close(zip);
OPEN_ZIP_FAILED:
  if (has_origin_file_path == '1')
    free((char *)ORIGIN_FILE_PATH);
  if (has_output_file_name == '1')
    free((char *)OUTPUT_FILE_NAME);
  if (has_tmp_dir == '1')
    free((char *)TEMP_DIR);
  free((char *)RESOURCE_URL);
  free((char *)CHUNKS_DIR_PATH);
  free((char *)OUTPUT_DIR);
  int status_clean_ss_data = clean_ss_data(SHAREDSTRINGS_HTML_FILE_PATH);
  if (status_clean_ss_data != 0)
    debug_print("%s\n", strerror(errno));
  free((char *)SHAREDSTRINGS_HTML_FILE_PATH);
  exit(EXIT_SUCCESS);
}
