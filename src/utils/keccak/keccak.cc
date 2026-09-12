// keccak.cc
// Originally a baseline Keccak implementation by
// Markku-Juhani O. Saarinen <mjos@iki.fi>, 19-Nov-11.
//
// The permutation is now unrolled: Theta keeps the column parities in
// registers, Rho/Pi is expanded from the rotation tables (dropping the
// indirect table lookups and the serial temporary chain) and Chi is expanded
// to remove the modulo indexing. Digests are unchanged; this is the same
// Keccak-256 that Ethereum uses, verified against its published vectors.

#include "keccak.h"

const uint64_t keccakf_rndc[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL};

// Reads a 64-bit little-endian word without assuming pointer alignment or host
// byte order. Compilers lower this to a single load on little-endian targets.
// The previous code cast the input buffer straight to uint64_t*, which is both
// a strict-aliasing violation and an unaligned access.
static uint64_t load64_le(const uint8_t *p) {
  return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) |
         ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32) |
         ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48) |
         ((uint64_t)p[7] << 56);
}

// update the state with given number of rounds

void keccakf(uint64_t st[25], int rounds) {
  uint64_t t, u, b0, b1, b2, b3, b4, c0, c1, c2, c3, c4;
  int round;

  for (round = 0; round < rounds; round++) {

    // Theta, with the column parities held in registers.
    c0 = st[0] ^ st[5] ^ st[10] ^ st[15] ^ st[20];
    c1 = st[1] ^ st[6] ^ st[11] ^ st[16] ^ st[21];
    c2 = st[2] ^ st[7] ^ st[12] ^ st[17] ^ st[22];
    c3 = st[3] ^ st[8] ^ st[13] ^ st[18] ^ st[23];
    c4 = st[4] ^ st[9] ^ st[14] ^ st[19] ^ st[24];

    t = c4 ^ ROTL64(c1, 1);
    st[0] ^= t;
    st[5] ^= t;
    st[10] ^= t;
    st[15] ^= t;
    st[20] ^= t;
    t = c0 ^ ROTL64(c2, 1);
    st[1] ^= t;
    st[6] ^= t;
    st[11] ^= t;
    st[16] ^= t;
    st[21] ^= t;
    t = c1 ^ ROTL64(c3, 1);
    st[2] ^= t;
    st[7] ^= t;
    st[12] ^= t;
    st[17] ^= t;
    st[22] ^= t;
    t = c2 ^ ROTL64(c4, 1);
    st[3] ^= t;
    st[8] ^= t;
    st[13] ^= t;
    st[18] ^= t;
    st[23] ^= t;
    t = c3 ^ ROTL64(c0, 1);
    st[4] ^= t;
    st[9] ^= t;
    st[14] ^= t;
    st[19] ^= t;
    st[24] ^= t;

    // Rho and Pi, unrolled from the rotation/permutation tables.
    t = st[1];
    u = st[10];
    st[10] = ROTL64(t, 1);
    t = u;
    u = st[7];
    st[7] = ROTL64(t, 3);
    t = u;
    u = st[11];
    st[11] = ROTL64(t, 6);
    t = u;
    u = st[17];
    st[17] = ROTL64(t, 10);
    t = u;
    u = st[18];
    st[18] = ROTL64(t, 15);
    t = u;
    u = st[3];
    st[3] = ROTL64(t, 21);
    t = u;
    u = st[5];
    st[5] = ROTL64(t, 28);
    t = u;
    u = st[16];
    st[16] = ROTL64(t, 36);
    t = u;
    u = st[8];
    st[8] = ROTL64(t, 45);
    t = u;
    u = st[21];
    st[21] = ROTL64(t, 55);
    t = u;
    u = st[24];
    st[24] = ROTL64(t, 2);
    t = u;
    u = st[4];
    st[4] = ROTL64(t, 14);
    t = u;
    u = st[15];
    st[15] = ROTL64(t, 27);
    t = u;
    u = st[23];
    st[23] = ROTL64(t, 41);
    t = u;
    u = st[19];
    st[19] = ROTL64(t, 56);
    t = u;
    u = st[13];
    st[13] = ROTL64(t, 8);
    t = u;
    u = st[12];
    st[12] = ROTL64(t, 25);
    t = u;
    u = st[2];
    st[2] = ROTL64(t, 43);
    t = u;
    u = st[20];
    st[20] = ROTL64(t, 62);
    t = u;
    u = st[14];
    st[14] = ROTL64(t, 18);
    t = u;
    u = st[22];
    st[22] = ROTL64(t, 39);
    t = u;
    u = st[9];
    st[9] = ROTL64(t, 61);
    t = u;
    u = st[6];
    st[6] = ROTL64(t, 20);
    t = u;
    u = st[1];
    st[1] = ROTL64(t, 44);
    t = u;

    // Chi, unrolled to drop the modulo indexing.
    b0 = st[0];
    b1 = st[1];
    b2 = st[2];
    b3 = st[3];
    b4 = st[4];
    st[0] ^= (~b1) & b2;
    st[1] ^= (~b2) & b3;
    st[2] ^= (~b3) & b4;
    st[3] ^= (~b4) & b0;
    st[4] ^= (~b0) & b1;
    b0 = st[5];
    b1 = st[6];
    b2 = st[7];
    b3 = st[8];
    b4 = st[9];
    st[5] ^= (~b1) & b2;
    st[6] ^= (~b2) & b3;
    st[7] ^= (~b3) & b4;
    st[8] ^= (~b4) & b0;
    st[9] ^= (~b0) & b1;
    b0 = st[10];
    b1 = st[11];
    b2 = st[12];
    b3 = st[13];
    b4 = st[14];
    st[10] ^= (~b1) & b2;
    st[11] ^= (~b2) & b3;
    st[12] ^= (~b3) & b4;
    st[13] ^= (~b4) & b0;
    st[14] ^= (~b0) & b1;
    b0 = st[15];
    b1 = st[16];
    b2 = st[17];
    b3 = st[18];
    b4 = st[19];
    st[15] ^= (~b1) & b2;
    st[16] ^= (~b2) & b3;
    st[17] ^= (~b3) & b4;
    st[18] ^= (~b4) & b0;
    st[19] ^= (~b0) & b1;
    b0 = st[20];
    b1 = st[21];
    b2 = st[22];
    b3 = st[23];
    b4 = st[24];
    st[20] ^= (~b1) & b2;
    st[21] ^= (~b2) & b3;
    st[22] ^= (~b3) & b4;
    st[23] ^= (~b4) & b0;
    st[24] ^= (~b0) & b1;

    // Iota
    st[0] ^= keccakf_rndc[round];
  }
}

