#include "xdrfile.h"

XDRFILE *read_open_xdr(const char filename[], XDRFILE *xdr) {
  xdr->fp = fopen(filename, "rb");
  if (NULL == xdr->fp) {
    return NULL;
  }
  // xdr->filename = filename;
  return xdr;
}