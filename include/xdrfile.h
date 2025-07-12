#ifndef __XDRFILE_H__
#define __XDRFILE_H__

#include <stdio.h>
#include <stdint.h>

enum {
  xdrOK, xdrINT, xdrFLOAT, xdrMAGIC, xdrOPENERROR,
};

typedef struct {
  FILE *fp;
  char *filename;
  uint32_t n_frames;
  uint32_t n_atoms;
  // current frames;
  int32_t current_frame;
  float time;
} XDRFILE;

int read_open_xdr(const char filename[], XDRFILE *xdr);
int xdr_read_int32(int32_t *ptr, int ndata, XDRFILE *xdr);
int xdr_read_float(float *ptr, int ndata, XDRFILE *xdr);

#endif