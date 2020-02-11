#include <read_main.h>
#include <read_styles.h>


/*int load_contenttype(zip_t *zip){
  const char *zip_file_name = "[Content_Types].xml";
  int status = process_zip_file(zip, zip_file_name);
  return status;
}*/

zip_t *open_zip(const char *file_name) {
  return zip_open(file_name, ZIP_RDONLY, NULL);
}

zip_file_t *open_zip_file(zip_t *zip, const char *zip_file_name) {
  return zip_fopen(zip, zip_file_name, ZIP_FL_UNCHANGED);
}

int load_workbook(zip_t *zip) {
  const char *zip_file_name = "xl/workbook.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  int status = process_zip_file(archive, NULL, styles_start_element, styles_end_element);
  for(int i = 0; i < array_sheets.length; i++) {
    printf("Name %s\n", array_sheets.sheets[i]->name);
    printf("sheetID: %s\n", array_sheets.sheets[i]->sheet_id);
    printf("Path name: %s\n", array_sheets.sheets[i]->path_name);
    free(array_sheets.sheets[i]->name);
    free(array_sheets.sheets[i]->sheet_id);
    free(array_sheets.sheets[i]->path_name);
    free(array_sheets.sheets[i]);
  }
  free(array_sheets.sheets);
  return status;
}

