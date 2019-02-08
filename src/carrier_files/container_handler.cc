#include "container_handler.h"

namespace stego_disk
{

	ContainerHandler::ContainerHandler(const std::string &name)
		:file_name_(name)
	{
		Init();
	}

	ContainerHandler::ContainerHandler()
	{

	}

	ContainerHandler::~ContainerHandler()
	{

	}

	void ContainerHandler::SetFileName(const std::string &name)
	{
		file_name_ = name;
	}

	std::string ContainerHandler::GetFileName() const
	{
		return file_name_;
	}

	void ContainerHandler::Load()
	{
		while (true)
		{
			auto packet = std::make_unique<AVPacket>();
			AVStream *stream{ nullptr };

			if (av_read_frame(input_context_, packet.get()) < 0)
			{
				break;
			}

			stream = input_context_->streams[packet->stream_index];
			auto stream_type = GetStreamType(stream->codecpar->codec_type);
			data_.emplace_back(std::move(packet));
			stream_data_[stream_type].emplace_back(std::ref(*data_.back()));
		}
	}

	void ContainerHandler::Save()
	{
		if (avformat_alloc_output_context2(&output_context_, nullptr, nullptr, file_name_.c_str()) <= 0)
		{
			// TODO error
		}

		for (auto i = 0u; i < input_context_->nb_streams; i++)
		{
			AVStream *input_stream = input_context_->streams[i];
			AVStream *output_stream = avformat_new_stream(output_context_, nullptr);

			if (!output_context_)
			{
				// TODO error
			}

			if (avcodec_parameters_copy(output_stream->codecpar, input_stream->codecpar) < 0)
			{
				// TODO error
			}

			output_stream->codecpar->codec_tag = 0;
		}

		if (avio_open(&output_context_->pb, file_name_.c_str(), AVIO_FLAG_WRITE) < 0)
		{
			// TODO error
		}

		if (avformat_write_header(output_context_, nullptr) < 0)
		{
			// TODO error
		}

		for (auto &packet : data_)
		{
			if (av_interleaved_write_frame(output_context_, packet.get()) < 0)
			{
				// TODO error
			}

			av_packet_unref(packet.get());
		}

		av_write_trailer(output_context_);
		this->Close();
	}

	stego_disk::StreamData ContainerHandler::GetStreamData()
	{
		return stream_data_;
	}

	void ContainerHandler::Init()
	{
		if (!avformat_open_input(&input_context_, file_name_.c_str(), nullptr, nullptr))
		{
			// TODO error
		}

		if (!avformat_find_stream_info(input_context_, nullptr))
		{
			// TODO error
		}
	}

	void ContainerHandler::Close()
	{
		if (input_context_)
		{
			avformat_close_input(&input_context_);
		}

		if (output_context_ && !(output_context_->flags & AVFMT_NOFILE))
		{
			avio_closep(&output_context_->pb);
			avformat_free_context(output_context_);
		}
	}

	StreamType ContainerHandler::GetStreamType(int type) const
	{
		switch (type)
		{
			case AVMEDIA_TYPE_VIDEO:
			{
				return StreamType::Video;
			}
			case AVMEDIA_TYPE_AUDIO:
			{
				return StreamType::Audio;
			}
			case AVMEDIA_TYPE_SUBTITLE:
			{
				return StreamType::Subtitles;
			}
			default:
			{
				return StreamType::Unknown;
			}
		}
	}
}