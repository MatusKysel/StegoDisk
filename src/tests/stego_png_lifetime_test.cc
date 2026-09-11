#include <filesystem>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "lodepng.h"
#include "stego_storage.h"

namespace {

namespace fs = std::filesystem;

void Require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

class TestDirectory {
 public:
  TestDirectory() {
    std::random_device random;
    for (int attempt = 0; attempt < 100; ++attempt) {
      path = fs::temp_directory_path() /
          ("stegodisk-png-lifetime-" + std::to_string(random()));
      if (fs::create_directory(path)) return;
    }
    throw std::runtime_error("Cannot create PNG test directory");
  }

  ~TestDirectory() {
    std::error_code error;
    fs::remove_all(path, error);
  }

  fs::path path;
};

void TestMetadataLifetime() {
  TestDirectory directory;
  const auto filename = (directory.path / "carrier.png").string();
  const std::string title = "StegoDisk PNG lifetime regression";
  std::vector<unsigned char> pixels(64 * 64 * 3);
  std::mt19937 random(42);
  for (auto& pixel : pixels) pixel = static_cast<unsigned char>(random());

  lodepng::State encoder;
  encoder.info_raw.colortype = LCT_RGB;
  encoder.info_png.color.colortype = LCT_RGB;
  encoder.encoder.auto_convert = 0;
  Require(lodepng_add_text(&encoder.info_png, "Title", title.c_str()) == 0,
          "Cannot add PNG text metadata");
  std::vector<unsigned char> encoded;
  Require(lodepng::encode(encoded, pixels, 64, 64, encoder) == 0,
          "Cannot encode PNG fixture");
  Require(lodepng::save_file(encoded, filename) == 0, "Cannot save PNG fixture");

  const std::vector<unsigned char> payload(128, 0x5a);
  for (int iteration = 0; iteration < 3; ++iteration) {
    {
      stego_disk::StegoStorage storage;
      storage.Configure(stego_disk::EncoderFactory::EncoderType::LSB,
                        stego_disk::PermutationFactory::PermutationType::IDENTITY,
                        stego_disk::PermutationFactory::PermutationType::IDENTITY);
      storage.Open(directory.path.string(), "");
      storage.Load();
      if (iteration > 0) {
        std::vector<unsigned char> restored(payload.size());
        storage.Read(restored.data(), 0, restored.size());
        Require(restored == payload, "PNG payload did not survive reopening");
      }
      storage.Write(payload.data(), 0, payload.size());
      storage.Save();
    } // LeakSanitizer verifies that carrier metadata is released here.

    lodepng::State decoder;
    std::vector<unsigned char> saved, decoded;
    unsigned width = 0, height = 0;
    Require(lodepng::load_file(saved, filename) == 0, "Cannot read saved PNG");
    Require(lodepng::decode(decoded, width, height, decoder, saved) == 0,
            "Cannot decode saved PNG");
    Require(width == 64 && height == 64, "PNG dimensions changed");
    bool found_title = false;
    for (std::size_t i = 0; i < decoder.info_png.text_num; ++i) {
      if (std::string(decoder.info_png.text_keys[i]) == "Title") {
        Require(decoder.info_png.text_strings[i] == title, "PNG metadata changed");
        found_title = true;
      }
    }
    Require(found_title, "PNG metadata was lost");
  }
}

} // namespace

int main() {
  try {
    TestMetadataLifetime();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
