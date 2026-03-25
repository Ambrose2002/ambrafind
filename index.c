
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "index.h"

static char *output = NULL;
static size_t output_len = 0;
static size_t output_cap = 0;
static long file_count = 0;

static int append_path(const char *path) {

  file_count++;
  size_t path_len = strlen(path);
  size_t needed = output_len + path_len + 1; // terminator

  if (needed > output_cap) {
    size_t new_cap = output_cap == 0 ? 1024 : output_cap;
    while (new_cap < needed) {
      new_cap *= 2;
    }

    char *new_output = realloc(output, new_cap);
    if (new_output == NULL) {
      return -1;
    }

    output = new_output;
    output_cap = new_cap;
  }

  memcpy(output + output_len, path, path_len);
  output_len += path_len;
  output[output_len] = '\0';
  return 0;
}

void getFilePaths(const char *base) {
  DIR *dir = opendir(base);
  struct stat st;

  if (dir == NULL) {
    perror("opendir failed");
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
        strcmp(entry->d_name, ".git") == 0 ||
        strcmp(entry->d_name, "build") == 0) {
      continue;
    }

    char path[PATH_MAX];
    int written = snprintf(path, sizeof(path), "%s/%s", base, entry->d_name);
    if (written < 0 || (size_t)written >= sizeof(path)) {
      continue;
    }

    if (stat(path, &st) != 0) {
      continue;
    }

    if (S_ISDIR(st.st_mode)) {
      getFilePaths(path);
    } else if (append_path(path) != 0) {
      perror("realloc failed");
      closedir(dir);
      return;
    }
  }

  closedir(dir);
}

const char *get_output(void) { return output; }

long get_file_count(void) { return file_count; }

void clear_file_paths(void) {
  free(output);
  output = NULL;
  output_len = 0;
  output_cap = 0;
  file_count = 0;
}
