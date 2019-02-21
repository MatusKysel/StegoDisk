#include "carrier_file_mpeg.h"

namespace stego_disk
{
	CarrierFileMPEG::CarrierFileMPEG(File file, std::shared_ptr<Encoder> encoder, std::shared_ptr<Permutation> permutation, std::unique_ptr<Fitness> fitness)
		: CarrierFile(file, encoder, permutation, std::move(fitness))
	{
		container_handler_ = std::make_unique<ContainerHandler>(file.GetAbsolutePath());

		if (container_handler_)
		{
			container_handler_->Load();
		}

		raw_capacity_ = this->CalculateCapacity();
	}

	void CarrierFileMPEG::LoadFile()
	{
		if (container_handler_)
		{
			auto data_buffer = MemoryBuffer();
			this->LoadBuffer(data_buffer);

			buffer_.Resize(raw_capacity_);
			buffer_.Clear();

			if (!permutation_->GetSize())
			{
				permutation_->Init(raw_capacity_, subkey_);
			}

			for (uint64 i = 0; i < permutation_->GetSize(); ++i)
			{
				if (data_buffer[i] & 0x01)
				{
					SetBitInBufferPermuted(i);
				}
			}

			ExtractBufferUsingEncoder();
		}
	}

	void CarrierFileMPEG::SaveFile()
	{
		if (container_handler_)
		{
			auto data_buffer = MemoryBuffer();
			this->LoadBuffer(data_buffer);

			buffer_.Resize(raw_capacity_);
			buffer_.Clear();

			if (!permutation_->GetSize())
			{
				permutation_->Init(raw_capacity_, subkey_);
			}

			for (uint64 i = 0; i < permutation_->GetSize(); ++i)
			{
				if (data_buffer[i] & 0x01)
				{
					SetBitInBufferPermuted(i);
				}
			}

			EmbedBufferUsingEncoder();

			for (uint64 i = 0; i < permutation_->GetSize(); ++i)
			{
				data_buffer[i] = (data_buffer[i] & 0xFE) | GetBitInBufferPermuted(i);
			}

			this->SaveBuffer(data_buffer);
		}
	}

	stego_disk::uint64 CarrierFileMPEG::CalculateCapacity() const
	{
		if (container_handler_)
		{
			auto stream_data = container_handler_->GetStreamData();
			uint64 size{ 0 };

			if (stream_data.find(StreamType::Video) != stream_data.end())
			{
				auto video_stream = stream_data.at(StreamType::Video);

				for (const auto &packet : video_stream)
				{
					auto zero_pts = this->ModifyLSB(packet.get().pts, 0);
					auto one_pts = this->ModifyLSB(packet.get().pts, 1);
					auto dts = packet.get().dts;

					if (zero_pts >= dts && one_pts >= dts)
					{
						size += 1;
					}
				}
			}
			else
			{
				return 0;
			}

			return (size / 8);
		}
		else
		{
			return 0;
		}
	}

	void CarrierFileMPEG::LoadBuffer(MemoryBuffer &buffer)
	{
		buffer.Resize(raw_capacity_);
		buffer.Clear();

		std::size_t byte_position{ 0u }, buff_offset{ 0u };
		uint8 current_byte{ 0u };

		for (const auto &packet : container_handler_->GetStreamData().at(StreamType::Video))
		{
			auto zero_pts = this->ModifyLSB(packet.get().pts, 0);
			auto one_pts = this->ModifyLSB(packet.get().pts, 1);
			auto dts = packet.get().dts;

			if (zero_pts >= dts && one_pts >= dts)
			{
				current_byte |= (packet.get().pts & 0x1) << (7 - byte_position);

				byte_position += 1;

				if (byte_position == 8)
				{
					buffer.Write(buff_offset, &current_byte, sizeof(uint8));
					current_byte = 0u;
					byte_position = 0;
					buff_offset += 1;
				}
			}
		}
	}

	void CarrierFileMPEG::SaveBuffer(const MemoryBuffer &buffer)
	{
		std::size_t byte_position{ 0u }, buffer_offset{ 0u };

		for (auto &packet : container_handler_->GetStreamData().at(StreamType::Video))
		{
			auto zero_pts = this->ModifyLSB(packet.get().pts, 0);
			auto one_pts = this->ModifyLSB(packet.get().pts, 1);
			auto dts = packet.get().dts;

			if (zero_pts >= dts && one_pts >= dts)
			{
				packet.get().pts = (packet.get().pts & (~1)) | ((buffer[buffer_offset] >> (7 - byte_position)) & 1);

				byte_position += 1;

				if (byte_position == 8)
				{
					byte_position = 0;
					buffer_offset += 1;
				}

				if (buffer_offset >= raw_capacity_)
				{
					break;
				}
			}
		}

		container_handler_->Save();
	}

	stego_disk::uint64 CarrierFileMPEG::ModifyLSB(uint64 value, uint64 lsb) const
	{
		auto modified_value = (value & static_cast<uint64>(~0x1)) | lsb;
		return modified_value;
	}
}