int load_styles(zip_t *zip) {
  const char *zip_file_name = "xl/styles.xml";
  zip_file_t *archive = open_zip_file(zip, zip_file_name);
  // Load NumFMT first
  int status = process_zip_file(archive, NULL, styles_start_element, styles_end_element);
  for (int i = 0; i < array_numfmts.length; i++) {
    printf("Format code: %s\n", array_numfmts.numfmts[i].format_code);
    printf("Format id: %s\n", array_numfmts.numfmts[i].format_id);
    free(array_numfmts.numfmts[i].format_code);
    free(array_numfmts.numfmts[i].format_id);
  }
  free(array_numfmts.numfmts);
  printf("Count font: %d\n", array_fonts.length);
  for (int i = 0; i < array_fonts.length; i++) {
    printf("Font size: %d\n", array_fonts.fonts[i].size);
    printf("Font name: %s\n", array_fonts.fonts[i].name);
    printf("Font is bold: %d\n", array_fonts.fonts[i].is_bold);
    printf("Font is italic: %d\n", array_fonts.fonts[i].is_italic);
    printf("Font underline: %s\n", array_fonts.fonts[i].underline);
    printf("Font color rgb: %s\n", array_fonts.fonts[i].color.rgb);
    free(array_fonts.fonts[i].name);
    free(array_fonts.fonts[i].underline);
    free(array_fonts.fonts[i].color.rgb);
  }
  free(array_fonts.fonts);
  printf("Count fills: %d\n", array_fills.length);
  for (int i = 0; i < array_fills.length; i++) {
    printf("Fill pattern type: %s\n", array_fills.fills[i].pattern_fill.pattern_type);
    printf("Fill bg_color rgb: %s\n", array_fills.fills[i].pattern_fill.bg_color.rgb);
    printf("Fill fg_color rgb: %s\n", array_fills.fills[i].pattern_fill.fg_color.rgb);
    free(array_fills.fills[i].pattern_fill.pattern_type);
    free(array_fills.fills[i].pattern_fill.bg_color.rgb);
    free(array_fills.fills[i].pattern_fill.fg_color.rgb);
  }
  free(array_fills.fills);
  printf("Count border: %d\n", array_borders.length);
  for (int i = 0; i < array_borders.length; i++) {
    printf("---------------------------------------------------------\n");
    printf("Border left style: %s\n", array_borders.borders[i].left.style);
    printf("Border left color rgb: %s\n", array_borders.borders[i].left.border_color.rgb);
    printf("Border right style: %s\n", array_borders.borders[i].right.style);
    printf("Border right color rgb: %s\n", array_borders.borders[i].right.border_color.rgb);
    printf("Border top style: %s\n", array_borders.borders[i].top.style);
    printf("Border top color rgb: %s\n", array_borders.borders[i].top.border_color.rgb);
    printf("Border bottom style: %s\n", array_borders.borders[i].bottom.style);
    printf("Border bottom color rgb: %s\n", array_borders.borders[i].bottom.border_color.rgb);
    free(array_borders.borders[i].left.style);
    free(array_borders.borders[i].left.border_color.rgb);
    free(array_borders.borders[i].right.style);
    free(array_borders.borders[i].right.border_color.rgb);
    free(array_borders.borders[i].top.style);
    free(array_borders.borders[i].top.border_color.rgb);
    free(array_borders.borders[i].bottom.style);
    free(array_borders.borders[i].bottom.border_color.rgb);
  }
  free(array_borders.borders);
  for (int i = 0; i < array_cellStyleXfs.length; i++) {
    printf("---------------------------------------------------------\n");
    printf("Xf borderId: %s\n", array_cellStyleXfs.Xfs[i].borderId);
    printf("Xf fillId: %s\n", array_cellStyleXfs.Xfs[i].fillId);
    printf("Xf fontId: %s\n", array_cellStyleXfs.Xfs[i].fontId);
    printf("Xf numFmtId: %s\n", array_cellStyleXfs.Xfs[i].numFmtId);
    printf("Xf alignment horizontal: %s\n", array_cellStyleXfs.Xfs[i].alignment.horizontal);
    printf("Xf alignment vertical: %s\n", array_cellStyleXfs.Xfs[i].alignment.vertical);
    printf("Xf alignment textRotation: %s\n", array_cellStyleXfs.Xfs[i].alignment.textRotation);
    printf("Xf alignment isWrapText: %c\n", array_cellStyleXfs.Xfs[i].alignment.isWrapText);
    free(array_cellStyleXfs.Xfs[i].borderId);
    free(array_cellStyleXfs.Xfs[i].fillId);
    free(array_cellStyleXfs.Xfs[i].fontId);
    free(array_cellStyleXfs.Xfs[i].numFmtId);
    free(array_cellStyleXfs.Xfs[i].alignment.horizontal);
    free(array_cellStyleXfs.Xfs[i].alignment.vertical);
    free(array_cellStyleXfs.Xfs[i].alignment.textRotation);
  }
  free(array_cellStyleXfs.Xfs);
  printf("Count cellXfs: %d\n", array_cellXfs.length);
  for (int i = 0; i < array_cellXfs.length; i++) {
    printf("---------------------------------------------------------\n");
    printf("Xf borderId: %s\n", array_cellXfs.Xfs[i].borderId);
    printf("Xf fillId: %s\n", array_cellXfs.Xfs[i].fillId);
    printf("Xf fontId: %s\n", array_cellXfs.Xfs[i].fontId);
    printf("Xf numFmtId: %s\n", array_cellXfs.Xfs[i].numFmtId);
    printf("Xf xfId: %s\n", array_cellXfs.Xfs[i].xfId);
    printf("Xf alignment horizontal: %s\n", array_cellXfs.Xfs[i].alignment.horizontal);
    printf("Xf alignment vertical: %s\n", array_cellXfs.Xfs[i].alignment.vertical);
    printf("Xf alignment textRotation: %s\n", array_cellXfs.Xfs[i].alignment.textRotation);
    printf("Xf alignment isWrapText: %c\n", array_cellXfs.Xfs[i].alignment.isWrapText);
    free(array_cellXfs.Xfs[i].borderId);
    free(array_cellXfs.Xfs[i].fillId);
    free(array_cellXfs.Xfs[i].fontId);
    free(array_cellXfs.Xfs[i].numFmtId);
    free(array_cellXfs.Xfs[i].xfId);
    free(array_cellXfs.Xfs[i].alignment.horizontal);
    free(array_cellXfs.Xfs[i].alignment.vertical);
    free(array_cellXfs.Xfs[i].alignment.textRotation);
  }
  free(array_cellXfs.Xfs);
  return status;
}

int load_sheet(zip_t *zip, const char *sheet_file_name) {
  return 1;
}

int process_zip_file(zip_file_t *archive, void *callbackdata, XML_StartElementHandler start_element, XML_EndElementHandler end_element) {
  void *buf;
  zip_int64_t buflen;
  XML_Parser xmlparser = XML_ParserCreate(NULL);
  int done;
  enum XML_Status status = XML_STATUS_ERROR;

  if (callbackdata){
    XML_SetUserData(xmlparser, callbackdata);
  }
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

int main(void) {
  const char *file_name = "/home/huydang/Downloads/excelsample/report__codestringers.xlsx";
  zip_t *zip = open_zip(file_name);
  if (zip == NULL){
    fprintf(stderr, "File not found");
    return 0;
  }
  int status_workbook = load_workbook(zip);
  if (!status_workbook) {
    fprintf(stderr, "Failed to read workbook");
    zip_close(zip);
    return 0;
  }
  int status_styles = load_styles(zip);
  if (!status_styles) {
    fprintf(stderr, "Failed to read workbook");
    zip_close(zip);
    return 0;
  }
  zip_close(zip);
  return 0; 
}
