#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "dcdfile.h"

#define LINE_MAX 256

typedef struct
{
  unsigned int length;
  int *data;
} ARRAY;

const struct option long_options[] = {
  {"outname", required_argument, NULL, 'o'},
  {"indexname"   , required_argument, NULL, 'i'},
  {NULL          , 0                , NULL, 0  },
};

void usage(char *prog_name)
{
  fprintf(stderr, "Usage: %s [-o outname] [-i indexname] trajlist\n", prog_name);
}

// void error(char *msg)
// {
//   fprintf(stderr, "%s\n", msg);
//   exit(EXIT_FAILURE);
// }

void opt_print(char opt, char *optarg, int optind)
{
  printf("-%c: optarg = %s, optind = %d\n", opt, optarg, optind);
}

ARRAY *read_index(char *index_filename, ARRAY *arr)
{
  char line[LINE_MAX];
  FILE *fp = fopen(index_filename, "r");
  if (NULL == fp)
  {
    return NULL;
  }

  // read index line number
  unsigned int line_num = 0;
  while (fgets(line, sizeof(line), fp))
  {
    if (line[0] == '\n') continue;
    line_num++;
  }

  arr->length = line_num;
  arr->data = (int *)malloc(line_num * sizeof(int));

  fseek(fp, 0, SEEK_SET);

  // read data
  int i = 0;
  while (fgets(line, sizeof(line), fp) && i < arr->length)
  {
    if (line[0] == '\n') continue;
    arr->data[i++] = atoi(line);
  }

  return arr;
}

void free_index(ARRAY *arr)
{
  arr->length = 0;
  free(arr->data);
}


int main(int argc, char *argv[])
{
  int opt;
  char *trajlist_name = NULL;
  char *index_name = NULL;
  char *out_name = NULL;

  while ((opt = getopt(argc, argv, "o:i:")) != -1)
  {
    switch (opt)
    {
      case 'o':
        out_name = optarg;
        // opt_print(opt, out_name, optind);
        break;
      case 'i':
        index_name = optarg;
        // opt_print(opt, index_name, optind);
        break;
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc)
  {
    fprintf(stderr, "You must input trajlist!\n");
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  trajlist_name = argv[optind];

  // read index
  ARRAY *err_p;
  ARRAY index_array;
  if (NULL != index_name)
  {
    err_p = read_index(index_name, &index_array);
    if (NULL == err_p)
    {
      fprintf(stderr, "Can not open %s\n", index_name);
      exit(EXIT_FAILURE);
    }
  }


  char line[LINE_MAX];
  FILE *filelist_fp = fopen(trajlist_name, "r");
  if (NULL == filelist_fp)
  {
    fprintf(stderr, "Can not open %s\n", trajlist_name);
    exit(EXIT_FAILURE);
  }

  char *read_status;
  DCDFILE out_dcd;
  DCDFILE in_dcd;
  DCDFILE *open_status = NULL;
  uint32_t n_atoms;

  while ((read_status = fgets(line, sizeof(line), filelist_fp)) && line[0] == '\n');
  if (NULL == read_status)
  {
    fprintf(stderr, "There is no filename in %s\n", trajlist_name);
    exit(EXIT_FAILURE);
  }

  int len = strlen(line);
  if (line[len - 1] == '\n') line[len - 1] = '\0';
  
  open_status = read_open_dcd(line, &in_dcd);
  if (NULL == open_status)
  {
    fprintf(stderr, "Can not open %s\n", line);
    exit(EXIT_FAILURE);
  }

  n_atoms = NULL == index_name ? in_dcd.n_atoms : index_array.length; 
  open_status = write_open_dcd(out_name, n_atoms, &out_dcd);

  for (int fi = 0; fi < in_dcd.n_frames; fi++)
  {
    read_dcd_next_frame(&in_dcd);
    memcpy(out_dcd.unitcell, in_dcd.unitcell, sizeof(in_dcd.unitcell));
    if (NULL == index_name)
    {
      for(int dim = 0; dim < DIM; dim++)
      {
        memcpy(out_dcd.xyz[dim], in_dcd.xyz[dim], in_dcd.n_atoms * sizeof(float));
      }
    }
    else
    {
      for(int dim = 1; dim < DIM; dim++)
      {
        for (int i = 0; i < index_array.length; i++)
        {
          out_dcd.xyz[dim][i] = in_dcd.xyz[dim][index_array.data[i]];
        }
      }
    }
    write_dcd_next_frame(&out_dcd);
  }
  close_dcd(&in_dcd);

  while ((fgets(line, sizeof(line), filelist_fp)))
  {
    if (line[0] == '\n') continue;

    int len = strlen(line);
    if (line[len - 1] == '\n') line[len - 1] = '\0';
    
    open_status = read_open_dcd(line, &in_dcd);
    if (NULL == open_status)
    {
      fprintf(stderr, "Can not open %s\n", line);
      exit(EXIT_FAILURE);
    }
    for (int fi = 0; fi < in_dcd.n_frames; fi++)
      {
        read_dcd_next_frame(&in_dcd);
        memcpy(out_dcd.unitcell, in_dcd.unitcell, sizeof(in_dcd.unitcell));
        if (NULL == index_name)
        {
          for(int dim = 0; dim < DIM; dim++)
          {
            memcpy(out_dcd.xyz[dim], in_dcd.xyz[dim], in_dcd.n_atoms * sizeof(float));
          }
        }
        else
        {
          for(int dim = 1; dim < DIM; dim++)
          {
            for (int i = 0; i < index_array.length; i++)
            {
              out_dcd.xyz[dim][i] = in_dcd.xyz[dim][index_array.data[i]];
            }
          }
        }
        write_dcd_next_frame(&out_dcd);
      }
      close_dcd(&in_dcd);
  }

  close_dcd(&out_dcd);

  if (NULL != index_name)
  {
    free_index(&index_array);
  }

  return 0;
}

