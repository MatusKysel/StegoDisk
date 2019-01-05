#include "carrier_file_mpeg.h"

namespace stego_disk
{
	CarrierFileMPEG::CarrierFileMPEG(File file, std::shared_ptr<Encoder> encoder, std::shared_ptr<Permutation> permutation, std::unique_ptr<Fitness> fitness)
		: CarrierFile(file, encoder, permutation, std::move(fitness))
	{

	}

	void CarrierFileMPEG::LoadFile()
	{

	}

	void CarrierFileMPEG::SaveFile()
	{

	}
}