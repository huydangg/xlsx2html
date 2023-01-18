#include "ssfc.h"
#include <alloca.h>
#include <math.h> /* ceil, round */
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

const char *pad0r1(char *v, const size_t d) {
  int iv = (int)(round(atof(v)));
  printf("AAAAAAAAAAAAAAAa: %d\n", iv);
  sprintf(v, "%d", iv);
  printf("BBBBBBBBBBBBBBBBBBBBBBBBBB\n");
  char *t = (char *)malloc(strlen(v) * sizeof(char));
  strcpy(t, v);
  return strlen(t) >= d ? t : nconcat(fill("0", d - strlen(t)), t);
}
