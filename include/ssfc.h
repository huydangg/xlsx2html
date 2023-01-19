/* ssf.js (C) 2013-present SheetJS -- http://sheetjs.com */
#ifndef INCLUDED_SSFC_H
#define INCLUDED_SSFC_H

#define SSFJS_VERSION 0.11.2
#define p2_32 pow(2, 32)
const char *days[][2] = {{"Sun", "Sunday"},   {"Mon", "Monday"},
                         {"Tue", "Tuesday"},  {"Wed", "Wednesday"},
                         {"Thu", "Thursday"}, {"Fri", "Friday"},
                         {"Sat", "Saturday"}};
const char *months[][3] = {{"J", "Jan", "January"},   {"F", "Feb", "February"},
                           {"M", "Mar", "March"},     {"A", "Apr", "April"},
                           {"M", "May", "May"},       {"J", "Jun", "June"},
                           {"J", "Jul", "July"},      {"A", "Aug", "August"},
                           {"S", "Sep", "September"}, {"O", "Oct", "October"},
                           {"N", "Nov", "November"},  {"D", "Dec", "December"}};

#include "uthash.h"
struct ele_hfmt {
  int id; /* key */
  char *format;
  UT_hash_handle hh; /* makes this structure hashable */
};
struct ele_hfmt *table_fmt = NULL;
const char *table_fmt_2d_mapping[][2] = {
    {"0", "General"},
    {"1", "0"},
    {"2", "0.00"},
    {"3", "#,##0"},
    {"4", "#,##0.00"},
    {"9", "0%"},
    {"10", "0.00%"},
    {"11", "0.00E+00"},
    {"12", "# ?/?"},
    {"13", "# ??/??"},
    {"14", "m/d/yy"},
    {"15", "d-mmm-yy"},
    {"16", "d-mmm"},
    {"17", "mmm-yy"},
    {"18", "h:mm AM/PM"},
    {"19", "h:mm:ss AM/PM"},
    {"20", "h:mm"},
    {"21", "h:mm:ss"},
    {"22", "m/d/yy h:mm"},
    {"37", "#,##0 ;(#,##0)"},
    {"38", "#,##0 ;[Red](#,##0)"},
    {"39", "#,##0.00;(#,##0.00)"},
    {"40", "#,##0.00;[Red](#,##0.00)"},
    {"45", "mm:ss"},
    {"46", "[h]:mm:ss"},
    {"47", "mmss.0"},
    {"48", "##0.0E+0"},
    {"49", "@"},
    {"56", "\"上午 / 下午 \"hh\"時 \"mm\"分 \"ss\"秒 \""}};

#include <math.h> /* pow */
#include <stdbool.h>
#include <string.h> /* strrev */
const char *fill(const char *c, const size_t l);
const char *nconcat(const char *s1, const char *s2);
const char *pad0(const char *v, const size_t d);
const char *pad_(const char *v, const size_t d);
const char *rpad_(const char *v, const size_t d);
const char *pad0r1(const char *v, const size_t d);
const char *pad0r2(const char *v, const size_t d);

bool isgeneral(const char *s, int i);
#endif
