#include "xdrfile.h"
#include <stdint.h>

#define xdr_read_int xdr_read_int32

int32_t xdr_int32(FILE *fp, int32_t *data_p);
int32_t ntohl_int32(int32_t x);
int32_t swapbytes_int32(int32_t x);

int read_open_xdr(const char filename[], XDRFILE *xdr) {
  xdr->fp = fopen(filename, "rb");
  if (NULL == xdr->fp) {
    return xdrOPENERROR;
  }
  // xdr->filename = filename;
  return xdrOK;
}

/* We should combine these functions into MICROs */
int xdr_read_int32(int32_t *ptr, int ndata, XDRFILE *xdr) {
  int i = 0;
  while (i < ndata && xdr_int32(xdr->fp, ptr + i)) {
    i++;
  }
  return i;
}

int xdr_read_float(float *ptr, int ndata, XDRFILE *xdr) {
  int i = 0;
  while (i < ndata && xdr_int32(xdr->fp, (int32_t *)(ptr + i))) {
    i++;
  }
  return i;
}

int xdr_int32(FILE *fp, int32_t *data_p) {
  int32_t copy;
  if (fread(&copy, sizeof(int32_t), 1, fp) == 1) {
    *data_p = ntohl_int32(copy);
    return 1;
  }
  return 0;
}

int32_t ntohl_int32(int32_t x) {
  int s = 0x1234;
  if ((char)0x34 == *((char *)&s)) {
    return swapbytes_int32(x);
  } else {
    return x;
  }
}

int32_t swapbytes_int32(int32_t x) {
  return (((x >> 24) & 0xff) | ((x & 0xff) << 24) | ((x >> 8) & 0xff00) | ((x & 0xff00) << 8));
}

/* We should combine these functions into MICROs */