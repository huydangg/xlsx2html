#include "ssfc.h"
#include <alloca.h>
#include <math.h> /* ceil, round */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> /* malloc */

const char *nconcat(const char *s1, const char *s2) {
  char *ns = (char *)malloc((strlen(s1) + strlen(s2) + 1) * sizeof(char));
  ns[0] = '\0';
  strcpy(ns, s1);
  strcat(ns, s2);
  return ns;
}

const char *fill(const char *c, const size_t l) {
  size_t len_c = strlen(c);
  int size_c = ceil((double)l / len_c);
  char *o = (char *)malloc((len_c * size_c) * sizeof(char));
  for (int i = 0; i < size_c; i++) {
    strcat(o, c);
  }
  return o;
}

const char *pad0(const char *v, const size_t d) {
  char *t = (char *)malloc(strlen(v) * sizeof(char));
  strcpy(t, v);
  return strlen(t) >= d ? t : nconcat(fill("0", d - strlen(t)), t);
}

const char *pad_(const char *v, const size_t d) {
  char *t = (char *)malloc(strlen(v) * sizeof(char));
  strcpy(t, v);
  return strlen(t) >= d ? t : nconcat(fill(" ", d - strlen(t)), t);
}

const char *rpad_(const char *v, const size_t d) {
  char *t = (char *)malloc(strlen(v) * sizeof(char));
  strcpy(t, v);
  return strlen(t) >= d ? t : nconcat(t, fill(" ", d - strlen(t)));
}

const char *pad0r1(const char *v, const size_t d) {
  int iv = (int)(round(atof(v)));
  int n_ditgits = floor(log10(abs(iv))) + 1;
  char *tv = (char *)malloc(n_ditgits * sizeof(char));
  sprintf(tv, "%d", iv);
  return strlen(tv) >= d ? tv : nconcat(fill("0", d - strlen(tv)), tv);
}

const char *pad0r2(const char *v, const size_t d) {
  char *t = (char *)malloc(strlen(v) * sizeof(char));
  strcpy(t, v);
  return strlen(t) >= d ? t : nconcat(fill("0", d - strlen(t)), t);
}

const char *pad0r(const char *v, const size_t d) {
  double dv = atof(v);
  if (dv > p2_32 || dv < -1 * p2_32) {
    return pad0r1(v, d);
  }
  int iv = (int)(round(atof(v)));
  int n_ditgits = floor(log10(abs(iv))) + 1;
  char *tv = (char *)malloc(n_ditgits * sizeof(char));
  sprintf(tv, "%d", iv);
  return pad0r2(tv, d);
}

unsigned short charCodeAt(const char *s, unsigned short index) {
  return (unsigned short)s[index];
}
bool isgeneral(const char *s, int i) {
  return (int)strlen(s) >= 7 + i && (charCodeAt(s, i) | 32) == 103 &&
         (charCodeAt(s, i + 1) | 32) == 101 &&
         (charCodeAt(s, i + 2) | 32) == 110 &&
         (charCodeAt(s, i + 3) | 32) == 101 &&
         (charCodeAt(s, i + 4) | 32) == 114 &&
         (charCodeAt(s, i + 5) | 32) == 97 &&
         (charCodeAt(s, i + 6) | 32) == 108;
}

char days[][2][10] = {{"Sun", "Sunday"},   {"Mon", "Monday"},
                      {"Tue", "Tuesday"},  {"Wed", "Wednesday"},
                      {"Thu", "Thursday"}, {"Fri", "Friday"},
                      {"Sat", "Saturday"}};
char months[][3][10] = {{"J", "Jan", "January"},   {"F", "Feb", "February"},
                        {"M", "Mar", "March"},     {"A", "Apr", "April"},
                        {"M", "May", "May"},       {"J", "Jun", "June"},
                        {"J", "Jul", "July"},      {"A", "Aug", "August"},
                        {"S", "Sep", "September"}, {"O", "Oct", "October"},
                        {"N", "Nov", "November"},  {"D", "Dec", "December"}};
