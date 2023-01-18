/* ssf.js (C) 2013-present SheetJS -- http://sheetjs.com */
#ifndef INCLUDED_SSFC_H
#define INCLUDED_SSFC_H

#define SSFJS_VERSION 0.11.2
#define p2_32 pow(2, 32)
const char days[][2][10] = {{"Sun", "Sunday"},   {"Mon", "Monday"},
                            {"Tue", "Tuesday"},  {"Wed", "Wednesday"},
                            {"Thu", "Thursday"}, {"Fri", "Friday"},
                            {"Sat", "Saturday"}};
const char months[][3][10] = {
    {"J", "Jan", "January"},   {"F", "Feb", "February"},
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
