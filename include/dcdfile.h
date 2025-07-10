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
#define DCD_SUCCESS 0
#define DCD_FAILURE 1
#define DIM 3

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
uint32_t HAVE_UNITCELL // MD delta step
uint32_t[8]
uint32_t CHARMM_VERSION // charmm_version
*/

typedef struct {
  FILE *fp;
  char *filename;
  // hdr information
  enum ENDIAN endian;
  uint32_t n_frames;      // hdr[1]
  uint32_t initial_step;  // hdr[2]
  uint32_t step_interval; // hdr[3]
  uint32_t md_steps;      // hdr[4]
  uint32_t natom_nfreat;  // hdr[9]
  double delta;           // hdr[10]
  uint32_t have_unitcell; // hdr[11]
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

DCDFILE *read_open_dcd(char filename[], DCDFILE *dcd);
DCDFILE *write_open_dcd(char filename[], uint32_t n_atoms, DCDFILE *dcd);
// int write_dcd_header(DCDFILE *dcd);
void read_dcd_next_frame(DCDFILE *dcd);
void write_dcd_next_frame(DCDFILE *dcd);
void close_dcd(DCDFILE *dcd);

#endif
