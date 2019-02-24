#include "carrier_file_mkv.h"
#include "utils/memory_buffer.h"
#include "utils/exceptions.h"
#include "logging/logger.h"

namespace stego_disk
{
	CarrierFileMKV::CarrierFileMKV(File file, std::shared_ptr<Encoder> encoder, std::shared_ptr<Permutation> permutation, std::unique_ptr<Fitness> fitness)
		:CarrierFile(file, encoder, permutation, std::move(fitness))
	{
		container_handler_ = std::make_unique<ContainerHandler>(file.GetAbsolutePath());

		if (container_handler_)
		{
			container_handler_->Load();
		}

		raw_capacity_ = this->CalculateCapacity();
		LOG_DEBUG("File: " + file.GetAbsolutePath() + " raw capacity: " + std::to_string(raw_capacity_));
	}

	void CarrierFileMKV::LoadFile()
	{
		LOG_INFO("Loading mkv carrier file: " + this->GetFile().GetAbsolutePath());

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

	void CarrierFileMKV::SaveFile()
	{
		LOG_INFO("Saving mkv carrier file: " + this->GetFile().GetAbsolutePath());

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

	stego_disk::uint64 CarrierFileMKV::CalculateCapacity() const
	{
		LOG_DEBUG("Calculating MKV raw capacity");

		if (container_handler_)
		{
			auto stream_data = container_handler_->GetStreamData();
			uint64 size{ 0 };

			for (const auto &stream : stream_data)
			{
				size += static_cast<uint64>(stream.second.size());
			}

			LOG_DEBUG("Raw capacity: " + std::to_string(size / 8) + "B");
			return (size / 8);
		}
		else
		{
			return 0;
		}
	}

	void CarrierFileMKV::LoadBuffer(MemoryBuffer &buffer)
	{
		LOG_DEBUG("Loading MKV buffer");

		buffer.Resize(static_cast<std::size_t>(raw_capacity_ * 8));
		buffer.Clear();

		std::size_t buffer_offset{ 0u };
		uint8 current_byte{ 0u };

		for (const auto &packet : container_handler_->GetData())
		{
			LOG_TRACE("Reading from packet, pts: " + std::to_string(packet->pts) +
				" dts: " + std::to_string(packet->dts) +
				" stream index: " + std::to_string(packet->stream_index));

			current_byte = packet->pts & 0xFF;
			buffer.Write(buffer_offset, &current_byte, sizeof(uint8));
			buffer_offset += 1;

			LOG_TRACE("Byte read: " + std::to_string(current_byte) +
				" offset: " + std::to_string(buffer_offset));

			if (buffer_offset == buffer.GetSize())
			{
				break;
			}
		}
	}

	void CarrierFileMKV::SaveBuffer(const MemoryBuffer &buffer)
	{
		LOG_DEBUG("Saving MKV buffer");

		std::size_t buffer_offset{ 0u };

		// saving new pts for all packets
		for (auto &packet : container_handler_->GetData())
		{
			LOG_TRACE("Packet before, pts: " + std::to_string(packet->pts) +
				"dts: " + std::to_string(packet->dts) +
				"stream index: " + std::to_string(packet->stream_index));

			packet->pts = (packet->pts & (~1)) | ((buffer[buffer_offset] & 1));
			buffer_offset += 1;

			LOG_TRACE("Packet after, pts: " + std::to_string(packet->pts) +
				"dts: " + std::to_string(packet->dts) +
				"stream index: " + std::to_string(packet->stream_index))

			if (packet->pts < packet->dts)
			{
				LOG_TRACE("Packet dts correction from: " + std::to_string(packet->dts) + " to: " + std::to_string(packet->pts));
				packet->dts = packet->pts;
			}

			if (buffer_offset == buffer.GetSize())
			{
				break;
			}
		}

		container_handler_->Save();
	}
}