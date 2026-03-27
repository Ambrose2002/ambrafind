
#include <stdio.h>

#include "index.h"

int main(void) {
    const char *base = "/Users/ambroseblay/developer/ambrafind";
    buildBlob(base);
    if (get_output() != NULL) {
        printf("%ld files found\n", get_file_count());
    }
    clear_file_paths();
    return 0;
}
