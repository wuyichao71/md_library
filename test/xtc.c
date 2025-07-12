#include <stdio.h>
#include "xdrfile.h"
#include "xdrfile_xtc.h"

int main(int argc, char *argv[]) {
  XDRFILE xtc;
  int data;
  if (xdrOK != read_open_xtc(argv[1], &xtc)) {
    return 0;
  }

  printf("xtc.n_atoms = %d\n", xtc.n_atoms);
  printf("xtc.current_frame = %d\n", xtc.current_frame);
  printf("xtc.time = %f\n", xtc.time);
  
  return 0;
}