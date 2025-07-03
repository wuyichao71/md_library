#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dcdfile.h"

#define PRINT_ENUM(a, ...) printf(#a " = %d\n", a);
#define GET_MACRO(_1, _2, NAME, ...) NAME
#define FUNC1(a) a,
#define FUNC2(a, b) a = b,
#define ENUM_DEFINE(...) GET_MACRO(__VA_ARGS__, FUNC2, FUNC1)(__VA_ARGS__)
#define DEF(f) f(CORD) f(NSET) f(ISTRT) f(NSAVC) f(NSTEP) f(NATOM_NFREAT, 9) f(DELTA) f(CHARMM_VERSION, 20) f(HDR_LENGTH)

#define FIRST_RECORD_LENGTH (HDR_LENGTH * 4)
#define TITLE_LENGTH 80

enum
{
  DEF(ENUM_DEFINE)
};

void read_error(const char *msg);

inline void read_error(const char *msg)
{
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

/* read dcd file header */
int read_dcd_header(DCDFILE *dcd)
{
  uint32_t record_marker[2];

  // hdr
  const union
  {
    char _cord_char[4];
    uint32_t magic_int;
  } cord = {'C', 'O', 'R', 'D'};
  uint32_t header;
  uint32_t hdr[HDR_LENGTH];

  // title
  int ntitle;
  char title[TITLE_LENGTH];

  // memcpy(&magic_int, dcdbytes, sizeof(magic_int));

  /* get the begin size of the first block (hdr) */
  fread(&record_marker, sizeof(uint32_t), 1, dcd->fp);
  if (FIRST_RECORD_LENGTH != record_marker[0])
  {
    read_error("HDR BEGIN: This format is not correct or supported!");
    // read_error("The dcd file is not little endian, I don't implement it now!");
  }



  // read hdr, the size is 80 chars
  fread(hdr, sizeof(uint32_t), HDR_LENGTH, dcd->fp);

  if (hdr[CORD] != cord.magic_int)
  {
    read_error("The CORD magic is not right");
  }

  dcd->n_frames = hdr[NSET];
  dcd->initial_step = hdr[ISTRT];
  dcd->step_interval = hdr[NSAVC];
  dcd->md_steps = hdr[NSTEP];
  dcd->natom_nfreat = hdr[NATOM_NFREAT];
  dcd->delta = *(float *)(hdr + 9);
  dcd->charmm_version = hdr[CHARMM_VERSION];

  if (dcd->charmm_version != 0)
  {
    dcd->is_charmm = true;
  }

  /* get the end size of the first block (hdr). */
  fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
  if (FIRST_RECORD_LENGTH != record_marker[0])
  {
    read_error("HDR END: This format is not correct!");
  }

  /* get the begin size of the second block (title). */
  fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
  if (0 != ((record_marker[0] - 4) % 80))
  {
    read_error("TITLE BEGIN: This titles is not multiple 80 chars");
  }

  // read title
  fread(&ntitle, sizeof(int32_t), 1, dcd->fp);
  // check format
  if (TITLE_LENGTH * ntitle + 4 != record_marker[0])
  {
    read_error("TITLE BEGIN: The record size is not equal to the title size!");
  }

  for (int i = 0; i < ntitle; i++)
  {
    fread(title, sizeof(char), TITLE_LENGTH, dcd->fp);
  }

  fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
  if (TITLE_LENGTH * ntitle + 4 != record_marker[0])
  {
    read_error("TITLE END: The record size is not equal to the title size!");
  }

  fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
  if (sizeof(uint32_t) != record_marker[0])
  {
    read_error("NATOM BEGIN: The record size of natom block is not right");
  }

  fread(&dcd->n_atoms, sizeof(uint32_t), 1, dcd->fp);

  fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
  if (sizeof(uint32_t) != record_marker[0])
  {
    read_error("NATOM END: The record size of natom block is not right");
  }

  for (int dim = 0; dim < 3; dim++)
  {
    dcd->xyz[dim] = (float *)malloc(dcd->n_atoms * sizeof(float)); 
  }

  dcd->current_frame = -1;

#ifdef DEBUG
  DEF(PRINT_ENUM);

  // debug hdr
  printf("n_frames = %d\n", dcd->n_frames);
  printf("initial_step = %d\n", dcd->initial_step);
  printf("step_interval = %d\n", dcd->step_interval);
  printf("md_steps = %d\n", dcd->md_steps);
  printf("natom_nfreat = %d\n", dcd->natom_nfreat);
  printf("delta = %lf\n", dcd->delta);
  printf("charmm_version = %d\n", dcd->charmm_version);

  // debug title
  printf("ntitle = %d\n", ntitle);

  // debug natom
  printf("n_atoms = %d\n", dcd->n_atoms);
#endif

  return 0;
}

void read_dcd_next_frame(DCDFILE *dcd)
{
  uint32_t record_marker[2];

  /* read unitcell */
  fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
  if (48 != record_marker[0])
  {
    read_error("UNITCELL BEGIN: The record size of unitcell block is not right");
  }

  fread(dcd->unitcell, sizeof(double), UNITCELL_LENGTH, dcd->fp);

  fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
  if (48 != record_marker[0])
  {
    read_error("BOX SIZE END: The record size of unitcell block is not right");
  };

  /* read xyz data*/
  for (int dim = 0; dim < 3; dim++)
  {
    fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
    if (sizeof(float) * dcd->n_atoms != record_marker[0])
    {
      read_error("XYZ BEGIN: The record size of xyz block is not right");
    }

    fread(dcd->xyz[dim], sizeof(float), dcd->n_atoms, dcd->fp);

    fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
    if (sizeof(float) * dcd->n_atoms != record_marker[0])
    {
      read_error("XYZ END: The record size of xyz block is not right");
    }
  }

  dcd->current_frame++;
}

DCDFILE read_dcd(char filename[])
{
  DCDFILE dcd;
  int status;
  dcd.fp = fopen(filename, "rb");
  status = read_dcd_header(&dcd);
  return dcd;
}