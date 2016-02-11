#ifndef STEGODISK_UTILS_STEGOCONFIG_H_
#define STEGODISK_UTILS_STEGOCONFIG_H_

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <memory>

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


    //      if (Instance().ssl_context_.load_verify_file(ssl_config["ca_cert_file"].ToString(), *error)) {
    //        LOG_ERROR("StegoConfig.Init", error->message() << ':' << ssl_config["ca_cert_file"].ToString());
    //        return *error;
    //      }

    //    if (Instance().ssl_context_.use_certificate_chain_file(ssl_config["cert_chain_file"].ToString(), *error)) {
    //      LOG_ERROR("StegoConfig.Init", error->message() << ':' << ssl_config["cert_chain_file"].ToString());
    //      return *error;
    //    }

    //    if (Instance().ssl_context_.use_private_key_file(ssl_config["private_key_file"].ToString(), boost::asio::ssl::context::pem, *error)) {
    //      LOG_ERROR("StegoConfig.Init", error->message() << ':' << ssl_config["private_key_file"].ToString());
    //      return *error;
    //    }

    //    if (Instance().ssl_context_.use_tmp_dh_file(ssl_config["tmp_dh_file"].ToString(), *error)) {
    //      LOG_ERROR("StegoConfig.Init", error->message() << ':' << ssl_config["tmp_dh_file"].ToString());
    //      return *error;
    //    }

    Instance().stego_config_loaded_ = true;

  }

  inline static PermutationFactory::PermutationType &global_perm() { return Instance().global_perm_; }
  inline static PermutationFactory::PermutationType &local_perm() { return Instance().local_perm_; }
  inline static EncoderFactory::EncoderType &encoder() { return Instance().encoder_; }
  inline static std::unique_ptr< std::vector<std::string> > &exclude_list() { return Instance().exclude_list_; }
  inline static std::unique_ptr<std::map<std::string,
  std::pair<EncoderFactory::EncoderType, PermutationFactory::PermutationType> > >
  &file_config() { return Instance().file_config_; }


private:
  StegoConfig() :
    stego_config_loaded_(false),
    exclude_list_(nullptr),
    file_config_(nullptr)
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
  std::unique_ptr< std::vector<std::string> > exclude_list_;
  std::unique_ptr<std::map<std::string,
  std::pair<EncoderFactory::EncoderType, PermutationFactory::PermutationType> > > file_config_;

  static StegoConfig stego_config_;
};

} // stego_disk
#endif // STEGODISK_UTILS_STEGOCONFIG_H_
