/**
* @file stego_unit_test.cc
* @brief Unit tests for permutations, encoders and their factories
*
* Usage: stego-unit-test <group> [instance]
*   group:    permutations | encoders | factories
*   instance: optional permutation/encoder selector (see the tables below);
*             when omitted every instance of the group is tested.
*
* Exit code is the number of failed checks (0 = success).
*/

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "encoders/encoder.h"
#include "encoders/encoder_factory.h"
#include "encoders/hamming_encoder.h"
#include "encoders/lsb_encoder.h"
#include "hash/hash.h"
#include "hash/keccak_hash_impl.h"
#include "keys/key.h"
#include "utils/keccak/keccak.h"
#include "logging/logger.h"
#include "permutations/permutation.h"
#include "permutations/permutation_factory.h"
#include "utils/exceptions.h"
#include "utils/stego_types.h"

using stego_disk::Encoder;
using stego_disk::EncoderFactory;
using stego_disk::HammingEncoder;
using stego_disk::Hash;
using stego_disk::KeccakHashImpl;
using stego_disk::Key;
using stego_disk::LsbEncoder;
using stego_disk::PermElem;
using stego_disk::Permutation;
using stego_disk::PermutationFactory;
using stego_disk::uint32;
using stego_disk::uint8;

namespace {

int g_failures = 0;
std::string g_context;

// Records a failed check without aborting, so one run reports every failure.
#define EXPECT(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      ++g_failures;                                                            \
      std::cout << "FAIL " << __FILE__ << ':' << __LINE__ << " [" << g_context \
                << "] " << #expr << std::endl;                                 \
    }                                                                          \
  } while (0)

// Passes if evaluating `stmt` throws an exception derived from `type`.
#define EXPECT_THROWS(stmt, type)                                              \
  do {                                                                         \
    bool thrown = false;                                                       \
    try {                                                                      \
      stmt;                                                                    \
    } catch (const type &) {                                                   \
      thrown = true;                                                           \
    } catch (...) {                                                            \
    }                                                                          \
    EXPECT(thrown);                                                            \
  } while (0)

std::mt19937 &Rng() {
  static std::mt19937 rng(20160911u);
  return rng;
}

void FillRandom(std::vector<uint8> *buffer) {
  std::uniform_int_distribution<int> dist(0, 255);
  for (auto &b : *buffer)
    b = static_cast<uint8>(dist(Rng()));
}

int PopCount(uint8 value) {
  int count = 0;
  for (; value; value >>= 1)
    count += value & 1;
  return count;
}

std::string Lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  return s;
}

// ---------------------------------------------------------------- permutations

struct PermutationCase {
  const char *name;
  PermutationFactory::PermutationType type;
};

const PermutationCase kPermutations[] = {
    {"identity", PermutationFactory::PermutationType::IDENTITY},
    {"affine", PermutationFactory::PermutationType::AFFINE},
    {"affine64", PermutationFactory::PermutationType::AFFINE64},
    {"num_feistel", PermutationFactory::PermutationType::FEISTEL_NUM},
    {"mix_feistel", PermutationFactory::PermutationType::FEISTEL_MIX},
};

// Every permutation implementation accepts these sizes (Feistel variants need >= 1024).
const PermElem kPermutationSizes[] = {1024, 4099, 65536, 100003};

void TestPermutationBijection(const PermutationCase &pc, PermElem requested) {
  g_context = std::string("permutation ") + pc.name + " size " +
              std::to_string(requested);

  Key key = Key::FromString("stego-unit-test-key-1");
  auto perm = PermutationFactory::GetPermutation(pc.type);
  EXPECT(perm != nullptr);
  if (!perm)
    return;

  EXPECT(!perm->IsInitialized());
  // An uninitialized permutation reports its state, not a range error.
  EXPECT_THROWS(perm->Permute(0), stego_disk::exception::InvalidState);

  const PermElem announced = perm->GetSizeUsingParams(requested, key);
  perm->Init(requested, key);
  const PermElem size = perm->GetSize();

  EXPECT(perm->IsInitialized());
  EXPECT(size > 0);
  EXPECT(size <= requested);
  EXPECT(size == announced);

  // Bijection on [0, size): every output in range and hit exactly once.
  std::vector<uint8> seen(static_cast<std::size_t>(size), 0);
  bool in_range = true;
  bool unique = true;
  for (PermElem i = 0; i < size; ++i) {
    const PermElem p = perm->Permute(i);
    if (p >= size) {
      in_range = false;
      break;
    }
    if (seen[static_cast<std::size_t>(p)]) {
      unique = false;
      break;
    }
    seen[static_cast<std::size_t>(p)] = 1;
  }
  EXPECT(in_range);
  EXPECT(unique);

  EXPECT_THROWS(perm->Permute(size), std::out_of_range);

  // Deterministic: a second instance with the same key yields the same mapping.
  auto same = PermutationFactory::GetPermutation(pc.type);
  same->Init(requested, key);
  EXPECT(same->GetSize() == size);
  bool identical = true;
  for (PermElem i = 0; i < size && identical; ++i)
    identical = (same->Permute(i) == perm->Permute(i));
  EXPECT(identical);

  // Key sensitivity: a different key changes the mapping (identity ignores keys).
  if (pc.type != PermutationFactory::PermutationType::IDENTITY) {
    Key other_key = Key::FromString("stego-unit-test-key-2");
    auto other = PermutationFactory::GetPermutation(pc.type);
    other->Init(requested, other_key);
    bool differs = (other->GetSize() != size);
    for (PermElem i = 0;
         i < std::min<PermElem>(size, other->GetSize()) && !differs; ++i)
      differs = (other->Permute(i) != perm->Permute(i));
    EXPECT(differs);
  }
}

