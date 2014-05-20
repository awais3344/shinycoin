#include <memory.h>

#include "ramhog.h"
#include "pbkdf2.h"


typedef struct {
    uint64_t s[64];
    uint8_t p;
    uint64_t last;
} xorshift_ctx;

static inline uint64_t xorshift_next(xorshift_ctx *pctx) {
    uint64_t s0 = pctx->s[ pctx->p ];
    uint64_t s1 = pctx->s[ pctx->p = ( pctx->p + 1 ) & 63 ];
    s1 ^= s1 << 25; // a
    s1 ^= s1 >> 3;  // b
    s0 ^= s0 >> 49; // c
    pctx->s[ pctx->p ] = s0 ^ s1;
    return ( pctx->s[ pctx->p ] = s0 ^ s1 ) * 8372773778140471301LL;
}

static void xorshift_pbkdf2_seed(xorshift_ctx *ctx, const uint8_t *seed, size_t seedlen, const uint8_t *salt, size_t saltlen) {
    uint64_t fullSeed[65];
    
    PBKDF2_SHA256(seed, seedlen, salt, saltlen, 128, (uint8_t *)fullSeed, sizeof(uint64_t)*65);
    
    memcpy(ctx->s, &fullSeed[0], sizeof(uint64_t)*64);
    ctx->p = (uint8_t)(fullSeed[16] & 63);
}

void ramhog_no_malloc(const uint8_t *input, size_t input_size, uint8_t *output, size_t output_size,
                      uint32_t N, uint32_t C, uint32_t I, uint64_t **scratchpads) {
    xorshift_ctx ctx;
    uint32_t i, padIndex, chunk;
    
    for (padIndex=0; padIndex < N; padIndex++) {
        uint64_t *padOut = scratchpads[padIndex];
        
        xorshift_pbkdf2_seed(&ctx, input, input_size, (uint8_t *)&padIndex, 4);
        
        padOut[0] = xorshift_next(&ctx);
        padOut[1] = xorshift_next(&ctx);
        
        for (chunk=2; chunk < C; chunk++) {
            padOut[chunk] = xorshift_next(&ctx);
            if (!(padOut[chunk] & 31)) {
                padOut[chunk] ^= padOut[xorshift_next(&ctx) % (chunk/2) + chunk/2];
            }
        }
    }
    
    uint64_t finalChunks[N];
    for (padIndex=0; padIndex < N; padIndex++) {
        finalChunks[padIndex] = scratchpads[padIndex][C - 1];
    }
    
    xorshift_pbkdf2_seed(&ctx, input, input_size, (uint8_t *)&finalChunks[0], sizeof(uint64_t) * N);
    
    uint64_t X = xorshift_next(&ctx);
    
    for (i=0; i < I - 16; i++) {
        X = scratchpads[(X >> 32) % N][(X & 0x00000000ffffffffL) % C] ^ xorshift_next(&ctx);
    }
    uint64_t finalXs[16];
    for (i=0; i < 16; i++) {
        X = scratchpads[(X >> 32) % N][(X & 0x00000000ffffffffL) % C] ^ xorshift_next(&ctx);
        finalXs[i] = X;
    }
    
    PBKDF2_SHA256(input, input_size,
                  (uint8_t *)finalXs, sizeof(uint64_t)*16,
                  1, (uint8_t *)output, output_size);
}

void ramhog(const uint8_t *input, size_t input_size, uint8_t *output, size_t output_size,
            uint32_t N, uint32_t C, uint32_t I) {
    uint32_t i;
    uint64_t *scratchpads[N];
    
    for (i=0; i < N; i++) {
        scratchpads[i] = (uint64_t *)malloc(sizeof(uint64_t) * C);
    }
    
    ramhog_no_malloc(input, input_size, output, output_size, N, C, I, scratchpads);
    
    for (i=0; i < N; i++) {
        free(scratchpads[i]);
    }
}



