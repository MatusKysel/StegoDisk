// Regression tests for configuration, capacity queries and storage lifecycle.
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "carrier_files/carrier_file.h"
#include "carrier_files/carrier_file_factory.h"
#include "encoders/hamming_encoder.h"
#include "encoders/lsb_encoder.h"
#include "file_management/carrier_files_manager.h"
#include "logging/logger.h"
#include "lodepng.h"
#include "stego_storage.h"
#include "utils/exceptions.h"
#include "utils/stego_config.h"

namespace {

using namespace stego_disk;
namespace fs = std::filesystem;

int failures = 0;

#define EXPECT(expr) do { \
  if (!(expr)) { \
    std::cerr << "FAIL line " << __LINE__ << ": " << #expr << '\n'; \
    ++failures; \
  } \
} while (0)

#define EXPECT_THROWS(stmt, type) do { \
  bool thrown = false; \
  try { stmt; } catch (const type &) { thrown = true; } \
  EXPECT(thrown); \
} while (0)

// Each process owns a unique, small fixture; no checked-in carrier is modified.
class TestDirectory {
 public:
  TestDirectory() {
    std::random_device random;
    for (int attempt = 0; attempt < 100; ++attempt) {
      path = fs::temp_directory_path() /
             ("stegodisk-state-" + std::to_string(random()));
      if (fs::create_directory(path)) return;
    }
    throw std::runtime_error("Cannot create test directory");
  }

  ~TestDirectory() {
    std::error_code error;
    fs::remove_all(path, error);
  }

  void AddPng() const {
    // 64 * 64 * 3 channel LSBs hold exactly 1536 bytes with identity + LSB.
    std::vector<unsigned char> pixels(64 * 64 * 3);
    std::mt19937 random(42);
    for (auto &pixel : pixels) pixel = static_cast<unsigned char>(random());
    if (lodepng::encode((path / "carrier.png").string(), pixels, 64, 64, LCT_RGB))
      throw std::runtime_error("Cannot encode test PNG");
  }

  fs::path path;
};

void ConfigureJson(const std::string &text) {
  json::JsonObject config;
  const std::string error = json::Parse(text, &config);
  if (!error.empty()) throw std::runtime_error(error);
  StegoConfig::Init(config);
}

void ConfigureIdentity(StegoStorage &storage) {
  storage.Configure(EncoderFactory::EncoderType::LSB,
                    PermutationFactory::PermutationType::IDENTITY,
                    PermutationFactory::PermutationType::IDENTITY);
}

void TestConfigurationReset() {
  TestDirectory directory;
  directory.AddPng();
  const std::string original_config = R"({"exclude_types":["png"],"file_types":[
    {"file_type":"png","encoder":"hamming","permutation":"affine"}]})";
  ConfigureJson(original_config);

  CarrierFilesManager manager;
  manager.LoadDirectory(directory.path.string());
  EXPECT(manager.GetRawCapacity() == 0);
  EXPECT(StegoConfig::file_config().size() == 1);

  StegoStorage storage;
  ConfigureIdentity(storage);
  EXPECT(StegoConfig::exclude_list().empty());
  EXPECT(StegoConfig::file_config().empty());
  manager.LoadDirectory(directory.path.string());
  EXPECT(manager.GetRawCapacity() == 1536);

  // Replacing one nonempty configuration must remove entries absent in the next.
  ConfigureJson(original_config);
  ConfigureJson(R"({"exclude_types":["bmp"],"file_types":[
    {"file_type":"jpg","encoder":"lsb","permutation":"identity"}]})");
  EXPECT(StegoConfig::exclude_list() == std::set<std::string>{"bmp"});
  EXPECT(StegoConfig::file_config().size() == 1);
  EXPECT(StegoConfig::file_config().count("jpg") == 1);
}

