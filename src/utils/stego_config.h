/**
* @file stego_config.h
* @author Matus Kysel
* @date 2016
* @brief File containing singleton class with whole configuration for stego storage
*
*/

#ifndef STEGODISK_UTILS_STEGOCONFIG_H_
#define STEGODISK_UTILS_STEGOCONFIG_H_

#include <string>
#include <set>
#include <map>
#include <tuple>
#include <memory>

#include "api_mask.h"
#include "encoders/encoder_factory.h"
#include "permutations/permutation_factory.h"
#include "utils/json.h"

namespace stego_disk {

class StegoConfig {
public:
  inline static bool initialized() { return Instance().stego_config_loaded_; }

  inline static void Init(const json::JsonObject &config) {

    Instance().encoder_ = EncoderFactory::GetEncoderType(config["encoder"].ToString());
    Instance().global_perm_ = PermutationFactory::GetPermutationType(config["glob_perm"].ToString());
    Instance().local_perm_ = PermutationFactory::GetPermutationType(config["local_perm"].ToString());
    Instance().stego_config_loaded_ = true;

    if(config["exclude_types"].IsArray()) {
      json::JsonObject exclude_list = config["exclude_types"];
      for (size_t i = 0; i < exclude_list.ArraySize(); ++i) {
         if(exclude_list[i].IsString()) {
           Instance().exclude_list_.insert(exclude_list[i].ToString());
         }
       }
    }

    if(config["file_types"].IsArray()) {
      json::JsonObject file_types = config["file_types"];
       for (size_t i = 0; i < file_types.ArraySize(); ++i) {
         if(file_types[i].IsObject()) {
           if(file_types[i]["file_type"].IsString()){
              Instance().file_config_[file_types[i]["file_type"].ToString()] =
                  std::make_pair(EncoderFactory::GetEncoderType(file_types[i]["encoder"].ToString()),
                  PermutationFactory::GetPermutationType(file_types[i]["permutation"].ToString()));
           }
         }
       }
    }
  }

  inline static PermutationFactory::PermutationType &global_perm() { return Instance().global_perm_; }
  inline static PermutationFactory::PermutationType &local_perm() { return Instance().local_perm_; }
  inline static EncoderFactory::EncoderType &encoder() { return Instance().encoder_; }
  inline static std::set<std::string> &exclude_list() { return Instance().exclude_list_; }
  inline static std::map<std::string, std::pair<EncoderFactory::EncoderType, PermutationFactory::PermutationType> >
  &file_config() { return Instance().file_config_; }


private:
  StegoConfig() :
    stego_config_loaded_(false),
    exclude_list_(),
    file_config_()
  {}

  ~StegoConfig() {}

  inline static StegoConfig &Instance() {
    static StegoConfig stego_config;
    return stego_config;
  }

  bool stego_config_loaded_;
  EncoderFactory::EncoderType encoder_;
  PermutationFactory::PermutationType global_perm_;
  PermutationFactory::PermutationType local_perm_;
  std::set<std::string> exclude_list_;
  std::map<std::string, std::pair<EncoderFactory::EncoderType, PermutationFactory::PermutationType> > file_config_;

  static StegoConfig stego_config_;
};

} // stego_disk

#endif // STEGODISK_UTILS_STEGOCONFIG_H_
