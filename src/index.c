
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../include/index.h"

static char *output = NULL;
static size_t output_len = 0;
static size_t output_cap = 0;
static FileRecord *file_records = NULL;
static size_t file_records_len = 0;
static size_t file_records_cap = 0;

static int append_string(const char *value, uint32_t *offset) {
  size_t value_len = strlen(value);
  size_t needed = output_len + value_len + 1; // terminator

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

  if (offset != NULL) {
    if (output_len > UINT32_MAX) {
      return -1;
    }
    *offset = (uint32_t)output_len;
  }

  memcpy(output + output_len, value, value_len + 1);
  output_len += value_len + 1;
  return 0;
}

static int append_file_record(const FileRecord *record) {
  if (file_records_len == file_records_cap) {
    size_t new_cap = file_records_cap == 0 ? 128 : file_records_cap * 2;
    FileRecord *new_records = realloc(file_records, new_cap * sizeof(FileRecord));
    if (new_records == NULL) {
      return -1;
    }

    file_records = new_records;
    file_records_cap = new_cap;
  }

  file_records[file_records_len++] = *record;
  return 0;
}

void buildBlob(const char *base) {
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
      buildBlob(path);
    } else {
      const char *filename = entry->d_name;
      const char *dot = strrchr(filename, '.');
      const char *ext =
          (dot != NULL && dot != filename && dot[1] != '\0') ? dot + 1 : "";

      FileRecord fr = {0};
      int code = append_string(filename, &fr.filename_offset);
      if (code != 0) {
        perror("realloc failed");
        closedir(dir);
        return;
      }

      code = append_string(path, &fr.path_offset);
      if (code != 0) {
        perror("realloc failed");
        closedir(dir);
        return;
      }

      code = append_string(ext, &fr.ext_offset);
      if (code != 0) {
        perror("realloc failed");
        closedir(dir);
        return;
      }

      fr.size = (uint64_t)st.st_size;
      fr.mtime = (int64_t)st.st_mtime;
      code = append_file_record(&fr);
      if (code != 0) {
        perror("realloc failed");
        closedir(dir);
        return;
      }
    }
  }

  closedir(dir);
}

const char *get_output(void) { return output; }

const FileRecord *get_file_records(void) { return file_records; }

long get_file_count(void) { return (long)file_records_len; }

void clear_file_paths(void) {
  free(output);
  free(file_records);
  output = NULL;
  file_records = NULL;
  output_len = 0;
  output_cap = 0;
  file_records_len = 0;
  file_records_cap = 0;
}
