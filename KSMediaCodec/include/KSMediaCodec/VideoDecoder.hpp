#ifndef KSMediaCodec_VideoDecoder_hpp
#define KSMediaCodec_VideoDecoder_hpp

#include <string>
#include <vector>
#include <Foundation/Foundation.hpp>
#include "defs.hpp"
#include "FFmpeg.h"
#include "MediaTimeMapping.hpp"

namespace ks
{
	class KSMediaCodec_API VideoDecoder : public noncopyable
	{
	public:
		static VideoDecoder* New(const std::string & filePath, const ks::PixelBuffer::FormatType& formatType);
		~VideoDecoder();

	private:
		std::string filePath = "";
		ks::PixelBuffer::FormatType outputFormatType;
		AVFormatContext *formatContext = nullptr;
		AVStream *videoStream = nullptr;
		AVCodecContext *videoCodecCtx = nullptr;
		AVCodec *codec = nullptr;
		int videoStreamIndex = -1;
		struct SwsContext *imageSwsContext = nullptr;
		MediaTime _lastDecodedImageDisplayTime = MediaTime::zero;

	private:
		ks::PixelBuffer* newDecodedFrame(AVCodecContext * videoCodecCtx, AVFrame* frame, AVPacket* packet, MediaTime& outTime);

	public:
		ks::PixelBuffer* newFrame(MediaTime& outPts);
		bool seek(const MediaTime& time);

		MediaTime lastDecodedImageDisplayTime();
		MediaTime fps();

	public:
		static AVPixelFormat getAVPixelFormat(const ks::PixelBuffer::FormatType& formatType) noexcept;
	};
}

#endif // KSMediaCodec_VideoDecoder_hpp