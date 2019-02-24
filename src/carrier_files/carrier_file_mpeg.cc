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
		LOG_DEBUG("File: " + file.GetAbsolutePath() + " raw capacity: " + std::to_string(raw_capacity_));
	}

	void CarrierFileMPEG::LoadFile()
	{
		LOG_INFO("Loading mpeg carrier file: " + this->GetFile().GetAbsolutePath());

		if (container_handler_)
		{
			auto data_buffer = MemoryBuffer();
			this->LoadBuffer(data_buffer);

			buffer_.Resize(static_cast<std::size_t>(raw_capacity_));
			buffer_.Clear();

			if (!permutation_->GetSize())
			{
				permutation_->Init(raw_capacity_ * 8, subkey_);
			}

			for (uint64 i = 0; i < permutation_->GetSize(); ++i)
			{
				if (data_buffer[static_cast<std::size_t>(i)] & 0x01)
				{
					SetBitInBufferPermuted(i);
				}
			}

			ExtractBufferUsingEncoder();
		}
	}

	void CarrierFileMPEG::SaveFile()
	{
		LOG_INFO("Saving mpeg carrier file: " + this->GetFile().GetAbsolutePath());

		if (container_handler_)
		{
			auto data_buffer = MemoryBuffer();
			this->LoadBuffer(data_buffer);

			buffer_.Resize(static_cast<std::size_t>(raw_capacity_));
			buffer_.Clear();

			if (!permutation_->GetSize())
			{
				permutation_->Init(raw_capacity_ * 8, subkey_);
			}

			for (uint64 i = 0; i < permutation_->GetSize(); ++i)
			{
				if (data_buffer[static_cast<std::size_t>(i)] & 0x01)
				{
					SetBitInBufferPermuted(i);
				}
			}

			EmbedBufferUsingEncoder();

			for (uint64 i = 0; i < permutation_->GetSize(); ++i)
			{
				data_buffer[static_cast<std::size_t>(i)] = (data_buffer[static_cast<std::size_t>(i)] & 0xFE) | GetBitInBufferPermuted(i);
			}

			this->SaveBuffer(data_buffer);
		}
	}

	stego_disk::uint64 CarrierFileMPEG::CalculateCapacity() const
	{
		LOG_DEBUG("Calculating MPEG raw capacity");

		if (container_handler_)
		{
			auto stream_data = container_handler_->GetStreamData();
			uint64 size{ 0 };

			if (stream_data.find(StreamType::Video) != stream_data.end())
			{
				auto video_stream = stream_data.at(StreamType::Video);

				for (const auto &packet : video_stream)
				{
					auto[zero_pts, one_pts, dts] = this->GetTestValues(packet.get());

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

			LOG_DEBUG("Raw capacity: " + std::to_string(size / 8) + "B");
			return (size / 8);
		}
		else
		{
			return 0;
		}
	}

	void CarrierFileMPEG::LoadBuffer(MemoryBuffer &buffer)
	{
		LOG_DEBUG("Loading MPEG buffer");

		buffer.Resize(static_cast<std::size_t>(raw_capacity_ * 8));
		buffer.Clear();

		std::size_t buffer_offset{ 0u };
		uint8 current_byte{ 0u };

		for (const auto &packet : container_handler_->GetStreamData().at(StreamType::Video))
		{
			auto[zero_pts, one_pts, dts] = this->GetTestValues(packet.get());

			if (zero_pts >= dts && one_pts >= dts)
			{
				LOG_TRACE("Reading from packet, pts: " + std::to_string(packet.get().pts) +
					" dts: " + std::to_string(packet.get().dts) +
					" stream index: " + std::to_string(packet.get().stream_index));

				current_byte = packet.get().pts & 0xFF;
				buffer.Write(buffer_offset, &current_byte, sizeof(uint8));
				buffer_offset += 1;

				LOG_TRACE("Byte read: " + std::to_string(current_byte) +
					" offset: " + std::to_string(buffer_offset));

				if (buffer_offset == buffer.GetSize())
				{
					break;
				}
			}
			else
			{
				LOG_TRACE("Packet not applicable, dropping, pts: " + std::to_string(packet.get().pts) +
					"dts: " + std::to_string(packet.get().dts) +
					"stream index: " + std::to_string(packet.get().stream_index))
			}
		}
	}

	void CarrierFileMPEG::SaveBuffer(const MemoryBuffer &buffer)
	{
		LOG_DEBUG("Saving MPEG buffer");

		std::size_t buffer_offset{ 0u };

		for (auto &packet : container_handler_->GetStreamData().at(StreamType::Video))
		{
			auto[zero_pts, one_pts, dts] = this->GetTestValues(packet.get());

			if (zero_pts >= dts && one_pts >= dts)
			{
				LOG_TRACE("Packet before, pts: " + std::to_string(packet.get().pts) +
					"dts: " + std::to_string(packet.get().dts) +
					"stream index: " + std::to_string(packet.get().stream_index));

				packet.get().pts = (packet.get().pts & (~1)) | ((buffer[buffer_offset] & 1));
				buffer_offset += 1;

				LOG_TRACE("Packet after, pts: " + std::to_string(packet.get().pts) +
					"dts: " + std::to_string(packet.get().dts) +
					"stream index: " + std::to_string(packet.get().stream_index))

				if (buffer_offset == buffer.GetSize())
				{
					break;
				}
			}
			else
			{
				LOG_TRACE("Packet not applicable, dropping, pts: " + std::to_string(packet.get().pts) +
					"dts: " + std::to_string(packet.get().dts) +
					"stream index: " + std::to_string(packet.get().stream_index))
			}
		}

		container_handler_->Save();
	}

	stego_disk::uint64 CarrierFileMPEG::ModifyLSB(uint64 value, uint64 lsb) const
	{
		auto modified_value = (value & static_cast<uint64>(~0x1)) | lsb;
		return modified_value;
	}

	// returns testing values for packet, used in checks whether we can use actual packet for embedding or not
	std::tuple<uint64, uint64, uint64> CarrierFileMPEG::GetTestValues(const AVPacket &packet) const
	{
		auto zero_pts = this->ModifyLSB(packet.pts, 0);
		auto one_pts = this->ModifyLSB(packet.pts, 1);
		auto dts = packet.dts;

		return std::make_tuple(zero_pts, one_pts, dts);
	}
}