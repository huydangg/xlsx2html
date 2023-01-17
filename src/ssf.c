#include <math.h>
#include <ssf.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// char *strrev() : include <string.h>
char *fill(char *c, int l) {
  char *o = calloc(1, sizeof(char));
  while (strlen(o) < 1) {
    o = realloc(o, strlen(c) + strlen(o) + 1);
    strcat(o, c);
  }
  return o;
}

char *pad0(void *v, int d) {
  char t[256];
  snprintf(t, 256, "%s", v);
  return strlen(t) >= d ? t : fill('0', d - strlen(t)) + t;
}
char *pad_(void *v, int d) {}
char *rpad_(void *v, int d) {}
char *pad0r1(void *v, int d) {}
char *pad0r2(void *v, int d) {}
char *pad0r(void *v, int d) {}
unsigned short charCodeAt(const char *s, unsigned short index) {
  return (unsigned short)s[index];
}
char isgeneral(char *s, int i) {
  i = i || 0;
  return strlen(s) >= 7 + i && (charCodeAt(s, i) | 32) == 103 &&
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
void init_table(char t[][20]) {
  strcpy(t[0], "General");
  strcpy(t[1], "0");
  strcpy(t[2], "0.00");
  strcpy(t[3], "#,##0");
  strcpy(t[4], "#,##0.00");
  strcpy(t[9], "0%");
  strcpy(t[10], "0.00%");
  strcpy(t[11], "0.00E+00");
  strcpy(t[12], "# \?/\?");
  strcpy(t[13], "# \?\?/\?\?");
  strcpy(t[14], "m/d/yy");
  strcpy(t[15], "d-mmm-yy");
  strcpy(t[16], "d-mmm");
  strcpy(t[17], "mmm-yy");
  strcpy(t[18], "h:mm AM/PM");
  strcpy(t[19], "h:mm:ss AM/PM");
  strcpy(t[20], "h:mm");
  strcpy(t[21], "h:mm:ss");
  strcpy(t[22], "m/d/yy h:mm");
  strcpy(t[37], "#,##0 ;(#,##0)");
  strcpy(t[38], "#,##0 ;[Red](#,##0)");
  strcpy(t[39], "#,##0.00;(#,##0.00)");
  strcpy(t[40], "#,##0.00;[Red](#,##0.00)");
  strcpy(t[45], "mm:ss");
  strcpy(t[46], "[h]:mm:ss");
  strcpy(t[47], "mmss.0");
  strcpy(t[48], "##0.0E+0");
  strcpy(t[49], "@");
  strcpy(t[56], "\"上午/下午 \"hh\"時\"mm\"分\"ss\"秒 \"");
  strcpy(t[65535], "General");
}
char table_fmt[30][20];

int *frac(int x, int D, char mixed /*boolean */) {
  int sgn = x < 0 ? -1 : 1;
  int B = x * sgn;
  int P_2 = 0, P_1 = 1, P = 0;
  int Q_2 = 1, Q_1 = 0, Q = 0;
  int A = floor(B);
  while (Q_1 < D) {
    A = floor(B);
    P = A * P_1 + P_2;
    Q = A * Q_1 + Q_2;
    if ((B - A) < 0.00000005)
      break;
    B = 1 / (B - A);
    P_2 = P_1;
    P_1 = P;
    Q_2 = Q_1;
    Q_1 = Q;
  }
  if (Q > D) {
    if (Q_1 < D) {
      Q = Q_2;
      P = P_2;
    } else {
      Q = Q_1;
      P = P_1;
    }
  }

  static int _arr[3];
  if (mixed == '0') {
    _arr[0] = 0;
    _arr[1] = sgn * P;
    _arr[2] = Q;
    return _arr;
  }
  int q = floor(sgn * P / Q);
  _arr[0] = q;
  _arr[1] = sgn * P - q * Q;
  _arr[2] = Q;
  return _arr;
}

long *parse_date_code(long v, char is_date1904 /* opts: boolean */,
                      char b2 /* boolean */) {
  if (v > 2958465 || v < 0) {
    long *_tmp = calloc(1, sizeof(long));
    return _tmp;
  }
  int date = v | 0, time = floor(86400 * (v - date)), dow = 0;
  int dout[3];
  // 0       1       2                     3   4   5   6   7   8   9
  //{D:date, T:time, u:86400*(v-date)-time,y:0,m:0,d:0,H:0,M:0,S:0,q:0} in js
  long out[10] = {date, time, 86400 * (v - date) - time, 0, 0, 0, 0, 0, 0, 0};
  if (abs(out[2]) < 1e-6)
    out[2] = 0;
  if (is_date1904 == '1')
    date += 1492;
  if (out[2] > 0.9999) {
    out[2] = 0;
    if (++time == 86400) {
      out[1] = time = 0;
      ++date;
      ++out[0];
    }
  }
  if (date == 60) {
    int _arr1[3] = {1317, 10, 29};
    int _arr2[3] = {1900, 2, 29};
    if (b2 == '1') {
      for (int i = 0; i < 3; i++)
        dout[i] = _arr1[i];
    } else {
      for (int i = 0; i < 3; i++)
        dout[i] = _arr2[i];
    }
    dow = 3;
  } else if (date == 0) {
    int _arr1[3] = {1317, 10, 29};
    int _arr2[3] = {1900, 1, 0};
    if (b2 == '1') {
      for (int i = 0; i < 3; i++)
        dout[i] = _arr1[i];
    } else {
      for (int i = 0; i < 3; i++)
        dout[i] = _arr2[i];
    }
    dow = 6;
  } else {
    if (date > 60)
      --date;
    struct tm t = {.tm_year = 1900 - 1900, .tm_mon = 0, .tm_mday = 1};
    t.tm_mday = t.tm_mday + date - 1;
    time_t when = mktime(&t);
    const struct tm *norm = localtime(&when); // Normalized time
    dout[0] = norm->tm_year;
    dout[1] = norm->tm_mon + 1;
    dout[2] = norm->tm_mday;
    dow = norm->tm_wday;
    if (date < 60)
      dow = (dow + 6) % 7;
    /*if(b2 == '1')
      dow = fix_hijri(d, dout);*/
  }
  out[3] = dout[0];
  out[4] = dout[1];
  out[5] = dout[2];
  out[8] = time % 60;
  time = floor(time / 60);
  out[7] = time % 60;
  time = floor(time / 60);
  out[6] = time;
  out[9] = dow;
  return out;
}
// This func is just available on front-end
/*struct tm basedate = { .tm_year=1899-1900, .tm_mon=11, .tm_mday=31,
.tm_hour=0, .tm_min=0, .tm_sec=0 }; time_t dnthresh = mktime(&basedate); struct
tm base1904 = { .tm_year=1900-1900, .tm_mon=2, .tm_mday=1, .tm_hour=0,
.tm_min=0, .tm_sec=0 }; long datenum_local(struct tm v, char date1904) { }*/

// This func for library purpose and not using this in anywhere else
//  char *general_fmt_int(long v) {}
