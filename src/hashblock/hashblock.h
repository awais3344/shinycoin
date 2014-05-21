#ifndef HASHBLOCK_H
#define HASHBLOCK_H

#include <inttypes.h>

extern uint32_t nShinyScratchpads;
extern uint32_t nShinyHashChunks;
extern uint32_t nShinyHashIterations;

uint256 ShinyHash(const char *pbegin, const char *pend);

#endif // HASHBLOCK_H
