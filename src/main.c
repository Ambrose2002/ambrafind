
#include <stdio.h>

#define STB_DS_IMPLEMENTATION
#include "../include/index.h"
#include "../lib/stb_ds.h"


int main(void) {
	const char *base = "/Users/ambroseblay/developer/ambrafind";

	build_blob(base);

	#ifdef DEBUG
		printf("--- Debug: Map State ---\n");
	    print_my_map(get_token_map());
	    // printf("--- Blob and Records ---\n");
		// print_blob_and_records(get_output());
	#endif

	clear_file_paths();
	return 0;
}
