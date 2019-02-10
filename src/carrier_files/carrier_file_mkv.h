#pragma once

#include "carrier_file.h"
#include "utils/stego_types.h"
#include "container_handler.h"

namespace stego_disk
{
	class MemoryBuffer;

	class CarrierFileMKV : public CarrierFile {
	public:
		CarrierFileMKV(File file, std::shared_ptr<Encoder> encoder, std::shared_ptr<Permutation> permutation, std::unique_ptr<Fitness> fitness);

		virtual void LoadFile() override;
		virtual void SaveFile() override;
	private:
		uint64 CalculateCapacity() const;
		void LoadBuffer(MemoryBuffer &buffer);
		void SaveBuffer(const MemoryBuffer &buffer);
	private:
		ContainerHandlerUPtr container_handler_{ nullptr };
	};
}