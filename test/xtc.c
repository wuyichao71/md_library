#include <stdio.h>
#include "xdrfile.h"
#include "xdrfile_xtc.h"

int main(int argc, char *argv[]) {
  XDRFILE xtc;
  int data;
  if (NULL == read_open_xdr(argv[1], &xtc)) {
    return 0;
  }
  xdr_read_int32(&data, 1, &xtc);
  printf("%d\n", data);
  return 0;
}