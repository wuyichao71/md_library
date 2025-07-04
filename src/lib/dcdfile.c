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
#define DEF(f) f(CORD) f(NSET) f(ISTRT) f(NSAVC) f(NSTEP) f(NATOM_NFREAT, 9) f(DELTA) f(HAVE_UNITCELL) f(CHARMM_VERSION, 20) f(HDR_LENGTH)

#define FIRST_RECORD_LENGTH (HDR_LENGTH * 4)
#define TITLE_LENGTH 80

enum
{
  DEF(ENUM_DEFINE)
};

  const union
  {
    char _cord_char[4];
    uint32_t magic_int;
  } cord = {'C', 'O', 'R', 'D'};

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
  // uint32_t header;
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
  dcd->delta = *(float *)(hdr + DELTA);
  dcd->have_unitcell = hdr[HAVE_UNITCELL];
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
  if (0 != ((record_marker[0] - 4) % TITLE_LENGTH))
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

  // read natom
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

  for (int dim = 0; dim < DIM; dim++)
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
  printf("have_unitcell = %d\n", dcd->have_unitcell);
  printf("charmm_version = %d\n", dcd->charmm_version);

  // debug title
  printf("ntitle = %d\n", ntitle);

  // debug natom
  printf("n_atoms = %d\n", dcd->n_atoms);
#endif

  return DCD_SUCCESS;
}

int write_dcd_header(DCDFILE *dcd)
{
  // fseek(dcd->fp, 0, SEEK_SET);
  uint32_t record_marker;
  uint32_t hdr[HDR_LENGTH] = {0};
  const char title[][TITLE_LENGTH] = {
    "My dcd library version 0.0",
    "Hello, DCD",
    "Hello, DCD",
    "Hello, DCD"
  };
  uint32_t ntitle = sizeof(title) / sizeof(title[0]);

  hdr[CORD] = cord.magic_int;
  hdr[NSET] = dcd->n_frames;
  hdr[ISTRT] = dcd->initial_step;
  hdr[NSAVC] = dcd->step_interval;
  hdr[NSTEP] = dcd->md_steps;
  hdr[NATOM_NFREAT] = dcd->natom_nfreat;
  *(float *)(hdr + DELTA) = dcd->delta;
  hdr[HAVE_UNITCELL] = dcd->have_unitcell;
  hdr[CHARMM_VERSION] = dcd->charmm_version;

  record_marker = FIRST_RECORD_LENGTH;

  // write hdr
  fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);
  fwrite(hdr, sizeof(uint32_t), HDR_LENGTH, dcd->fp);
  fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);

  // write title
  record_marker = sizeof(ntitle) + sizeof(title);
  fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);
  fwrite(&ntitle, sizeof(ntitle), 1, dcd->fp);
  for (int i = 0; i < ntitle; i++)
  {
    fwrite(title[i], sizeof(title[i]), 1, dcd->fp);
  }
  fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);

  // write natom
  record_marker = sizeof(dcd->n_atoms);
  fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);
  fwrite(&dcd->n_atoms, sizeof(dcd->n_atoms), 1, dcd->fp);
  fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);

  // fseek(dcd->fp, 0, SEEK_END);


#ifdef DEBUG
  printf("OUT: n_atoms = %d\n", dcd->n_atoms);

  printf("OUT: title = %d\n", sizeof(title));
  printf("OUT: title = %d\n", sizeof(title[0]));
  printf("OUT: ntitle = %d\n", ntitle);
#endif
  return DCD_SUCCESS;
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

  fread(dcd->unitcell, sizeof(dcd->unitcell), 1, dcd->fp);

  fread(record_marker, sizeof(uint32_t), 1, dcd->fp);
  if (48 != record_marker[0])
  {
    read_error("BOX SIZE END: The record size of unitcell block is not right");
  };

  /* read xyz data*/
  for (int dim = 0; dim < DIM; dim++)
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

void write_dcd_next_frame(DCDFILE *dcd)
{
  uint32_t record_marker;

  /* write unitcell */
  record_marker = UNITCELL_LENGTH * sizeof(double);
  fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);
  fwrite(dcd->unitcell, sizeof(dcd->unitcell), 1, dcd->fp);
  fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);
  // printf("sizeof(unitcell) = %d\n", sizeof(dcd->unitcell));

  /* write xyz data */
  for (int dim = 0; dim < DIM; dim++)
  {
    record_marker = dcd->n_atoms * sizeof(float);
    fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);
    fwrite(dcd->xyz[dim], sizeof(float), dcd->n_atoms, dcd->fp);
    fwrite(&record_marker, sizeof(record_marker), 1, dcd->fp);
  }

  /* update n_frames */
  dcd->n_frames++;
  dcd->md_steps++;
  dcd->current_frame++;

  long original_pos = ftell(dcd->fp);
  fseek(dcd->fp, (1 + NSET) * sizeof(uint32_t), SEEK_SET);
  fwrite(&dcd->n_frames, sizeof(dcd->n_frames), 1, dcd->fp);
  fseek(dcd->fp, (1 + NSTEP) * sizeof(uint32_t), SEEK_SET);
  fwrite(&dcd->md_steps, sizeof(dcd->n_frames), 1, dcd->fp);

  fseek(dcd->fp, original_pos, SEEK_SET);
}

DCDFILE *read_open_dcd(char filename[], DCDFILE *dcd)
{
  dcd->fp = fopen(filename, "rb");
  if (NULL == dcd->fp)
  {
    return NULL;
  }

  int status;
  status = read_dcd_header(dcd);
  if (DCD_SUCCESS != status)
  {
    return NULL;
  }
  return dcd;
}

DCDFILE *write_open_dcd(char filename[], uint32_t n_atoms, DCDFILE *dcd)
{
  dcd->fp = fopen(filename, "wb");
  if (NULL == dcd->fp)
  {
    return NULL;
  }

  dcd->n_atoms = n_atoms;

  for (int dim = 0; dim < DIM; dim++)
  {
    dcd->xyz[dim] = (float *)malloc(dcd->n_atoms * sizeof(float)); 
  }
  dcd->n_frames = 0;
  dcd->initial_step = 1;
  dcd->step_interval = 1;
  dcd->md_steps = 0;
  dcd->natom_nfreat = 0;
  dcd->delta = 1.0;
  dcd->have_unitcell = 1;
  dcd->is_charmm = true;
  dcd->charmm_version = 24;
  dcd->current_frame = -1;
  write_dcd_header(dcd);

  // write_dcd_header(dcd);
  return dcd;
}


void close_dcd(DCDFILE *dcd)
{


  for(int dim = 0; dim < DIM; dim++)
  {
    free(dcd->xyz[dim]);
    dcd->xyz[dim] = NULL;
  }

  dcd->current_frame = -1;
  dcd->n_frames = 0;
  dcd->md_steps = 0;
  dcd->n_atoms = 0;

  fclose(dcd->fp);
  dcd->fp = NULL;  
}