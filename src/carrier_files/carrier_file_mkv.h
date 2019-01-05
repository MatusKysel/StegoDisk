#pragma once

#include "carrier_file.h"

namespace stego_disk
{
	class CarrierFileMKV : public CarrierFile {
	public:
		CarrierFileMKV(File file, std::shared_ptr<Encoder> encoder, std::shared_ptr<Permutation> permutation, std::unique_ptr<Fitness> fitness);

		virtual void LoadFile() override;
		virtual void SaveFile() override;
	private:
	};
}