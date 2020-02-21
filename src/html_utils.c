#include <string.h>



char *create_tag(char *tag_name) {
  const char *tag_n = tag_name;
  const char *tag_n = tag_name;
  char *result = "<"
  result = strcat(result, tag_n);
  result = strcat(result, ">");
  return result;
}

char *close_tag(const char *tag_name) {
  const char *tag_n = tag_name;
  char *result = "</"
  result = strcat(result, tag_n);
  result = strcat(result, ">");
  return result;
}

char write_attr(const char *obj_tag, const char *attr_name, const char *value) {
  // obj_tag: It's value from create_tag
  const char *obj = obj_tag;
  const char *attr_n = attr_name;
  const char *v = value;

}
