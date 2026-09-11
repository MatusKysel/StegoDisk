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
#include "keys/key.h"
#include "logging/logger.h"
#include "permutations/permutation.h"
#include "permutations/permutation_factory.h"
#include "utils/exceptions.h"
#include "utils/stego_types.h"

using stego_disk::Encoder;
using stego_disk::EncoderFactory;
using stego_disk::HammingEncoder;
using stego_disk::Key;
using stego_disk::LsbEncoder;
using stego_disk::PermElem;
using stego_disk::Permutation;
using stego_disk::PermutationFactory;
using stego_disk::uint8;
using stego_disk::uint32;

namespace {

int g_failures = 0;
std::string g_context;

// Records a failed check without aborting, so one run reports every failure.
#define EXPECT(expr)                                                        \
  do {                                                                      \
    if (!(expr)) {                                                          \
      ++g_failures;                                                         \
      std::cout << "FAIL " << __FILE__ << ':' << __LINE__ << " [" << g_context \
                << "] " << #expr << std::endl;                              \
    }                                                                       \
  } while (0)

// Passes if evaluating `stmt` throws an exception derived from `type`.
#define EXPECT_THROWS(stmt, type)                                           \
  do {                                                                      \
    bool thrown = false;                                                    \
    try { stmt; } catch (const type &) { thrown = true; } catch (...) {}    \
    EXPECT(thrown);                                                         \
  } while (0)

std::mt19937 &Rng() {
  static std::mt19937 rng(20160911u);
  return rng;
}

void FillRandom(std::vector<uint8> *buffer) {
  std::uniform_int_distribution<int> dist(0, 255);
  for (auto &b : *buffer) b = static_cast<uint8>(dist(Rng()));
}

int PopCount(uint8 value) {
  int count = 0;
  for (; value; value >>= 1) count += value & 1;
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
  {"identity",    PermutationFactory::PermutationType::IDENTITY},
  {"affine",      PermutationFactory::PermutationType::AFFINE},
  {"affine64",    PermutationFactory::PermutationType::AFFINE64},
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
  if (!perm) return;

  EXPECT(!perm->IsInitialized());
  EXPECT_THROWS(perm->Permute(0), std::exception);

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
    if (p >= size) { in_range = false; break; }
    if (seen[static_cast<std::size_t>(p)]) { unique = false; break; }
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
    for (PermElem i = 0; i < std::min<PermElem>(size, other->GetSize()) && !differs; ++i)
      differs = (other->Permute(i) != perm->Permute(i));
    EXPECT(differs);
  }
}

void TestPermutationArguments(const PermutationCase &pc) {
  g_context = std::string("permutation ") + pc.name + " arguments";
  Key key = Key::FromString("stego-unit-test-key-1");

  auto perm = PermutationFactory::GetPermutation(pc.type);
  EXPECT_THROWS(perm->Init(0, key), std::exception);

  const bool feistel = pc.type == PermutationFactory::PermutationType::FEISTEL_NUM ||
                       pc.type == PermutationFactory::PermutationType::FEISTEL_MIX;
  if (feistel) {
    auto small = PermutationFactory::GetPermutation(pc.type);
    EXPECT_THROWS(small->Init(16, key), std::invalid_argument);

    Key empty_key;
    auto unkeyed = PermutationFactory::GetPermutation(pc.type);
    EXPECT_THROWS(unkeyed->Init(4096, empty_key), stego_disk::exception::EmptyArgument);
  }
}

void TestPermutation(const PermutationCase &pc) {
  for (PermElem size : kPermutationSizes) TestPermutationBijection(pc, size);
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

  std::vector<uint8> data(data_size), codeword(codeword_size), extracted(data_size);
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
  EXPECT_THROWS(encoder->Embed(nullptr, data.data()), stego_disk::exception::NullptrArgument);
  EXPECT_THROWS(encoder->Embed(codeword.data(), nullptr), stego_disk::exception::NullptrArgument);
  EXPECT_THROWS(encoder->Extract(nullptr, extracted.data()), stego_disk::exception::NullptrArgument);
  EXPECT_THROWS(encoder->Extract(codeword.data(), nullptr), stego_disk::exception::NullptrArgument);
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
  EXPECT_THROWS(LsbEncoder(3), std::invalid_argument);           // not a power of two
  EXPECT_THROWS(LsbEncoder(LsbEncoder::GetBlockSizeMax() * 2), std::invalid_argument);

  auto encoder = EncoderFactory::GetEncoder(EncoderFactory::EncoderType::LSB);
  EncoderFactory::SetEncoderArg(encoder, "blockSize", "8");
  EXPECT(encoder->GetDataBlockSize() == 8);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "blockSize", "6"), std::invalid_argument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "noSuchParam", "1"), std::invalid_argument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "", "1"), stego_disk::exception::EmptyArgument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "blockSize", ""), stego_disk::exception::EmptyArgument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(nullptr, "blockSize", "1"), stego_disk::exception::NullptrArgument);
  EXPECT(Lower(encoder->GetNameInstance()) == Lower(LsbEncoder::GetName()));
  EXPECT(encoder->GetNewInstance() != nullptr);
}

