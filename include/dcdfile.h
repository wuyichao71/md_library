#ifndef __DCDFILE_H__
#define __DCDFILE_H__

#include <stdbool.h>
#include <stdint.h>

enum ENDIAN
{
  LittleEndian,
  BigEndian
};

#define UNITCELL_LENGTH 6

/*
CHARMM FORMAT
uint32_t 84 // sometimes uint64_t
char[4] "CORD"
uint32_t NSET // n_frames
uint32_t ISTRT // initial_step
uint32_t NSAVC // save_interval
uint32_t MDSTEP // md_steps
uint32_t[4]
uint32_t NATOM_NFREAT // the number of fixed atoms
float delta // MD delta step
uint32_t[9]
uint32_t CHARMM_VERSION // charmm_version
*/

typedef struct
{
  FILE *fp;
  char *filename;
  // hdr information
  enum ENDIAN endian;
  uint32_t n_frames;      // hdr[0]
  uint32_t initial_step;  // hdr[1]
  uint32_t step_interval; // hdr[2]
  uint32_t md_steps;      // hdr[3]
  uint32_t natom_nfreat;  // hdr[8]
  double delta;           // hdr[9]
  bool is_charmm;
  uint32_t charmm_version; // hdr[19]

  // natom
  uint32_t n_atoms;

  // unitcell
  double unitcell[UNITCELL_LENGTH];

  // xyz
  // float (*xyz)[3];
  float *xyz[3];

  // current frames;
  int32_t current_frame;
} DCDFILE;

DCDFILE read_dcd(char filename[]);
void read_dcd_next_frame(DCDFILE *dcd);

#endif
