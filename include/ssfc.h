/* ssf.js (C) 2013-present SheetJS -- http://sheetjs.com */
#ifndef INCLUDED_SSFC_H
#define INCLUDED_SSFC_H

#define SSFJS_VERSION 0.11.2

#include <string.h> /* strrev */

const char *fill(const char *c, const size_t l);
const char *nconcat(const char *s1, const char *s2);
const char *pad0(const char *v, const size_t d);
const char *pad_(const char *v, const size_t d);
const char *rpad_(const char *v, const size_t d);
const char *pad0r1(char *v, const size_t d);

#endif