// compute a keccak hash (md) of given byte length from "in"

int keccak(const uint8_t *in, int inlen, uint8_t *md, int mdlen) {
  uint64_t st[25];
  // The rate is 200 - 2 * mdlen bytes, so 198 is the largest block any
  // valid digest length produces. The old buffer was an arbitrary 500.
  uint8_t temp[KECCAK_MAX_RATE];

  int i, rsiz, rsizw;

  // Guards the rate arithmetic below: mdlen outside this range yields a
  // zero or negative block size and would run off the buffer.
  // A null input is only valid for an empty message, where there is nothing
  // to read; an empty std::vector legitimately yields a null data() pointer.
  if ((in == NULL && inlen != 0) || md == NULL || inlen < 0 || mdlen <= 0 ||
      mdlen >= 100)
    return -1;

  rsiz = 200 - 2 * mdlen;
  rsizw = rsiz / 8;

  memset(st, 0, sizeof(st));

  for (; inlen >= rsiz; inlen -= rsiz, in += rsiz) {
    for (i = 0; i < rsizw; i++)
      st[i] ^= load64_le(in + 8 * i);
    keccakf(st, KECCAK_ROUNDS);
  }

  // last block and padding. The length is checked because memcpy declares
  // its source non-null, so copying zero bytes from the null pointer that
  // an empty message legitimately carries is still undefined.
  if (inlen > 0)
    memcpy(temp, in, (size_t)inlen);
  temp[inlen++] = 1;
  memset(temp + inlen, 0, (size_t)(rsiz - inlen));
  temp[rsiz - 1] |= 0x80;

  for (i = 0; i < rsizw; i++)
    st[i] ^= load64_le(temp + 8 * i);

  keccakf(st, KECCAK_ROUNDS);

  // Squeeze, written so the digest does not depend on host byte order.
  for (i = 0; i < mdlen; i++)
    md[i] = (uint8_t)(st[i / 8] >> (8 * (i % 8)));

  return 0;
}
