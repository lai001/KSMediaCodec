#ifndef KSMediaCodec_VideoFileEncoder_hpp
#define KSMediaCodec_VideoFileEncoder_hpp

#include <Foundation/Foundation.hpp>
#include "FFmpeg.h"
#include "MediaTime.hpp"
#include "MediaTimeRange.hpp"
#include "defs.hpp"

namespace ks
{
	class KSMediaCodec_API VideoFileEncoder : public noncopyable
	{
	public:
		enum class Error
		{
			avformat_alloc_output_context2,
			avformat_new_stream_video,
			avformat_new_stream_audio,
			avcodec_find_encoder_video,
			avcodec_find_encoder_audio,
			avcodec_alloc_context3,
			avcodec_open2_video,
			avcodec_open2_audio,
			avcodec_parameters_from_context_video,
			avcodec_parameters_from_context_audio,
			avio_open,
			avformat_write_header,
		};

	public:
		struct VideoEncodeAttribute
		{
			ks::PixelBuffer::FormatType pixelBufferFormatType;
			unsigned int videoWidth;
			unsigned int videoHeight;
			MediaTime fps;
			MediaTime timeBase;
			long long bitRate;
			unsigned int gopSize;
		};

	public:
		static VideoFileEncoder* New(const std::string& outputPath,
			const VideoEncodeAttribute& videoEncodeAttribute,
			const ks::AudioFormat& outputAudioFormat,
			VideoFileEncoder::Error* error);

		~VideoFileEncoder();

		void encode(const ks::PixelBuffer& pixelBuffer, const ks::MediaTime& pts);
		void encode(const ks::AudioPCMBuffer& pcmBuffer, const ks::MediaTime& pts);
		void encodeTail();
		unsigned int getAudioSamples();

	private:
		std::string outputPath;
		VideoEncodeAttribute videoEncodeAttribute;
		ks::AudioFormat outputAudioFormat;

		const AVOutputFormat *outputFormat = nullptr;
		AVFormatContext *outputFormatContext = nullptr;

		const AVCodec *audioCodec = nullptr;
		AVStream *audioStream = nullptr;
		AVCodecContext *audioCodecContext = nullptr;

		const AVCodec *videoCodec = nullptr;
		AVStream *videoStream = nullptr;
		AVCodecContext *videoCodecContext = nullptr;

		int encodeFrame(AVFrame *frame, AVCodecContext *codecContext, AVStream *steam) noexcept;
	};
}

#endif // !KSMediaCodec_VideoFileEncoder_hpp