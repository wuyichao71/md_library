#ifndef __TRAJECTORY_H__
#define __TRAJECTORY_H__

#include <stdio.h>

typedef struct
{
  char *filename;
  FILE *fp;
  int _tmp;

} TRAJECTORY;

#endif