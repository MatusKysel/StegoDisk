#pragma once

#include "carrier_file.h"
#include "utils/stego_types.h"
#include "container_handler.h"

#include <tuple>

namespace stego_disk
{
	class MemoryBuffer;

	class CarrierFileMPEG : public CarrierFile {
	public:
		CarrierFileMPEG(File file, std::shared_ptr<Encoder> encoder, std::shared_ptr<Permutation> permutation, std::unique_ptr<Fitness> fitness);

		virtual void LoadFile() override;
		virtual void SaveFile() override;
	private:
		uint64 CalculateCapacity() const;
		void LoadBuffer(MemoryBuffer &buffer);
		void SaveBuffer(const MemoryBuffer &buffer);
		uint64 ModifyLSB(uint64 value, uint64 lsb) const;
		std::tuple<uint64, uint64, uint64> GetTestValues(const AVPacket &packet) const;
	private:
		ContainerHandlerUPtr container_handler_{ nullptr };
	};
}