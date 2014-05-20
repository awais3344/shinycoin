#ifndef RAMHOG_H
#define RAMHOG_H

#include <inttypes.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void ramhog(const uint8_t *input, size_t input_size, uint8_t *output, size_t output_size,
            uint32_t N, uint32_t C, uint32_t I);

void ramhog_no_malloc(const uint8_t *input, size_t input_size, uint8_t *output, size_t output_size,
                      uint32_t N, uint32_t C, uint32_t I, uint64_t **scratchpads);

#ifdef __cplusplus
}
#endif

#endif