#ifndef __XDRFILE_H__
#define __XDRFILE_H__

#include <stdio.h>
#include <stdint.h>

typedef struct {
  FILE *fp;
  char *filename;
} XDRFILE;

XDRFILE *read_open_xdr(const char filename[], XDRFILE *xdr);
int xdr_read_int32(int32_t *ptr, int ndata, XDRFILE *xdr);


#endif