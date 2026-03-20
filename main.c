#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp;

    char output_buffer[128];

    const char *command = "ls";

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return EXIT_FAILURE;
    }

    while (fgets(output_buffer, sizeof(output_buffer), fp) != NULL) {
        printf("Command Output: %s", output_buffer);
    }

    pclose(fp);
    
    return 0;
}