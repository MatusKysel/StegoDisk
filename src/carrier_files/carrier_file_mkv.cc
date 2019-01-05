#include "carrier_file_mkv.h"

namespace stego_disk
{
	CarrierFileMKV::CarrierFileMKV(File file, std::shared_ptr<Encoder> encoder, std::shared_ptr<Permutation> permutation, std::unique_ptr<Fitness> fitness)
		:CarrierFile(file, encoder, permutation, std::move(fitness))
	{

	}

	void CarrierFileMKV::LoadFile()
	{

	}

	void CarrierFileMKV::SaveFile()
	{

	}
}