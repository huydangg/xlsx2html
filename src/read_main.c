#include <read_main.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <unistd.h>
#include <limits.h>


XML_Parser xmlparser;
const char *ORIGIN_FILE_PATH;
const char *OUTPUT_DIR;
const char *OUTPUT_FILE_NAME;
const char *TEMP_DIR;


zip_t *open_zip(const char *file_name) {
  return zip_open(file_name, ZIP_RDONLY, NULL);
}

zip_file_t *open_zip_file(zip_t *zip, const char *zip_file_name) {
  return zip_fopen(zip, zip_file_name, ZIP_FL_UNCHANGED);
}

int load_workbook(zip_t *zip) { const char *zip_file_name = "xl/workbook.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  int status = process_zip_file(archive, NULL, NULL, workbook_start_element, workbook_end_element);
  for(int i = 0; i < array_sheets.length; i++) {
    printf("Name %s\n", array_sheets.sheets[i]->name);
    printf("sheetID: %s\n", array_sheets.sheets[i]->sheetId);
    printf("Path name: %s\n", array_sheets.sheets[i]->path_name);
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
  // Load NumFMT first
  int status = process_zip_file(archive, NULL, NULL, styles_start_element, styles_end_element);
  for (int i = 0; i < array_numfmts.length; i++) {
    printf("Format code: %s\n", array_numfmts.numfmts[i].formatCode);
    printf("Format id: %s\n", array_numfmts.numfmts[i].numFmtId);
  }
  printf("Count font: %d\n", array_fonts.length);
  for (int i = 0; i < array_fonts.length; i++) {
    printf("Font size: %f\n", array_fonts.fonts[i].sz);
    printf("Font name: %s\n", array_fonts.fonts[i].name);
    printf("Font is bold: %c\n", array_fonts.fonts[i].isBold);
    printf("Font is italic: %c\n", array_fonts.fonts[i].isItalic);
    printf("Font underline: %s\n", array_fonts.fonts[i].underline);
    printf("Font color rgb: %s\n", array_fonts.fonts[i].color.rgb);
  }
  printf("Count fills: %d\n", array_fills.length);
  for (int i = 0; i < array_fills.length; i++) {
    printf("Fill pattern type: %s\n", array_fills.fills[i].patternFill.patternType);
    printf("Fill bg_color rgb: %s\n", array_fills.fills[i].patternFill.bgColor.rgb);
    printf("Fill fg_color rgb: %s\n", array_fills.fills[i].patternFill.fgColor.rgb);
  }
  printf("Count border: %d\n", array_borders.length);
  for (int i = 0; i < array_borders.length; i++) {
    printf("---------------------------------------------------------\n");
    printf("Border left style: %s\n", array_borders.borders[i].left.style);
    printf("Border left color rgb: %s\n", array_borders.borders[i].left.color.rgb);
    printf("Border right style: %s\n", array_borders.borders[i].right.style);
    printf("Border right color rgb: %s\n", array_borders.borders[i].right.color.rgb);
    printf("Border top style: %s\n", array_borders.borders[i].top.style);
    printf("Border top color rgb: %s\n", array_borders.borders[i].top.color.rgb);
    printf("Border bottom style: %s\n", array_borders.borders[i].bottom.style);
    printf("Border bottom color rgb: %s\n", array_borders.borders[i].bottom.color.rgb);
  }
  for (int i = 0; i < array_cellStyleXfs.length; i++) {
    printf("---------------------------------------------------------\n");
    printf("Xf borderId: %d\n", array_cellStyleXfs.Xfs[i].borderId);
    printf("Xf fillId: %d\n", array_cellStyleXfs.Xfs[i].fillId);
    printf("Xf fontId: %d\n", array_cellStyleXfs.Xfs[i].fontId);
    printf("Xf numFmtId: %d\n", array_cellStyleXfs.Xfs[i].numFmtId);
    printf("Xf alignment horizontal: %s\n", array_cellStyleXfs.Xfs[i].alignment.horizontal);
    printf("Xf alignment vertical: %s\n", array_cellStyleXfs.Xfs[i].alignment.vertical);
    printf("Xf alignment textRotation: %s\n", array_cellStyleXfs.Xfs[i].alignment.textRotation);
    printf("Xf alignment isWrapText: %c\n", array_cellStyleXfs.Xfs[i].alignment.isWrapText);
  }
  printf("Count cellXfs: %d\n", array_cellXfs.length);
  for (int i = 0; i < array_cellXfs.length; i++) {
    printf("---------------------------------------------------------\n");
    printf("Xf borderId: %d\n", array_cellXfs.Xfs[i].borderId);
    printf("Xf fillId: %d\n", array_cellXfs.Xfs[i].fillId);
    printf("Xf fontId: %d\n", array_cellXfs.Xfs[i].fontId);
    printf("Xf numFmtId: %d\n", array_cellXfs.Xfs[i].numFmtId);
    printf("Xf xfId: %d\n", array_cellXfs.Xfs[i].xfId);
    printf("Xf alignment horizontal: %s\n", array_cellXfs.Xfs[i].alignment.horizontal);
    printf("Xf alignment vertical: %s\n", array_cellXfs.Xfs[i].alignment.vertical);
    printf("Xf alignment textRotation: %s\n", array_cellXfs.Xfs[i].alignment.textRotation);
    printf("Xf alignment isWrapText: %c\n", array_cellXfs.Xfs[i].alignment.isWrapText);
  }
  return status;
}

int load_worksheets(zip_t *zip) {
  printf("Length sheets: %d\n", array_sheets.length);
  for(int i = 0; i < array_sheets.length; i++) {
    printf("---------------------------------------------------------\n");
    printf("Loading %s\n", array_sheets.sheets[i]->path_name);
    zip_file_t *archive = open_zip_file(zip, array_sheets.sheets[i]->path_name);
    struct WorkSheet worksheet;
    worksheet.start_col = 'A';
    worksheet.start_row = '1';
    worksheet.index_sheet = i;
    worksheet.hasMergedCells = '0';
    int status_worksheet = process_zip_file(archive, &worksheet, NULL, worksheet_start_element, worksheet_end_element);
    if (!status_worksheet){
      return status_worksheet;
    }
    array_sheets.sheets[i]->hasMergedCells = worksheet.hasMergedCells;
    printf("HAS MERGED CELLS: %c\n", worksheet.hasMergedCells);
    printf("START_ROW: %c\n", worksheet.start_row);
    printf("START_COL: %c\n", worksheet.start_col);
    printf("END_ROW: %s\n", worksheet.end_row);
    printf("END_COL: %s\n", worksheet.end_col);
    printf("END_COL_IN_NUMBER: %d\n", worksheet.end_col_number);
    printf("Length cols: %d\n", worksheet.array_cols.length);

    free(worksheet.end_row);
    free(worksheet.end_col);
  }
  free(sharedStrings_position.positions);
  return 1;
}

int load_sharedStrings(zip_t *zip) {
  const char *file_name = "xl/sharedStrings.xml";
  int len_sharedStrings_html_file_name = strlen(OUTPUT_FILE_NAME) + strlen(SHAREDSTRINGS_HTML_FILE_PATTERN);
  char *SHAREDSTRINGS_HTML_FILE_NAME = malloc(len_sharedStrings_html_file_name + 1);
  snprintf(SHAREDSTRINGS_HTML_FILE_NAME, len_sharedStrings_html_file_name + 1, "%s%s", OUTPUT_FILE_NAME, SHAREDSTRINGS_HTML_FILE_PATTERN);
  int len_sharedStrings_file_path = strlen(TEMP_DIR) + 1 + len_sharedStrings_html_file_name;
  char *SHAREDSTRINGS_HTML_FILE_PATH = malloc(len_sharedStrings_file_path + 1);
  snprintf(SHAREDSTRINGS_HTML_FILE_PATH, len_sharedStrings_file_path + 1, "%s/%s", TEMP_DIR, SHAREDSTRINGS_HTML_FILE_NAME);
  free(SHAREDSTRINGS_HTML_FILE_NAME);
  FILE *sharedStrings_file;
  sharedStrings_file = fopen(SHAREDSTRINGS_HTML_FILE_PATH, "wb+");
  if (sharedStrings_file == NULL) {
    fprintf(stderr, "Cannot open %s to write\n", SHAREDSTRINGS_HTML_FILE_PATH);
    free(SHAREDSTRINGS_HTML_FILE_PATH);
    return -1;
  }
  zip_file_t *archive = open_zip_file(zip, file_name);
  int status_sharedStrings = process_zip_file(archive, sharedStrings_file, NULL, sharedStrings_main_start_element, sharedStrings_main_end_element);
  if (status_sharedStrings == -1) {
    fprintf(stderr, "Error when load sharedStrings\n");
    fclose(sharedStrings_file);
    free(SHAREDSTRINGS_HTML_FILE_PATH);
    return -1;
  }
  free(SHAREDSTRINGS_HTML_FILE_PATH);
  fclose(sharedStrings_file);
  return 1;
}

int process_zip_file(zip_file_t *archive, void *callbackdata, XML_CharacterDataHandler content_handler, XML_StartElementHandler start_element, XML_EndElementHandler end_element) {
  void *buf;
  zip_int64_t buflen;
  xmlparser = XML_ParserCreate(NULL);
  int done;
  enum XML_Status status = XML_STATUS_ERROR;
  XML_SetUserData(xmlparser, callbackdata);
  XML_SetElementHandler(xmlparser, start_element, end_element);
  XML_SetCharacterDataHandler(xmlparser, content_handler);
  buf = XML_GetBuffer(xmlparser, PARSE_BUFFER_SIZE);
  while(buf && (buflen = zip_fread(archive, buf, PARSE_BUFFER_SIZE)) >= 0) {
    done = buflen < PARSE_BUFFER_SIZE;
    if((status = XML_ParseBuffer(xmlparser, (int)buflen, (done ? 1 : 0))) == XML_STATUS_ERROR) {
      fprintf(stderr, "%" XML_FMT_STR " at line %" XML_FMT_INT_MOD "u\n",
              XML_ErrorString(XML_GetErrorCode(xmlparser)),
              XML_GetCurrentLineNumber(xmlparser));
       XML_ParserFree(xmlparser); 
       zip_fclose(archive);
       return 0;
    }
    if(done) {
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
    fprintf(stderr, "Cannot open css file to read");
  }
  char line[256];
  while (fgets(line, sizeof(line), fcss)) {
    fputs(line, f);
  }
  fclose(fcss);
}

void embed_js(FILE *f, const char *js_path) {
  const char *_js_path = js_path;
  FILE *fjs;
  fjs = fopen(_js_path, "rb");
  if (fjs == NULL) {
    fprintf(stderr, "Cannot open js file to read");
  }
  char line[256];
  while (fgets(line, sizeof(line), fjs)) {
    fputs(line, f);
  }
  fclose(fjs);
}

// Generate index html file
void pre_process() {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf("Current working dir: %s\n", cwd);
  } else {
      perror("getcwd() error");
      return;
  }
  int len_templates_dir_path = strlen(cwd) + strlen(TEMPLATES_DIR_NAME) + 1;
  char *TEMPLATES_DIR_PATH = malloc(len_templates_dir_path + 1);
  snprintf(TEMPLATES_DIR_PATH, len_templates_dir_path + 1, "%s/%s", cwd, TEMPLATES_DIR_NAME);
  int len_base_css_path = len_templates_dir_path + strlen(BASE_CSS_FILE_NAME) + 1;
  char *BASE_CSS_PATH = malloc(len_base_css_path + 1);
  snprintf(BASE_CSS_PATH, len_base_css_path + 1, "%s/%s", TEMPLATES_DIR_PATH, BASE_CSS_FILE_NAME);
  int len_base_js_path = len_templates_dir_path + strlen(BASE_JS_FILE_NAME) + 1; 
  char *BASE_JS_PATH = malloc(len_base_js_path + 1);
  snprintf(BASE_JS_PATH, len_base_js_path + 1, "%s/%s", TEMPLATES_DIR_PATH, BASE_JS_FILE_NAME);
  int len_manifest_path = len_templates_dir_path + strlen(MANIFEST_FILE_NAME) + 1;
  char *MANIFEST_PATH = malloc(len_manifest_path + 1);
  snprintf(MANIFEST_PATH, len_manifest_path + 1, "%s/%s", TEMPLATES_DIR_PATH, MANIFEST_FILE_NAME);
  //5: .html
  int len_index_html_path = strlen(OUTPUT_DIR) + strlen(OUTPUT_FILE_NAME) + 5 + 1;
  char *INDEX_HTML_PATH = malloc(len_index_html_path + 1);
  snprintf(INDEX_HTML_PATH, len_index_html_path + 1, "%s/%s.html", OUTPUT_DIR, OUTPUT_FILE_NAME);
  FILE *fmanifest;
  fmanifest = fopen(MANIFEST_PATH, "rb");
  if (fmanifest == NULL) {
    fprintf(stderr, "Cannot open manifest file to read");
    return;
  }
  FILE *findexhtml;
  findexhtml = fopen(INDEX_HTML_PATH, "ab+");
  if (findexhtml == NULL) {
    fprintf(stderr, "Cannot open index html file to read\n");
    return;
  }
  int len_chunks_dir_path = strlen(OUTPUT_DIR) + strlen(CHUNKS_DIR_NAME) + 1;
  char *CHUNKS_DIR_PATH = malloc(len_chunks_dir_path + 1);
  snprintf(CHUNKS_DIR_PATH, len_chunks_dir_path + 1, "%s/%s", OUTPUT_DIR, CHUNKS_DIR_NAME);

  char line[256];
  while(fgets(line, sizeof(line), fmanifest)) {
    if (line[0] == '\n') {
      continue;
    } else if (line[0] == '#') {
      continue;
    } else if (line[0] == '@') {
      if (strcmp(line, "@base.min.css\n") == 0) {
	fputs("<style>", findexhtml);
        embed_css(findexhtml, BASE_CSS_PATH);
	fputs("</style>", findexhtml);
      } else if (strcmp(line, "@xlsxmagic.min.js\n") == 0) {
	fputs("<script>", findexhtml);
        embed_js(findexhtml, BASE_JS_PATH);
	fputs("</script>", findexhtml);
      }
    } else if (line[0] == '$') {
      if (strcmp(line, "$tables\n") == 0) {
	for (int index_sheet = 0; index_sheet < array_sheets.length; index_sheet++) {
	  int len_index_sheet = snprintf(NULL, 0, "%d", index_sheet);
	  char div_table[256]; // Warning: Need to allocte dynamic
	  snprintf(div_table, sizeof(div_table), "<div id=\"sheet_%d\" name=\"%s\" style=\"position:relative;overflow:auto;width:100%%;height:95vh;display:none;\">", index_sheet, array_sheets.sheets[index_sheet]->name);
          fputs(div_table, findexhtml);
	  fputs("\n", findexhtml);
          char div_thead[256]; // Warning: Need to allocte dynamic
	  for (int index_chunk = 0; index_chunk < 2; index_chunk++) {
	    //7: chunk_%d_%d
	    int len_chunk_html_file_name = snprintf(NULL, 0, "%d", index_chunk) + len_index_sheet + 7;
	    char *CHUNK_HTML_FILE_NAME = malloc(len_chunk_html_file_name + 1);
	    snprintf(CHUNK_HTML_FILE_NAME, len_chunk_html_file_name + 1, "chunk_%d_%d", index_sheet, index_chunk);
            snprintf(div_thead, sizeof(div_thead), "<div id=\"%s\" data-chunk-url=\"file://%s/%s.html\"></div>", CHUNK_HTML_FILE_NAME, CHUNKS_DIR_PATH, CHUNK_HTML_FILE_NAME);
	    fputs(div_thead, findexhtml);
	    fputs("\n", findexhtml);
	    free(CHUNK_HTML_FILE_NAME);
	  }

          if (array_sheets.sheets[index_sheet]->hasMergedCells == '1') {
	    int len_chunk_mc_file_name = len_index_sheet + 9;
	    char *CHUNK_MC_FILE_NAME = malloc(len_chunk_mc_file_name + 1);
	    snprintf(CHUNK_MC_FILE_NAME, len_chunk_mc_file_name + 1, "chunk_%d_mc", index_sheet);
            snprintf(div_thead, sizeof(div_thead), "<div id=\"%s\" data-chunk-url=\"file://%s/%s.json\"></div>", CHUNK_MC_FILE_NAME, CHUNKS_DIR_PATH, CHUNK_MC_FILE_NAME);
            fputs(div_thead, findexhtml);
	    fputs("\n", findexhtml);
	    free(CHUNK_MC_FILE_NAME);
          }
	  fputs("</div>", findexhtml);
	  fputs("\n", findexhtml);
        }
      } else if (strcmp(line, "$buttons\n") == 0) {
        //<button id="btn-Form Responses 1">Form Responses 1</button>
	for (int index_sheet = 0; index_sheet < array_sheets.length; index_sheet++) {
	  char button_html[256];
	  if (index_sheet == 0) {
	    snprintf(button_html, sizeof(button_html), "<button id=\"btn_%d\" style=\"font-weight:bold;\"onclick=\"handleButtonClick(event)\">%s</button>", index_sheet, array_sheets.sheets[index_sheet]->name);
	  } else {
	    snprintf(button_html, sizeof(button_html), "<button id=\"btn_%d\" onclick=\"handleButtonClick(event)\">%s</button>", index_sheet, array_sheets.sheets[index_sheet]->name);
	  }
	  fputs(button_html, findexhtml);
	  fputs("\n", findexhtml);
          free(array_sheets.sheets[index_sheet]->name);
          free(array_sheets.sheets[index_sheet]->sheetId);
          free(array_sheets.sheets[index_sheet]->path_name);
          free(array_sheets.sheets[index_sheet]);
        }
        free(array_sheets.sheets);
      }
     } else {
      //Insert html statement
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
  free(CHUNKS_DIR_PATH);
}

void post_process() {
  //TODO: To handle remove redunant rows, columns, condition formating
}

int main(int argc, char **argv) {
  int c;
  int digit_optind = 0;
  while (1) {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
         {"origin-file-path", required_argument, 0, 0},
         {"output-dir",  required_argument, 0, 0},
         {"output-file-name", required_argument, 0, 0},
         {"tmp-dir", required_argument, 0, 0},
         {"help", no_argument, 0, 'h'},
         {0, 0, 0, 0}
    };

    c = getopt_long(argc, argv, "h",
             long_options, &option_index);
    if (c == -1)
       break;

    switch (c) {
      case 0:
        printf("option %s", long_options[option_index].name);
	if (strcmp(long_options[option_index].name, "origin-file-path") == 0) {
	  if (optarg) {
	    ORIGIN_FILE_PATH = strdup(optarg);
            printf(" with arg %s", optarg);
	  }
	} else if (strcmp(long_options[option_index].name, "output-dir") == 0) {
	  if (optarg) {
	    OUTPUT_DIR = strdup(optarg);
            printf(" with arg %s", optarg);
	  }
	} else if (strcmp(long_options[option_index].name, "output-file-name") == 0) {
	  if (optarg) {
	    OUTPUT_FILE_NAME = strdup(optarg);
            printf(" with arg %s", optarg);
	  }
	} else if (strcmp(long_options[option_index].name, "tmp-dir") == 0) {
	  if (optarg) {
	    TEMP_DIR = strdup(optarg);
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
        break;

      case 'h':
      case '?':
	printf("%s%50s\n", "--origin-file-path", "Path to xlsx file to be converted");
	printf("%s%40s\n", "--output-dir", "Path to ouput dir");
	printf("%s%58s\n", "--output-file-name", "File index html (include .html extension)");
	printf("%s%42s\n", "--tmp-dir", "Path to temp dir");
	printf("%s%48s\n", "-h, --help", "Print usage information");
        break;

      default:
        printf("?? getopt returned character code 0%o ??\n", c);
    }
  }

  if (optind < argc) {
    printf("non-option ARGV-elements: ");
    while (optind < argc)
      printf("%s ", argv[optind++]);
    printf("\n");
   }

  zip_t *zip = open_zip(ORIGIN_FILE_PATH);
  if (zip == NULL){
    fprintf(stderr, "File not found");
    return 0;
  }
  // +1 for "/" +1 for '\0'
  struct stat st = {0};
  if (stat(OUTPUT_DIR, &st) == -1) {
    int status = mkdir(OUTPUT_DIR, 0777);
    if (status != 0) {
      fprintf(stderr, "Error when create a output dir with status is %d\n", status);
      return -1;
    }
  }
  int status_workbook = load_workbook(zip);
  if (!status_workbook) {
    fprintf(stderr, "Failed to read workbook");
    zip_close(zip);
    return 0;
  }
  int status_styles = load_styles(zip);
  if (!status_styles) {
    fprintf(stderr, "Failed to read styles");
    zip_close(zip);
    return 0;
  }
  int status_sharedStrings = load_sharedStrings(zip);
  if (!status_sharedStrings) {
    fprintf(stderr, "Failed to read sharedStrings");
    zip_close(zip);
    return 0;
  }
  int status_worksheets = load_worksheets(zip);
  if (!status_worksheets) {
    fprintf(stderr, "Failed to read worksheets");
    zip_close(zip);
    return 0;
  }
  destroy_styles();
  pre_process();
  free((char *)OUTPUT_DIR);
  free((char *)ORIGIN_FILE_PATH);
  free((char *)OUTPUT_FILE_NAME);
  free((char *)TEMP_DIR);
  zip_close(zip);
  return 0; 
}
