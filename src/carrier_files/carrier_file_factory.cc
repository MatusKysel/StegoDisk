/**
* @file carrier_file_factory.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Factory for files
*
*/

#include "carrier_file_factory.h"

#include <algorithm>

#include "carrier_file.h"
#include "carrier_file_bmp.h"
#include "carrier_file_jpeg.h"
#include "carrier_file_png.h"
#include "carrier_file_mkv.h"
#include "encoders/encoder_factory.h"
#include "permutations/permutation_factory.h"
#include "utils/stego_config.h"
#include "permutations/permutation.h"

namespace stego_disk 
{
	CarrierFilePtr CarrierFileFactory::CreateCarrierFile(const File& file) 
	{
		std::shared_ptr<CarrierFile> carrier_file;

		if (auto ext = file.GetExtension(); ext == "bmp")
		{
			carrier_file = std::make_shared<CarrierFileBMP>(file, nullptr, CreatePermutation(ext), nullptr);
		}
		else if (ext == "jpg")
		{
			carrier_file = std::make_shared<CarrierFileJPEG>(file, nullptr, CreatePermutation(ext), nullptr);
		}
		else if (ext == "png")
		{
			carrier_file = std::make_shared<CarrierFilePNG>(file, nullptr, CreatePermutation(ext), nullptr);
		}
		else if (ext == "mkv")
		{
			carrier_file = std::make_shared<CarrierFileMKV>(file, nullptr, CreatePermutation(ext), nullptr);
		}

		if (carrier_file) 
		{
			if (carrier_file->GetRawCapacity() < 1) 
			{
				carrier_file = CarrierFilePtr();
			}
		}

		return carrier_file;
	}

	std::shared_ptr<stego_disk::Permutation> CarrierFileFactory::CreatePermutation(const std::string &ext)
	{
		auto config = StegoConfig::file_config();
		return PermutationFactory::GetPermutation((config.find(ext) != config.end() ? config[ext].second : StegoConfig::local_perm()));
	}

} // stego_disk
