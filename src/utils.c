#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

long load_file(const char* filename, char** res) {
    FILE* fp;
    char* result;
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

int compress_chunk_data(const uint8_t* input, const size_t input_size, uint8_t** output,
                        size_t* output_size) {
    *output_size = compressBound(input_size);
    *output = malloc(*output_size);

    if (!*output) return Z_MEM_ERROR;

    int rc = compress(*output, output_size, input, input_size);
    if (rc != Z_OK) {
        free(*output);
        *output = NULL;
        *output_size = 0;
    }
    return rc;
}

int decompress_chunk_data(const uint8_t* input, const size_t input_size, uint8_t** output,
                          const size_t original_size) {
    *output = malloc(original_size);
    if (!*output) return Z_MEM_ERROR;

    size_t dest_len = original_size;
    int rc = uncompress(*output, &dest_len, input, input_size);

    if (rc != Z_OK) {
        free(*output);
        *output = NULL;
    }

    return rc;
}
