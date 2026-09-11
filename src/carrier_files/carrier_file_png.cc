/**
* @file carrier_file_bmp.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File with implementation of loading and saving BMP files
*
*/

#include "carrier_file_png.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef STEGODISK_HAVE_ZLIB
#include <zlib.h>
#endif

#include "utils/exceptions.h"
#include "utils/stego_errors.h"


namespace stego_disk {

namespace {

#ifdef STEGODISK_HAVE_ZLIB
// Compression level for the rewritten carrier. Level 3 is roughly twice as
// fast as lodepng's own deflate for about 2% more bytes; level 6 gives back
// those bytes but none of the speed.
const int kDeflateLevel = 3;

// lodepng calls this instead of its own deflate. It must emit a complete zlib
// stream, which is exactly what compress2 produces, and allocate with malloc
// because lodepng frees the result.
unsigned CompressWithZlib(unsigned char **out, size_t *outsize,
                          const unsigned char *in, size_t insize,
                          const LodePNGCompressSettings *) {
  const uLongf bound = compressBound(static_cast<uLong>(insize));
  *out = static_cast<unsigned char *>(malloc(bound));
  if (*out == nullptr)
    return 83; // lodepng: allocation failed

  uLongf written = bound;
  if (compress2(*out, &written, in, static_cast<uLong>(insize),
                kDeflateLevel) != Z_OK) {
    free(*out);
    *out = nullptr;
    return 83;
  }
  *outsize = written;
  return 0;
}
#endif

// Re-encoding the carrier dominates a save, and the pixels are fixed by the
// embedding, so only the deflate settings are ours to choose. Both branches
// produce a valid PNG with identical pixels; they differ only in file size.
void ConfigureEncoderForSpeed(LodePNGState *state) {
#ifdef STEGODISK_HAVE_ZLIB
  state->encoder.zlibsettings.custom_zlib = CompressWithZlib;
#else
  // Without zlib, tune lodepng's deflate: most of its time goes into match
  // finding, and a shorter window with no lazy matching costs ~0.6% of size.
  state->encoder.zlibsettings.windowsize = 512;
  state->encoder.zlibsettings.lazymatching = 0;
  state->encoder.zlibsettings.nicematch = 32;
#endif
}

} // namespace

CarrierFilePNG::CarrierFilePNG(File file, std::shared_ptr<Encoder> encoder,
                               std::shared_ptr<Permutation> permutation,
                               std::unique_ptr<Fitness> fitness)
    : CarrierFile(file, encoder, permutation, std::move(fitness)) {

  auto file_ptr = file.Open();

  unsigned char png_header[64];

  fseek(file_ptr.Get(), 0, SEEK_SET);
  int read_cnt = static_cast<int>(fread(&png_header, 1, 64, file_ptr.Get()));


  if (read_cnt < 64) {
    throw exception::ParseError{file_.GetFileName(), "Wrong header size"};
  }

  unsigned error = lodepng_inspect(&width_, &height_, &state_, png_header, 64);
  if (error)
    throw exception::ParseError{file_.GetFileName(),
                                "Unable to read file state"};

  state_.info_raw.colortype = LCT_RGB;
  state_.info_raw.bitdepth = 8;
  ConfigureEncoderForSpeed(&state_);

  raw_capacity_ = (lodepng_get_raw_size(width_, height_, &state_.info_raw) / 8);
}

/**
 * @brief Reads the carrier from disk and decodes it into `image`
 *
 * Leaves `image` untouched when it already holds the decoded carrier, which
 * is what lets SaveFile() reuse the work LoadFile() did.
 */
void CarrierFilePNG::DecodeInto(DecodedImage *image) {
  if (*image != nullptr)
    return;

  auto file_ptr = file_.Open();

  MemoryBuffer png_buffer(file_.GetSize());

  fseek(file_ptr.Get(), 0, SEEK_SET);
  const uint32 read_cnt = static_cast<uint32>(
      fread(png_buffer.GetRawPointer(), 1, file_.GetSize(), file_ptr.Get()));

  if (read_cnt < file_.GetSize()) {
    LOG_ERROR("Unable to read file.");
    throw exception::IoError{file_.GetFileName()};
  }

  unsigned char *decoded = nullptr;
  unsigned width = 0;
  unsigned height = 0;
  const unsigned error =
      lodepng_decode(&decoded, &width, &height, &state_,
                     png_buffer.GetConstRawPointer(), read_cnt);

  if (error)
    throw exception::ParseError{file_.GetFileName(), "Unable to decode file"};

  *image = DecodedImage(decoded, &free);
}

void CarrierFilePNG::LoadFile() {

  if (file_loaded_)
    return;

  LOG_INFO("Loading file " << file_.GetRelativePath());

  if (permutation_->GetSize() == 0) {
    permutation_->Init(raw_capacity_ * 8, subkey_);
  }

  buffer_.Resize(raw_capacity_);
  buffer_.Clear();

  const uint64 bits_to_modify = permutation_->GetSize();

  // Held until SaveFile(), which needs these same pixels.
  DecodeInto(&decoded_image_);
  const unsigned char *image = decoded_image_.get();

  // copy LSB data to content buffer

  ExtractLsbToBufferPermuted(image, bits_to_modify);

  ExtractBufferUsingEncoder();

  file_loaded_ = true;

  LOG_INFO("File " << file_.GetRelativePath() << " loaded");
}


void CarrierFilePNG::SaveFile() {
  auto file_ptr = file_.Open();

  if (!file_loaded_)
    throw exception::InvalidState{exception::Operation::save,
                                  exception::Component::file,
                                  exception::ComponentState::notLoaded};


  LOG_INFO("Saving file " << file_.GetRelativePath());

  if (permutation_->GetSize() == 0) {
    permutation_->Init(raw_capacity_ * 8, subkey_);
  }

  buffer_.Resize(raw_capacity_);
  buffer_.Clear();

  const uint64 bits_to_modify = permutation_->GetSize();

  // Normally still holds the pixels LoadFile() decoded; only decodes when the
  // carrier was released, so the outcome does not depend on the cache.
  DecodeInto(&decoded_image_);
  unsigned char *image = decoded_image_.get();

  // copy LSB data to content buffer

  ExtractLsbToBufferPermuted(image, bits_to_modify);

  EmbedBufferUsingEncoder();

  ApplyBufferPermutedToLsb(image, bits_to_modify);

  unsigned char *image_out;
  size_t size_out;

  unsigned error =
      lodepng_encode(&image_out, &size_out, image, width_, height_, &state_);

  if (error)
    throw exception::ParseError{file_.GetFileName(), "Unable to encode file"};

  // write data

  fseek(file_ptr.Get(), 0, SEEK_SET);
  uint32 write_cnt =
      static_cast<uint32>(fwrite(image_out, 1, size_out, file_ptr.Get()));

  if (write_cnt != size_out) {
    LOG_ERROR("Writing PNG file expanded");
    throw exception::IoError{file_.GetFileName()};
  }

  free(image_out);
  // The carrier is written; the pixels are no longer worth holding.
  decoded_image_.reset();

  LOG_INFO("File " << file_.GetRelativePath() << " saved");
}

} // stego_disk
