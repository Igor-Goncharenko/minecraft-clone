#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

long load_file(const char* filename, char** res);

int compress_data(const uint8_t* input, const size_t input_size, uint8_t** output,
                  size_t* output_size);

int decompress_data(const uint8_t* input, const size_t input_size, uint8_t* output,
                    const size_t original_size);

#endif /* UTILS_H */
