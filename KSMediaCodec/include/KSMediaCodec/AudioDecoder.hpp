#ifndef KSMediaCodec_AudioDecoder_hpp
#define KSMediaCodec_AudioDecoder_hpp

#include <string>
#include <vector>

#include "defs.hpp"
#include "FFmpeg.h"
#include <Foundation/Foundation.hpp>
#include "MediaTimeMapping.hpp"

namespace ks
{
	class KSMediaCodec_API AudioDecoder : public noncopyable
	{
	public:
		static AudioDecoder* New(const std::string& filePath, const ks::AudioFormat& format);

		~AudioDecoder();

		std::string getFilePath() const;

		ks::AudioPCMBuffer* newFrame(MediaTimeRange& outTimeRange);

		int seek(MediaTime time);

		MediaTime lastDecodedAudioChunkDisplayTime() const;

		MediaTime fps() const;

	private:
		ks::AudioFormat outputAudioFormat;

		MediaTime _lastDecodedAudioChunkDisplayTime = MediaTime::zero;

		std::string filePath = "";

		SwrContext *swrctx = nullptr;
		AVFormatContext *formatContext = nullptr;
		AVStream *audioStream = nullptr;
		AVCodecContext *audioCodecCtx = nullptr;
		AVCodec *codec = nullptr;
		int audioStreamIndex = -1;

	private:
		ks::AudioPCMBuffer* newDecodedPCMBuffer(AVCodecContext* audioCodecCtx, AVFrame* frame, const AVPacket* packet, MediaTimeRange& outTimeRange);

	public:
		static AVSampleFormat getAVSampleFormat(const ks::AudioFormat& format) noexcept;
	};
}

#endif // KSMediaCodec_AudioDecoder_hpp