void TestCapacityEstimate() {
  TestDirectory directory;
  directory.AddPng();
  StegoStorage storage;
  ConfigureIdentity(storage);
  auto carrier = CarrierFileFactory::CreateCarrierFile(
      File(directory.path.string(), "carrier.png"));
  if (!carrier) throw std::runtime_error("Test carrier was not recognized");
  carrier->SetSubkey(Key::FromString("test-key"));

  auto lsb = std::make_shared<LsbEncoder>(8);
  auto hamming = std::make_shared<HammingEncoder>(5);
  carrier->SetEncoder(lsb);
  EXPECT(carrier->GetCapacityUsingEncoder(lsb) == 1536);
  // p=5 stores 5 bytes per 32-byte codeword block: 48 blocks give 240 bytes.
  EXPECT(carrier->GetCapacityUsingEncoder(hamming) == 240);
  EXPECT(carrier->GetCapacity() == 1536);  // Query must not change the active encoder.
  carrier->SetEncoder(hamming);
  EXPECT(carrier->GetCapacity() == 240);

  CarrierFilesManager manager;
  EXPECT(manager.GetCapacityUsingEncoder(lsb) == 0);
  manager.LoadDirectory(directory.path.string());
  EXPECT(manager.GetCapacityUsingEncoder(lsb) == 1536);
  EXPECT(manager.GetCapacityUsingEncoder(hamming) == 240);

  // Estimates are needed before applying an encoder, including keyed permutations.
  for (auto local : {PermutationFactory::PermutationType::AFFINE,
                     PermutationFactory::PermutationType::AFFINE64,
                     PermutationFactory::PermutationType::FEISTEL_NUM,
                     PermutationFactory::PermutationType::FEISTEL_MIX}) {
    storage.Configure(EncoderFactory::EncoderType::LSB,
                      PermutationFactory::PermutationType::IDENTITY, local);
    CarrierFilesManager keyed_manager;
    keyed_manager.SetPassword("test-password");
    keyed_manager.LoadDirectory(directory.path.string());
    try {
      const auto estimate = keyed_manager.GetCapacityUsingEncoder(lsb);
      EXPECT(estimate > 0);
      EXPECT(estimate <= 1536);
      keyed_manager.SetEncoder(lsb);
      keyed_manager.ApplyEncoder();
      EXPECT(keyed_manager.GetCapacity() == estimate);
      EXPECT(keyed_manager.GetCapacityUsingEncoder(hamming) > 0);
      EXPECT(keyed_manager.GetCapacity() == estimate);
    } catch (const std::exception &error) {
      std::cerr << "Capacity estimate for "
                << PermutationFactory::GetPermutationName(local)
                << " failed: " << error.what() << '\n';
      ++failures;
    }
  }
}

void TestEncoderOverride() {
  TestDirectory directory;
  directory.AddPng();
  for (bool use_lsb : {true, false}) {
    const std::string global_encoder = use_lsb ? "hamming" : "lsb";
    const std::string file_encoder = use_lsb ? "lsb" : "hamming";
    ConfigureJson("{\"encoder\":\"" + global_encoder +
        "\",\"glob_perm\":\"identity\",\"local_perm\":\"identity\",\"file_types\":["
        "{\"file_type\":\"png\",\"encoder\":\"" + file_encoder +
        "\",\"permutation\":\"identity\"}]}");

    StegoStorage storage;
    storage.Open(directory.path.string(), "test-password");
    storage.Load();
    // 32 bytes are reserved for the checksum.
    const std::size_t expected_size = use_lsb ? 1504 : 208;
    EXPECT(storage.GetSize() == expected_size);
    if (storage.GetSize() != expected_size) continue;
    std::vector<unsigned char> payload(expected_size, use_lsb ? 0x5A : 0xA5);
    storage.Write(payload.data(), 0, payload.size());
    storage.Save();

    StegoStorage reopened;
    reopened.Open(directory.path.string(), "test-password");
    reopened.Load();
    std::vector<unsigned char> actual(payload.size());
    reopened.Read(actual.data(), 0, actual.size());
    EXPECT(actual == payload);
  }
}

void ExpectUnloaded(StegoStorage &storage) {
  unsigned char byte = 0;
  EXPECT(storage.GetSize() == 0);
  EXPECT_THROWS(storage.Read(&byte, 0, 1), stego_disk::exception::InvalidState);
  EXPECT_THROWS(storage.Write(&byte, 0, 1), stego_disk::exception::InvalidState);
  EXPECT_THROWS(storage.Save(), stego_disk::exception::InvalidState);
}

void TestReopen() {
  TestDirectory directory;
  directory.AddPng();
  StegoStorage storage;
  ConfigureIdentity(storage);
  storage.Open(directory.path.string(), "test-password");
  storage.Load();
  const unsigned char payload[] = {1, 2, 3, 4};
  storage.Write(payload, 0, sizeof(payload));
  storage.Save();

  storage.Open(directory.path.string(), "test-password");
  ExpectUnloaded(storage);
  storage.Load();
  unsigned char actual[sizeof(payload)] = {};
  storage.Read(actual, 0, sizeof(actual));
  EXPECT(std::equal(std::begin(payload), std::end(payload), std::begin(actual)));

  EXPECT_THROWS(storage.Open((directory.path / "missing").string(), ""),
                std::exception);
  ExpectUnloaded(storage);
}

}  // namespace

int main(int argc, char *argv[]) {
  std::string level = "ERROR";
  Logger::SetVerbosityLevel(level, "cout");
  if (argc != 2) return 2;
  const std::string group = argv[1];
  try {
    if (group == "config") TestConfigurationReset();
    else if (group == "capacity") TestCapacityEstimate();
    else if (group == "encoder_override") TestEncoderOverride();
    else if (group == "reopen") TestReopen();
    else return 2;
  } catch (const std::exception &error) {
    std::cerr << "Unexpected exception: " << error.what() << '\n';
    return 1;
  }
  return failures == 0 ? 0 : 1;
}
