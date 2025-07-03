#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#include "dcdfile.h"

const struct option long_options[] = {
  {"trajlistfile", required_argument, NULL, 'l'},
  {"indexfile"   , required_argument, NULL, 'i'},
  {NULL          , 0                , NULL, 0  },
};

void usage(char *prog_name)
{
  fprintf(stderr, "Usage: %s [-l trajlist] [-i indexfile]\n", prog_name);
}

void opt_print(char opt, char *optarg, int optind)
{
  printf("-%c: optarg = %s, optind = %d\n", opt, optarg, optind);
}

int main(int argc, char *argv[])
{
  int opt;
  char *trajlistfile = NULL;
  char *indexfile = NULL;

  while ((opt = getopt(argc, argv, "l:i:")) != -1)
  {
    switch (opt)
    {
      case 'i':
        indexfile = optarg;
        opt_print(opt, indexfile, optind);
        break;
      case 'l':
        trajlistfile = optarg;
        opt_print(opt, trajlistfile, optind);
        break;
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  printf("optind = %d\n", optind);

  // char *dcdname = argv[1];

  // DCDFILE dcd = read_dcd(dcdname);

  // for (int fi = 0; fi < dcd.n_frames; fi++)
  // {
  //   read_dcd_next_frame(&dcd);
  //   // printf("%f, %f, %f\n", dcd.xyz[0][0], dcd.xyz[1][0], dcd.xyz[2][0]);
  //   // printf("frame index = %d\n", dcd.current_frame);
  // }

  return 0;
}

