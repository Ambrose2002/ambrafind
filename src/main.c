
#include <stdio.h>

#define STB_DS_IMPLEMENTATION
#include "../lib/stb_ds.h"
#include "../include/index.h"

int main(void) {
  const char *base = "/Users/ambroseblay/developer/ambrafind";

  build_blob(base);

  print_blob_and_records(get_output());

  clear_file_paths();
  return 0;
}
