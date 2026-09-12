// keccak.h
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>

#ifndef STEGODISK_UTILS_KECCAK_KECCAK_H_
#define STEGODISK_UTILS_KECCAK_KECCAK_H_

#include <stdint.h>
#include <string.h>

#ifndef KECCAK_ROUNDS
#define KECCAK_ROUNDS 24
#endif

#ifndef ROTL64
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))
#endif

// Largest block size any valid digest length produces: 200 - 2 * mdlen with
// mdlen >= 1.
#define KECCAK_MAX_RATE 198

// Computes a keccak hash (md) of the given byte length from "in".
// Returns 0 on success, -1 if the arguments are out of range.
int keccak(const uint8_t *in, int inlen, uint8_t *md, int mdlen);

// update the state
void keccakf(uint64_t st[25], int norounds);

#endif // STEGODISK_UTILS_KECCAK_KECCAK_H_
