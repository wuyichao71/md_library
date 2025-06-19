#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "dcdfile.h"

void read_error(const char *msg);

inline void read_error(const char *msg) {
  fprintf(stderr, msg);
  exit(EXIT_FAILURE);
}

/* read dcd file header */
void read_dcd_header(DCDFILE *dcd) {
  uint32_t first_int;
  uint32_t magic_int;
  const char dcdbytes[] = {'C', 'O', 'R', 'D'};
  uint32_t header;

  memcpy(&magic_int, dcdbytes, sizeof(magic_int));

  /* read first 32-bit int */
  fread(&first_int, sizeof(uint32_t), 1, dcd->fp);
  if (first_int != 84) {
    read_error("The dcd file is not little endian, I don't implement it now!");
  }

  fread(&header, sizeof(char), 4, dcd->fp);
  if (header != magic_int) {
    read_error("The CORD magic is not right");
  }
  // printf("0x%x\n", magic_int);
  // printf("0x%x\n", header);
  
  return;
}

DCDFILE read_dcd(char filename[]) {
  DCDFILE dcd;
  dcd.fp = fopen(filename, "rb");
  read_dcd_header(&dcd);
  return dcd;
}