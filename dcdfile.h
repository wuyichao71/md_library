#ifndef __DCDFILE_H__
#define __DCDFILE_H__

enum ENDIAN {
  LittleEndian,
  BigEndian
};

typedef struct {
  FILE *fp;
  char *filename;
  enum ENDIAN endian;

} DCDFILE;

DCDFILE read_dcd(char filename[]);

#endif
