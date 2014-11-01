
// Trivial debug logging macro.

#ifndef debug_h
#define debug_h

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef DEBUG
  #define debug(M, ...) fprintf(stderr, "DEBUG @%d: " M "\n", __LINE__, ##__VA_ARGS__)
#else
  #define debug(M, ...)
#endif

#endif
