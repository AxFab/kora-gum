#ifndef _KORA_CSS_H
#define _KORA_CSS_H 1

#include <stdio.h>

typedef void(*css_setter)(void*,const char*,const char*,const char*);

#define CSS_BUF_SIZE 1024

void css_read_file(FILE *stream, void *arg, css_setter setter);
unsigned int css_parse_color(const char* value);
int css_parse_usize(const char* value, int *pSz);
int css_parse_size(const char* value, int *pSz);

// Transform a size into pixels giveun unit-type dpi, dsp and container-size
#define CSS_GET_UNIT(v, u, dpi, dsp, sz) ( \
    (u) == CSS_SIZE_PX ? (v) : ( \
    (u) == CSS_SIZE_PTS ? (v) * (dpi) / 100 : ( \
    (u) == CSS_SIZE_DP ? (v) * (dsp) : ( \
    (u) == CSS_SIZE_PERC ? (v) * (sz) / 1000 : ( \
      0 )))))

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

enum {
  CSS_SIZE_UNDEF = 0,
  CSS_SIZE_PX,
  CSS_SIZE_PTS,
  CSS_SIZE_DP,
  CSS_SIZE_PERC,

};

#endif  /* _KORA_CSS_H */
