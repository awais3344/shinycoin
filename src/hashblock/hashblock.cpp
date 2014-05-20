#include "uint256.h"
#include "util.h"

#include "hashblock.h"
#include "ramhog.h"

uint32_t nShinyScratchpads = 8;
uint32_t nShinyHashChunks = (1 << 24) + (1 << 23) + (1 << 21);
uint32_t nShinyHashIterations = 1024*1024;

uint256 ShinyHash(const char *pbegin, const char *pend)
{
    static CCriticalSection cs_hash;
    static uint64_t **scratchpads = NULL;
    
    LOCK(cs_hash);
    
    if (scratchpads == NULL) {
        printf("Malloc()ing %d scratchpads @ %.2fMB each = %.2fGB RAM...\n",
               nShinyScratchpads, nShinyHashChunks*8/1024.0/1024.0,
               nShinyScratchpads*nShinyHashChunks*8/1024.0/1024.0/1024.0);
        scratchpads = (uint64_t **)malloc(sizeof(uint64_t *) * nShinyScratchpads);
        for (int i=0; i < nShinyScratchpads; i++) {
            scratchpads[i] = (uint64_t *)malloc(sizeof(uint64_t) * nShinyHashChunks);
        }
    }
    
    uint256 outHash;

    ramhog_no_malloc((const uint8_t *)pbegin, (pend - pbegin),
                     (uint8_t *)&outHash, sizeof(outHash),
                     nShinyScratchpads, nShinyHashChunks, nShinyHashIterations,
                     scratchpads);

    return outHash;
}
