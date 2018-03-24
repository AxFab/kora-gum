#include <kora/css.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int css_read_token(char *buffer, int len, char *result, char until, char next, char *state)
{
  int size = 0;
  if (buffer[0] == until) {
    *state = next;
    return 1;
  } else if (buffer[0] == '}') {
    *state = '\0';
    return 1;
  }

  while (size < len && !isspace(buffer[size]) && buffer[size] != until) ++size;
  memcpy(result, buffer, size);
  result[size] = '\0';
  return size;
}


void css_read_file(FILE *stream, void *arg, css_setter setter)
{
  // char name[CSS_BUF_SIZE];
  int len = 0, size = 0;
  char state = '\0';
  char *buffer = (char*)malloc(CSS_BUF_SIZE);
  char *property = (char*)malloc(CSS_BUF_SIZE);
  char *value = (char*)malloc(CSS_BUF_SIZE);
  memset(buffer, 0, CSS_BUF_SIZE);
  char *rule = (char*)malloc(CSS_BUF_SIZE);

  setvbuf(stream, NULL, _IONBF, 0);
  for (;;) {
    // Ensure the buffer is filled
    if (len < CSS_BUF_SIZE && !feof(stream)) {
      len += fread(&buffer[len], 1, CSS_BUF_SIZE - len - 1, stream);
    }
    if (len == 0) {
      break;
    }

    size = 0;
    if (isspace(buffer[0])) {
      while (size < len && isspace(buffer[size])) ++size;
    } else if (buffer[0] == '/' && buffer[1] == '*') {
      size += 2;
      while (buffer[size-1] != '/' || buffer[size-2] != '*') ++size;
    } else if (state == '\0') {
      size = css_read_token(buffer, len, property, '{', '{', &state);
      if (state == '\0') {
        strcpy(rule, property);
        // fprintf(stderr, "CSS name '%s'\n", property);
      }
    } else if (state == '{') {
      size = css_read_token(buffer, len, property, ':', ':', &state);
      if (state == '{') {
        // fprintf(stderr, "CSS property '%s'\n", property);
      }
    } else if (state == ':') {
      size = css_read_token(buffer, len, value, ';', '{', &state);
      if (state == ':') {
        setter(arg, rule, property, value);
        // fprintf(stderr, "CSS set '%s'='%s'\n", property, value);
      }
    }

    // Shift buffer
    // assert (size != 0);
    memmove(buffer, &buffer[size], CSS_BUF_SIZE - size);
    memset(&buffer[CSS_BUF_SIZE - size], 0, size);
    len -= size;
  }

  free(buffer);
  free(property);
  free(value);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

unsigned int css_parse_color(const char* value)
{
  unsigned int color;
  if (value[0] == '0' && (value[1] == 'x' || value[2] == 'x')) {
    sscanf(value, "0x%x", &color);
    if (strnlen(value, 12) == 10) {
      return color;
    } else if (strnlen(value, 12) == 8) {
      return 0xFF000000 | color;
    }
    return 0;
  } else if (value[0] == '#') {
    sscanf(value, "#%x", &color);
    return 0xFF000000 | color;
  } else if (value[0] == 'r' && value[1] == 'g' && value[2] == 'b') {
    if (value[3] == 'a') {
      sscanf(value, "rgba(%hhd,%hhd,%hhd,%hhd)",
        &((char*)&color)[2], &((char*)&color)[1], &((char*)&color)[0], &((char*)&color)[3]);
      return color;
    } else {
      sscanf(value, "rgb(%hhd,%hhd,%hhd)",
        &((char*)&color)[2], &((char*)&color)[1], &((char*)&color)[0]);
      return 0xFF000000 | color;
    }
  } else {
    if (!strcmp("black", value)) {
      return 0xFF000000;
    } else if (!strcmp("white", value)) {
      return 0xFFFFFFFF;
    } else if (!strcmp("red", value)) {
      return 0xFFFF0000;
    } else if (!strcmp("green", value)) {
      return 0xFF00FF00;
    } else if (!strcmp("blue", value)) {
      return 0xFF0000FF;
    }
  }

  return 0;
}

int css_parse_size(const char* value, int *pSz)
{
    char *unit;
    float sz = strtod(value, &unit);
    if (*unit == '\0') {
        *pSz = (short)sz;
        return CSS_SIZE_PX;
    } else if (!strcmp(unit, "px")) {
        *pSz = (short)sz;
        return CSS_SIZE_PX;
    } else if (!strcmp(unit, "pt")) {
        *pSz = (short)(sz * 100 / 72);
        return CSS_SIZE_PTS;
    } else if (!strcmp(unit, "mm")) {
        *pSz = (short)(sz * 100 / 25.4f);
        return CSS_SIZE_PTS;
    } else if (!strcmp(unit, "in")) {
        *pSz = (short)(sz * 100);
        return CSS_SIZE_PTS;
    } else if (!strcmp(unit, "dp")) {
        *pSz = (short)sz;
        return CSS_SIZE_DP;
    } else if (!strcmp(unit, "%")) {
        *pSz = (short)(sz * 10);
        return CSS_SIZE_PERC;
    } else {
        *pSz = (short)sz;
        return CSS_SIZE_PX;
    }
    *pSz = 0;
    return 0;
}

int css_parse_usize(const char* value, int *pSz)
{
  int sz;
  int unit = css_parse_size(value, &sz);
  *pSz = sz > 0 ? sz : 0;
  return unit;
}