void TestPermutationArguments(const PermutationCase &pc) {
  g_context = std::string("permutation ") + pc.name + " arguments";
  Key key = Key::FromString("stego-unit-test-key-1");

  auto perm = PermutationFactory::GetPermutation(pc.type);
  EXPECT_THROWS(perm->Init(0, key), std::exception);

  const bool feistel =
      pc.type == PermutationFactory::PermutationType::FEISTEL_NUM ||
      pc.type == PermutationFactory::PermutationType::FEISTEL_MIX;
  if (feistel) {
    auto small = PermutationFactory::GetPermutation(pc.type);
    EXPECT_THROWS(small->Init(16, key), std::invalid_argument);

    Key empty_key;
    auto unkeyed = PermutationFactory::GetPermutation(pc.type);
    EXPECT_THROWS(unkeyed->Init(4096, empty_key),
                  stego_disk::exception::EmptyArgument);
  }
}

void TestPermutation(const PermutationCase &pc) {
  for (PermElem size : kPermutationSizes)
    TestPermutationBijection(pc, size);
  TestPermutationArguments(pc);
}

// -------------------------------------------------------------------- encoders

void TestEncoderRoundTrip(const std::shared_ptr<Encoder> &encoder,
                          const std::string &label) {
  g_context = "encoder " + label + " round trip";
  const uint32 data_size = encoder->GetDataBlockSize();
  const uint32 codeword_size = encoder->GetCodewordBlockSize();
  EXPECT(data_size > 0);
  EXPECT(codeword_size >= data_size);

  std::vector<uint8> data(data_size), codeword(codeword_size),
      extracted(data_size);
  for (int trial = 0; trial < 50; ++trial) {
    FillRandom(&data);
    FillRandom(&codeword);
    EXPECT(encoder->Embed(codeword.data(), data.data()) == 0);
    EXPECT(encoder->Extract(codeword.data(), extracted.data()) == 0);
    EXPECT(extracted == data);
  }

  // Embedding into an all-zero and an all-ones carrier must also round-trip.
  for (uint8 fill : {static_cast<uint8>(0x00), static_cast<uint8>(0xFF)}) {
    FillRandom(&data);
    std::fill(codeword.begin(), codeword.end(), fill);
    encoder->Embed(codeword.data(), data.data());
    encoder->Extract(codeword.data(), extracted.data());
    EXPECT(extracted == data);
  }

  g_context = "encoder " + label + " null arguments";
  EXPECT_THROWS(encoder->Embed(nullptr, data.data()),
                stego_disk::exception::NullptrArgument);
  EXPECT_THROWS(encoder->Embed(codeword.data(), nullptr),
                stego_disk::exception::NullptrArgument);
  EXPECT_THROWS(encoder->Extract(nullptr, extracted.data()),
                stego_disk::exception::NullptrArgument);
  EXPECT_THROWS(encoder->Extract(codeword.data(), nullptr),
                stego_disk::exception::NullptrArgument);
}

