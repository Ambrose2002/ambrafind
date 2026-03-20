
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

int main() {
    FILE *fp;
    struct stat st;

    char output_buffer[128];

    const char *command = "ls";

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return EXIT_FAILURE;
    }

    while (fgets(output_buffer, sizeof(output_buffer), fp) != NULL) {
        char base[] = "/Users/ambroseblay/Developer/ambrafind/";
        char path[512];

        output_buffer[strcspn(output_buffer, "\n")] = '\0'; 
        snprintf(path, sizeof(path), "%s%s", base, output_buffer);

        if (stat(path, &st) == 0) {
            printf("%s: %lld bytes\n", output_buffer, (long long)st.st_size);
        }
    }

    pclose(fp);
    
    return 0;
}