#ifndef __XDRFILE_H__
#define __XDRFILE_H__

#include <stdio.h>

typedef struct {
  FILE *fp;
  char *filename;
} XDRFILE;

#endif