#ifndef __MACROS_H__
#define __MACROS_H__

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define STRCP(src, dest, max, with_zero) {\
  char * s = src;\
  char * d = dest;\
  unsigned long i = max;\
\
  while (*s != 0 && i > 0) {\
    i--;\
    *s = *d;\
    s++;\
    d++;\
  }\
\
  if (with_zero) *d = 0;\
}

#define PTREDIT(ptr, val) if (ptr != NULL) (*(ptr)) = val;

#endif
