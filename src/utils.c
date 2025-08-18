#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

long load_file(const char *filename, char **res) {
    FILE *fp;
    char *result;
    long file_size;

    if ((fp = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "Failed to open file '%s'.\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    if ((result = malloc(file_size + 1)) == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        fclose(fp);
        return -1;
    }

    if (fread(result, 1, file_size, fp) < (unsigned long)file_size) {
        fprintf(stderr, "Failed to read file '%s'. Read size is less than file size.\n", filename);
        free(result);
        fclose(fp);
        return -1;
    }

    result[file_size] = '\0';

    fclose(fp);

    *res = result;

    return file_size;
}