void TestLsbEncoder() {
  for (uint32 block = LsbEncoder::GetBlockSizeMin();
       block <= LsbEncoder::GetBlockSizeMax(); block <<= 1) {
    auto encoder = std::make_shared<LsbEncoder>(block);
    const std::string label = "lsb/" + std::to_string(block);
    TestEncoderRoundTrip(encoder, label);

    g_context = "encoder " + label + " block sizes";
    EXPECT(encoder->GetDataBlockSize() == block);
    EXPECT(encoder->GetCodewordBlockSize() == block);

    // LSB is a plain copy: after Embed the codeword equals the data.
    std::vector<uint8> data(block), codeword(block, 0xA5);
    FillRandom(&data);
    encoder->Embed(codeword.data(), data.data());
    EXPECT(codeword == data);
  }

  g_context = "encoder lsb arguments";
  EXPECT_THROWS(LsbEncoder(3), std::invalid_argument); // not a power of two
  EXPECT_THROWS(LsbEncoder(LsbEncoder::GetBlockSizeMax() * 2),
                std::invalid_argument);

  auto encoder = EncoderFactory::GetEncoder(EncoderFactory::EncoderType::LSB);
  // Called directly, bypassing the factory's own argument validation.
  EXPECT_THROWS(encoder->SetArgByName("", "1"),
                stego_disk::exception::EmptyArgument);
  EXPECT_THROWS(encoder->SetArgByName("blockSize", ""),
                stego_disk::exception::EmptyArgument);
  EncoderFactory::SetEncoderArg(encoder, "blockSize", "8");
  EXPECT(encoder->GetDataBlockSize() == 8);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "blockSize", "6"),
                std::invalid_argument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "noSuchParam", "1"),
                std::invalid_argument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "", "1"),
                stego_disk::exception::EmptyArgument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "blockSize", ""),
                stego_disk::exception::EmptyArgument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(nullptr, "blockSize", "1"),
                stego_disk::exception::NullptrArgument);
  EXPECT(Lower(encoder->GetNameInstance()) == Lower(LsbEncoder::GetName()));
  EXPECT(encoder->GetNewInstance() != nullptr);
}

void TestHammingEncoder() {
  for (int parity = HammingEncoder::GetParityBitsMin();
       parity <= HammingEncoder::GetParityBitsMax(); ++parity) {
    auto encoder =
        std::make_shared<HammingEncoder>(static_cast<uint32>(parity));
    const std::string label = "hamming/" + std::to_string(parity);
    TestEncoderRoundTrip(encoder, label);

    // Geometry implied by the (2^p - 1, 2^p - 1 - p) Hamming matrix embedding:
    // each codeword carries `parity` data bits and occupies ceil((2^p - 1) / 8) bytes.
    g_context = "encoder " + label + " block sizes";
    const uint32 data_bits = encoder->GetDataBlockSize() * 8;
    EXPECT(data_bits % static_cast<uint32>(parity) == 0);
    const uint32 codewords = data_bits / static_cast<uint32>(parity);
    const uint32 codeword_bytes = ((1u << parity) - 2) / 8 + 1;
    EXPECT(encoder->GetCodewordBlockSize() == codewords * codeword_bytes);

    // Matrix embedding changes at most one bit per codeword.
    g_context = "encoder " + label + " embedding distortion";
    std::vector<uint8> data(encoder->GetDataBlockSize());
    std::vector<uint8> before(encoder->GetCodewordBlockSize());
    for (int trial = 0; trial < 50; ++trial) {
      FillRandom(&data);
      FillRandom(&before);
      std::vector<uint8> after = before;
      encoder->Embed(after.data(), data.data());
      bool at_most_one_bit = true;
      for (uint32 c = 0; c < codewords && at_most_one_bit; ++c) {
        int changed = 0;
        for (uint32 b = 0; b < codeword_bytes; ++b)
          changed += PopCount(static_cast<uint8>(
              before[c * codeword_bytes + b] ^ after[c * codeword_bytes + b]));
        at_most_one_bit = (changed <= 1);
      }
      EXPECT(at_most_one_bit);
    }
  }

  g_context = "encoder hamming arguments";
  EXPECT_THROWS(HammingEncoder(HammingEncoder::GetParityBitsMin() - 1),
                std::out_of_range);
  EXPECT_THROWS(HammingEncoder(HammingEncoder::GetParityBitsMax() + 1),
                std::out_of_range);

  auto encoder =
      EncoderFactory::GetEncoder(EncoderFactory::EncoderType::HAMMING);
  const uint32 default_data_block = encoder->GetDataBlockSize();
  EncoderFactory::SetEncoderArg(encoder, "parityBits", "3");
  EXPECT(encoder->GetDataBlockSize() == 3); // lcm(3, 8) / 8
  EXPECT(encoder->GetDataBlockSize() != default_data_block);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "parityBits", "99"),
                std::out_of_range);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "parityBits", "abc"),
                std::invalid_argument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "noSuchParam", "1"),
                std::invalid_argument);
  EXPECT(Lower(encoder->GetNameInstance()) == Lower(HammingEncoder::GetName()));
  EXPECT(encoder->GetNewInstance() != nullptr);
}

