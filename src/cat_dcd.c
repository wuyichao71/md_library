#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#ifdef _WIN32
#else
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <unistd.h>
 #define PATH_SEPARATOR '/'
#endif


#include "dcdfile.h"

#define LINE_MAX 256

#define ERROR(format, ...) \
  do { \
    fprintf(stderr, "Error: " format "\n", ##__VA_ARGS__); \
    exit(EXIT_FAILURE); \
  } while (0)

typedef struct
{
  unsigned int length;
  int *data;
} ARRAY;

const struct option long_options[] = {
  {"outname",   required_argument, NULL, 'o'},
  {"indexname", required_argument, NULL, 'i'},
  {"trajlist",  required_argument, NULL, 'l'},
  {"help",      no_argument,       NULL, 'h'},
  {NULL,        no_argument,       NULL, 0  },
};

void usage(char *prog_name)
{
  fprintf(stderr, "Usage: %s [-h|--help] [-o|--outname outname] [-i|--indexname indexname] [-l|--trajlist trajlist] [filenames]\n", prog_name);
}

void error(char *msg)
{
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

ARRAY *read_index(char *index_filename, ARRAY *arr)
{
  char line[LINE_MAX];
  FILE *fp = fopen(index_filename, "r");
  if (NULL == fp) return NULL;

  // read index line number
  unsigned int line_num = 0;
  while (fgets(line, sizeof(line), fp)) {
    if (line[0] == '\n') continue;
    line_num++;
  }
  if (line_num == 0) {
    fclose(fp);
    return NULL; // empty index file
  }

  arr->length = line_num;
  arr->data = (int *)malloc(line_num * sizeof(int));

  fseek(fp, 0, SEEK_SET);

  // read data
  int i = 0;
  while (fgets(line, sizeof(line), fp) && i < arr->length) {
    if (line[0] == '\n') continue;
    arr->data[i++] = atoi(line);
  }
  fclose(fp);

  return arr;
}

void remove_newline(char *str)
{
  size_t len = strlen(str);
  if (len > 0 && str[len - 1] == '\n')
  {
    str[len - 1] = '\0';
  }
}

void free_index(ARRAY *arr)
{
  arr->length = 0;
  free(arr->data);
  arr->data = NULL;
}

void copy_frame(DCDFILE *dest, DCDFILE *src, ARRAY index_array)
{
  memcpy(dest->unitcell, src->unitcell, sizeof(src->unitcell));
  // printf("%lf, %lf\n", src->unitcell[0], dest->unitcell[0]);
  if (index_array.length == 0) {
    for (int dim = 0; dim < DIM; dim++) {
      memcpy(dest->xyz[dim], src->xyz[dim], src->n_atoms * sizeof(float));
    }
  } else {
    for (int dim = 0; dim < DIM; dim++) {
      for (int i = 0; i < index_array.length; i++) {
        dest->xyz[dim][i] = src->xyz[dim][index_array.data[i]];
      }
    }
  }
}

void mkdir_p(char *dir)
{
  size_t len = strlen(dir);
  if (dir[len - 1] == PATH_SEPARATOR) {
    // Remove trailing slash
    dir[len - 1] = '\0';
  }
  for (char *p = dir + 1; *p; p++) {
    if (*p == PATH_SEPARATOR) {
      *p = '\0'; // Temporarily terminate the string
      if (mkdir(dir, 0755) == -1 && errno != EEXIST) {
        fprintf(stderr, "Error creating directory %s: %s\n", dir, strerror(errno));
        exit(EXIT_FAILURE);
      }
      *p = PATH_SEPARATOR; // Restore the slash
    }
  }
  mkdir(dir, 0755); // Create the final directory
  if (errno != EEXIST && errno != 0) {
    fprintf(stderr, "Error creating directory %s: %s\n", dir, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  int opt;
  char *trajlist_name = NULL;
  char *index_name = NULL;
  char *out_name = NULL;
  bool use_index = false;
  bool use_trajlist = false;
  bool use_outname = false;

  ARRAY *err_p;
  ARRAY index_array;

  char *read_status;
  DCDFILE out_dcd;
  DCDFILE in_dcd;
  DCDFILE *open_status = NULL;
  uint32_t n_atoms;
  char trajname[LINE_MAX];

  while ((opt = getopt(argc, argv, "o:i:l:h")) != -1) {
    switch (opt) {
      case 'o':
        out_name = optarg;
        use_outname = true;
        break;
      case 'i':
        index_name = optarg;
        use_index = true;
        break;
      case 'l':
        trajlist_name = optarg;
        use_trajlist = true;
        break;
      case 'h':
        usage(argv[0]);
        exit(EXIT_SUCCESS);
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (use_trajlist && optind < argc) ERROR("You cannot specify both a trajectory list and additional filenames.");
  else if (!use_trajlist && optind >= argc) ERROR("You must input at least one trajectory file or a trajectory list.");

  // read index
  if (use_index) {
    err_p = read_index(index_name, &index_array);
    if (NULL == err_p) ERROR("Can not read index file %s or the index file is empty!", index_name);
  } else {
    index_array.length = 0;
    index_array.data = NULL; // no index data
  }

  // read the first trajectory name
  FILE *filelist_fp = NULL;
  if (use_trajlist) {
    filelist_fp = fopen(trajlist_name, "r");
    if (NULL == filelist_fp) ERROR("Can not open %s", trajlist_name);
    // skip empty lines
    while ((read_status = fgets(trajname, sizeof(trajname), filelist_fp)) && trajname[0] == '\n');
    if (NULL == read_status) ERROR("There is no filename in %s", trajlist_name);
    remove_newline(trajname);
  } else {
    // if not use trajlist, read from command line arguments
    strcpy(trajname, argv[optind++]);
  }

  if (use_outname && out_name == NULL) {
    ERROR("You must specify an output filename with -o or --outname option.");
  } else if (use_outname && out_name[0] == '\0') {
    ERROR("Output filename cannot be empty.");
  } else if (use_outname) {
    // ensure the output directory exists
    char *last_slash = strrchr(out_name, PATH_SEPARATOR);
    if (last_slash != NULL) {
      char dir_name[LINE_MAX];
      strncpy(dir_name, out_name, last_slash - out_name);
      dir_name[last_slash - out_name] = '\0';
      mkdir_p(dir_name);
    }
  }

  open_status = read_open_dcd(trajname, &in_dcd);
  if (NULL == open_status) ERROR("Can not open %s", trajname);

  n_atoms = use_index ? index_array.length : in_dcd.n_atoms;

  if (use_outname) {
    open_status = write_open_dcd(out_name, n_atoms, &out_dcd);
    if (NULL == open_status) ERROR("Can not open %s", out_name);
  }

  for (int fi = 0; fi < in_dcd.n_frames; fi++) {
    read_dcd_next_frame(&in_dcd);
    if (use_outname == false) continue; // skip if not writing to output
    copy_frame(&out_dcd, &in_dcd, index_array);
    write_dcd_next_frame(&out_dcd);
  }
  close_dcd(&in_dcd);

  while (true) {
    // process the file
    if (use_trajlist) {
      // read the next trajectory name
      read_status = fgets(trajname, sizeof(trajname), filelist_fp);
      if (NULL == read_status) {
        fclose(filelist_fp);
        break; // end of file
      } else {
        // skip empty lines
        if (trajname[0] == '\n') continue;
        remove_newline(trajname);
      }
    } else {
      if (optind < argc) {
        strcpy(trajname, argv[optind++]);
      } else {
        break;
      }
    }

    // open the trajectory file
    open_status = read_open_dcd(trajname, &in_dcd);
    if (NULL == open_status) ERROR("Can not open %s", trajname);

    for (int fi = 0; fi < in_dcd.n_frames; fi++)
      {
        read_dcd_next_frame(&in_dcd);
        if (use_outname == false) continue; // skip if not writing to output
        copy_frame(&out_dcd, &in_dcd, index_array);
        write_dcd_next_frame(&out_dcd);
      }
      close_dcd(&in_dcd);
  }

  close_dcd(&out_dcd);

  if (index_array.length != 0) free_index(&index_array);

  return 0;
}