void TestHammingEncoder() {
  for (int parity = HammingEncoder::GetParityBitsMin();
       parity <= HammingEncoder::GetParityBitsMax(); ++parity) {
    auto encoder = std::make_shared<HammingEncoder>(static_cast<uint32>(parity));
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
          changed += PopCount(static_cast<uint8>(before[c * codeword_bytes + b] ^
                                                 after[c * codeword_bytes + b]));
        at_most_one_bit = (changed <= 1);
      }
      EXPECT(at_most_one_bit);
    }
  }

  g_context = "encoder hamming arguments";
  EXPECT_THROWS(HammingEncoder(HammingEncoder::GetParityBitsMin() - 1), std::out_of_range);
  EXPECT_THROWS(HammingEncoder(HammingEncoder::GetParityBitsMax() + 1), std::out_of_range);

  auto encoder = EncoderFactory::GetEncoder(EncoderFactory::EncoderType::HAMMING);
  const uint32 default_data_block = encoder->GetDataBlockSize();
  EncoderFactory::SetEncoderArg(encoder, "parityBits", "3");
  EXPECT(encoder->GetDataBlockSize() == 3);                    // lcm(3, 8) / 8
  EXPECT(encoder->GetDataBlockSize() != default_data_block);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "parityBits", "99"), std::out_of_range);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "parityBits", "abc"), std::invalid_argument);
  EXPECT_THROWS(EncoderFactory::SetEncoderArg(encoder, "noSuchParam", "1"), std::invalid_argument);
  EXPECT(Lower(encoder->GetNameInstance()) == Lower(HammingEncoder::GetName()));
  EXPECT(encoder->GetNewInstance() != nullptr);
}

// ------------------------------------------------------------------- factories

void TestFactories() {
  g_context = "encoder factory";
  EXPECT(EncoderFactory::GetEncoderType("lsb") == EncoderFactory::EncoderType::LSB);
  EXPECT(EncoderFactory::GetEncoderType("HAMMING") == EncoderFactory::EncoderType::HAMMING);
  EXPECT(EncoderFactory::GetEncoderType("bogus") == EncoderFactory::GetDefaultEncoderType());
  EXPECT(EncoderFactory::GetEncoderName(EncoderFactory::EncoderType::LSB) == "lsb");
  EXPECT(EncoderFactory::GetEncoderName(EncoderFactory::EncoderType::HAMMING) == "hamming");
  EXPECT(EncoderFactory::GetEncoders().size() == 2);
  EXPECT(EncoderFactory::GetEncoderNames().size() == 2);
  EXPECT(EncoderFactory::GetDefaultEncoder() != nullptr);
  const std::size_t expected_all =
      static_cast<std::size_t>(HammingEncoder::GetParityBitsMax() -
                               HammingEncoder::GetParityBitsMin() + 1) + 11; // LSB 1..1024
  EXPECT(EncoderFactory::GetAllEncoders().size() == expected_all);
  for (const auto &encoder : EncoderFactory::GetAllEncoders())
    EXPECT(encoder != nullptr && encoder->GetDataBlockSize() > 0);

  g_context = "permutation factory";
  const auto all = PermutationFactory::GetPermutations();
  EXPECT(all.size() == 5);
  for (const auto &perm : all) {
    EXPECT(perm != nullptr);
    if (!perm) continue;
    // Lookup by the instance's own code name must return the same kind.
    auto by_name = PermutationFactory::GetPermutation(perm->GetNameInstance());
    EXPECT(by_name != nullptr);
    if (by_name) EXPECT(by_name->GetNameInstance() == perm->GetNameInstance());
  }
  EXPECT_THROWS(PermutationFactory::GetPermutation(std::string("no-such-permutation")),
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
  EXPECT(memcmp(a.GetData().GetConstRawPointer(), b.GetData().GetConstRawPointer(), a.GetSize()) == 0);
  EXPECT(memcmp(a.GetData().GetConstRawPointer(), c.GetData().GetConstRawPointer(), a.GetSize()) != 0);
  EXPECT(Key().GetSize() == 0);
}

int Usage(const char *name) {
  std::cerr << "Usage: " << name << " <permutations|encoders|factories> [instance]\n"
            << "  permutations instance: identity affine affine64 num_feistel mix_feistel\n"
            << "  encoders instance:     lsb hamming\n";
  return 2;
}

} // namespace

int main(int argc, char *argv[]) {
  std::cout << "CTEST_FULL_OUTPUT" << std::endl;
  std::string logging_level("ERROR");
  Logger::SetVerbosityLevel(logging_level, std::string("cout"));

  if (argc < 2) return Usage(argv[0]);
  const std::string group = Lower(argv[1]);
  const std::string instance = argc > 2 ? Lower(argv[2]) : "";

  if (group == "permutations") {
    bool matched = false;
    for (const auto &pc : kPermutations) {
      if (!instance.empty() && instance != pc.name) continue;
      matched = true;
      TestPermutation(pc);
    }
    if (!matched) return Usage(argv[0]);
  } else if (group == "encoders") {
    if (instance.empty() || instance == "lsb") TestLsbEncoder();
    if (instance.empty() || instance == "hamming") TestHammingEncoder();
    if (!instance.empty() && instance != "lsb" && instance != "hamming") return Usage(argv[0]);
  } else if (group == "factories") {
    TestFactories();
  } else {
    return Usage(argv[0]);
  }

  if (g_failures == 0) {
    std::cout << "OK: " << group << (instance.empty() ? "" : " " + instance) << std::endl;
  } else {
    std::cout << g_failures << " check(s) failed" << std::endl;
  }
  return g_failures;
}