// ------------------------------------------------------------------- factories

void TestFactories() {
  g_context = "encoder factory";
  EXPECT(EncoderFactory::GetEncoderType("lsb") ==
         EncoderFactory::EncoderType::LSB);
  EXPECT(EncoderFactory::GetEncoderType("HAMMING") ==
         EncoderFactory::EncoderType::HAMMING);
  EXPECT(EncoderFactory::GetEncoderType("bogus") ==
         EncoderFactory::GetDefaultEncoderType());
  EXPECT(EncoderFactory::GetEncoderName(EncoderFactory::EncoderType::LSB) ==
         "lsb");
  EXPECT(EncoderFactory::GetEncoderName(EncoderFactory::EncoderType::HAMMING) ==
         "hamming");
  EXPECT(EncoderFactory::GetEncoders().size() == 2);
  EXPECT(EncoderFactory::GetEncoderNames().size() == 2);
  EXPECT(EncoderFactory::GetDefaultEncoder() != nullptr);
  const std::size_t expected_all =
      static_cast<std::size_t>(HammingEncoder::GetParityBitsMax() -
                               HammingEncoder::GetParityBitsMin() + 1) +
      11; // LSB 1..1024
  EXPECT(EncoderFactory::GetAllEncoders().size() == expected_all);
  for (const auto &encoder : EncoderFactory::GetAllEncoders())
    EXPECT(encoder != nullptr && encoder->GetDataBlockSize() > 0);

  g_context = "permutation factory";
  const auto all = PermutationFactory::GetPermutations();
  EXPECT(all.size() == 5);
  for (const auto &perm : all) {
    EXPECT(perm != nullptr);
    if (!perm)
      continue;
    // Lookup by the instance's own code name must return the same kind.
    auto by_name = PermutationFactory::GetPermutation(perm->GetNameInstance());
    EXPECT(by_name != nullptr);
    if (by_name)
      EXPECT(by_name->GetNameInstance() == perm->GetNameInstance());
  }
  EXPECT_THROWS(
      PermutationFactory::GetPermutation(std::string("no-such-permutation")),
      std::invalid_argument);
  EXPECT_THROWS(PermutationFactory::GetPermutation(std::string("")),
                stego_disk::exception::EmptyArgument);
  EXPECT(PermutationFactory::GetDefaultPermutation() != nullptr);
  // Type <-> type-name round trip. Note: type names ("num_feistel") and instance
  // code names ("NumericFeistel") are separate namespaces.
  for (const auto &pc : kPermutations) {
    EXPECT(PermutationFactory::GetPermutation(pc.type) != nullptr);
    EXPECT(PermutationFactory::GetPermutationName(pc.type) == pc.name);
    EXPECT(PermutationFactory::GetPermutationType(pc.name) == pc.type);
  }
  EXPECT(PermutationFactory::GetPermutationType("bogus") ==
         PermutationFactory::GetDefaultPermutationType());

  g_context = "key";
  Key a = Key::FromString("password");
  Key b = Key::FromString("password");
  Key c = Key::FromString("Password");
  EXPECT(a.GetSize() > 0);
  EXPECT(a.GetSize() == b.GetSize());
  EXPECT(memcmp(a.GetData().GetConstRawPointer(),
                b.GetData().GetConstRawPointer(), a.GetSize()) == 0);
  EXPECT(memcmp(a.GetData().GetConstRawPointer(),
                c.GetData().GetConstRawPointer(), a.GetSize()) != 0);
  EXPECT(Key().GetSize() == 0);
}

// ------------------------------------------------------------------------ hash

// Digests are storage format: every existing volume's keys and permutations
// derive from them, so these values must never change. They are the original
// Keccak (0x01 padding) at a 32-byte digest, i.e. the same function Ethereum
// calls keccak256; the first two entries are its published vectors.
struct HashVector {
  const char *name;
  int length;
  const char *digest;
};

const HashVector kHashVectors[] = {
    {"empty", 0,
     "c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470"},
    {"abc", 3,
     "4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45"},
    {"rate_minus_1", 135,
     "aba4a58b3b2fcbcdc90624cc76fb6acc522abffc59c25642a5aaaafd00469e8b"},
    {"exact_rate", 136,
     "3b3e38f46f9e12a80496894e32e624903a092a42d2096603b4d8151235a56795"},
    {"rate_plus_1", 137,
     "0f018f4a7d578f411e6f2a380295e8abff3ba307c4a497253af577d0fb3d7592"},
    {"long", 1000,
     "c77d9bffcae9f0984e6dff7eea63cc14cad5f367f791e27b08a1953f192f30a5"},
};

