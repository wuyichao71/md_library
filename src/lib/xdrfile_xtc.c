#include "xdrfile.h"
#include "xdrfile_xtc.h"

#define MAGIC 1995

int read_xtc_header(XDRFILE *xtc) {
  int magic = MAGIC;
  int n = 1;
  if (xdr_read_int32(&magic, n, xtc) != n) return xdrINT;
  if (MAGIC != magic) return xdrMAGIC;
  // printf("%d\n", magic);

  /* n_atoms */
  if (xdr_read_int32(&xtc->n_atoms, n, xtc) != n) return xdrINT;

  /* current_frame */
  if (xdr_read_int32(&xtc->current_frame, n, xtc) != n) return xdrINT;

  /* time */
  if (xdr_read_float(&xtc->time, n, xtc) != n) return xdrFLOAT;
  
  return xdrOK;
}

int read_open_xtc(const char filename[], XDRFILE *xtc) {
  int status;
  status = read_open_xdr(filename, xtc);
  if (xdrOK != status) {
    return status;
  }
  return read_xtc_header(xtc);
}