// Reproduces the inputs the vectors above were generated from.
std::vector<uint8> HashVectorInput(const HashVector &vector) {
  const std::string name(vector.name);
  if (name == "abc")
    return {'a', 'b', 'c'};
  if (name == "long") {
    std::vector<uint8> data(static_cast<std::size_t>(vector.length));
    for (std::size_t i = 0; i < data.size(); ++i)
      data[i] = static_cast<uint8>(i * 31 + 7);
    return data;
  }
  return std::vector<uint8>(static_cast<std::size_t>(vector.length), 0xAA);
}

std::string ToHex(const uint8 *data, std::size_t length) {
  static const char kDigits[] = "0123456789abcdef";
  std::string hex;
  for (std::size_t i = 0; i < length; ++i) {
    hex.push_back(kDigits[data[i] >> 4]);
    hex.push_back(kDigits[data[i] & 0x0F]);
  }
  return hex;
}

void TestHash() {
  for (const auto &vector : kHashVectors) {
    g_context = std::string("keccak vector ") + vector.name;
    const std::vector<uint8> input = HashVectorInput(vector);
    EXPECT(static_cast<int>(input.size()) == vector.length);
    // An empty vector's data() may be null, which the Hash wrapper rejects.
    const uint8 empty = 0;
    const uint8 *bytes = input.empty() ? &empty : input.data();

    uint8 digest[32] = {};
    EXPECT(keccak(bytes, vector.length, digest, sizeof(digest)) == 0);
    EXPECT(ToHex(digest, sizeof(digest)) == std::string(vector.digest));

    // The Hash wrapper must produce the same bytes as the raw function.
    Hash hash(bytes, input.size());
    EXPECT(hash.GetStateSize() == sizeof(digest));
    EXPECT(ToHex(hash.GetState().GetConstRawPointer(), hash.GetStateSize()) ==
           std::string(vector.digest));
  }

  g_context = "keccak arguments";
  uint8 digest[32] = {};
  const uint8 data[1] = {0};
  EXPECT(keccak(nullptr, 0, digest, sizeof(digest)) == 0); // empty message
  EXPECT(keccak(nullptr, 1, digest, sizeof(digest)) == -1);
  EXPECT(keccak(data, 1, nullptr, sizeof(digest)) == -1);
  EXPECT(keccak(data, -1, digest, sizeof(digest)) == -1);
  EXPECT(keccak(data, 1, digest, 0) == -1);
  EXPECT(keccak(data, 1, digest, 100) == -1);

  g_context = "keccak state size";
  EXPECT_THROWS(KeccakHashImpl(0), std::invalid_argument);
  EXPECT_THROWS(KeccakHashImpl(30),
                std::invalid_argument); // rate not word aligned
  EXPECT_THROWS(KeccakHashImpl(100), std::invalid_argument);
}

int Usage(const char *name) {
  std::cerr << "Usage: " << name
            << " <permutations|encoders|factories> [instance]\n"
            << "  permutations instance: identity affine affine64 num_feistel "
               "mix_feistel\n"
            << "  encoders instance:     lsb hamming\n"
            << "  hash and factories take no instance\n";
  return 2;
}

} // namespace

int main(int argc, char *argv[]) {
  std::cout << "CTEST_FULL_OUTPUT" << std::endl;
  std::string logging_level("ERROR");
  Logger::SetVerbosityLevel(logging_level, std::string("cout"));

  if (argc < 2)
    return Usage(argv[0]);
  const std::string group = Lower(argv[1]);
  const std::string instance = argc > 2 ? Lower(argv[2]) : "";

  if (group == "permutations") {
    bool matched = false;
    for (const auto &pc : kPermutations) {
      if (!instance.empty() && instance != pc.name)
        continue;
      matched = true;
      TestPermutation(pc);
    }
    if (!matched)
      return Usage(argv[0]);
  } else if (group == "encoders") {
    if (instance.empty() || instance == "lsb")
      TestLsbEncoder();
    if (instance.empty() || instance == "hamming")
      TestHammingEncoder();
    if (!instance.empty() && instance != "lsb" && instance != "hamming")
      return Usage(argv[0]);
  } else if (group == "factories") {
    TestFactories();
  } else if (group == "hash") {
    TestHash();
  } else {
    return Usage(argv[0]);
  }

  if (g_failures == 0) {
    std::cout << "OK: " << group << (instance.empty() ? "" : " " + instance)
              << std::endl;
  } else {
    std::cout << g_failures << " check(s) failed" << std::endl;
  }
  return g_failures;
}
