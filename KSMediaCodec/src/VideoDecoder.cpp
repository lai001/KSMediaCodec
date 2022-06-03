#include "VideoDecoder.hpp"
#include <unordered_map>
#include <assert.h>
#include <functional>

namespace ks
{
	VideoDecoder * VideoDecoder::New(const std::string & filePath, const ks::PixelBuffer::FormatType& formatType)
	{
		AVFormatContext *formatContext = nullptr;
		AVStream *videoStream = nullptr;
		AVCodecContext *videoCodecCtx = nullptr;
		AVCodec *codec = nullptr;
		int videoStreamIndex = -1;
		struct SwsContext *imageSwsContext = nullptr;
		std::function<void()> cleanClosure = [&]()
		{
			if (imageSwsContext)
			{
				sws_freeContext(imageSwsContext);
			}
			if (videoCodecCtx)
			{
				avcodec_close(videoCodecCtx);
				avcodec_free_context(&videoCodecCtx);
			}
			if (formatContext)
			{
				avformat_close_input(&formatContext);
				avformat_free_context(formatContext);
			}
		};

		defer
		{
			cleanClosure();
		};

		formatContext = avformat_alloc_context();
		if (formatContext == nullptr)
		{
			return nullptr;
		}
		if (avformat_open_input(&formatContext, filePath.c_str(), nullptr, nullptr) != 0)
		{
			return nullptr;
		}

		if (avformat_find_stream_info(formatContext, nullptr) < 0)
		{
			return nullptr;
		}

		for (unsigned int i = 0; i < formatContext->nb_streams; i++)
		{
			if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				videoStreamIndex = i;
				videoStream = formatContext->streams[i];
				break;
			}
		}
		if (videoStreamIndex == -1)
		{
			return nullptr;
		}

		codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
		if (codec == nullptr)
		{
			return nullptr;
		}
		videoCodecCtx = avcodec_alloc_context3(codec);
		if (videoCodecCtx == nullptr)
		{
			return nullptr;
		}
		avcodec_parameters_to_context(videoCodecCtx, videoStream->codecpar);

		if (avcodec_open2(videoCodecCtx, codec, nullptr) < 0)
		{
			return nullptr;
		}

		imageSwsContext = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
			videoCodecCtx->width, videoCodecCtx->height, getAVPixelFormat(formatType), SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
		if (imageSwsContext == nullptr)
		{
			return nullptr;
		}
		
		VideoDecoder * decoder = new VideoDecoder();
		decoder->filePath = filePath;
		decoder->videoCodecCtx = videoCodecCtx;
		decoder->formatContext = formatContext;
		decoder->imageSwsContext = imageSwsContext;
		decoder->codec = codec;
		decoder->videoStream = videoStream;
		decoder->videoStreamIndex = videoStreamIndex;
		decoder->outputFormatType = formatType;
		cleanClosure = []() {};
		return decoder;
	}

	VideoDecoder::~VideoDecoder()
	{
		assert(imageSwsContext);
		assert(videoCodecCtx);
		assert(formatContext);

		sws_freeContext(imageSwsContext);

		avcodec_close(videoCodecCtx);
		avcodec_free_context(&videoCodecCtx);

		avformat_close_input(&formatContext);
		avformat_free_context(formatContext);
	}

	ks::PixelBuffer * VideoDecoder::newDecodedFrame(AVCodecContext * videoCodecCtx, AVFrame* frame, AVPacket* packet, MediaTime& outTime)
	{
		int gotPicture;
		int status = avcodec_decode_video2(videoCodecCtx, frame, &gotPicture, packet);

		if (status < 0)
		{
			return nullptr;
		}
		else
		{
			int linesizes[4];
			status = av_image_fill_linesizes(linesizes, getAVPixelFormat(outputFormatType), frame->width);
			if (status < 0)
			{
				return nullptr;
			}
			ks::PixelBuffer* outPixelBuffer = new ks::PixelBuffer(videoCodecCtx->width, videoCodecCtx->height, outputFormatType);
			unsigned char **outImageData = outPixelBuffer->getMutableData();

			sws_scale(imageSwsContext, frame->data,
				frame->linesize, 0, videoCodecCtx->height,
				outImageData, linesizes);

			outTime = MediaTime((int)frame->pts, videoStream->time_base.den);
			return outPixelBuffer;
		}
	}

	ks::PixelBuffer* VideoDecoder::newFrame(MediaTime& outPts)
	{
		AVFrame *frame = av_frame_alloc();
		AVPacket *packet = av_packet_alloc();
		defer
		{
			av_packet_free(&packet);
			av_frame_free(&frame);
		};

		while (true)
		{
			defer
			{
				av_packet_unref(packet);
				av_frame_unref(frame);
			};

			if (av_read_frame(formatContext, packet) < 0)
			{
				break;
			}
			else
			{
				if (packet->stream_index == videoStreamIndex)
				{
					ks::PixelBuffer* pixelBuffer = newDecodedFrame(videoCodecCtx, frame, packet, outPts);
					if (pixelBuffer)
					{
						_lastDecodedImageDisplayTime = outPts;
					}
					return pixelBuffer;
				}
				else
				{
					continue;
				}
			}
		}

		return nullptr;
	}

	bool VideoDecoder::seek(const MediaTime& time)
	{
		MediaTime seekTime = time;
		seekTime = seekTime.convertScale(videoStream->time_base.den);
		int seekResult = av_seek_frame(formatContext, videoStreamIndex, seekTime.timeValue(), AVSEEK_FLAG_BACKWARD);
		if (seekResult < 0)
		{
			return false;
		}
		avcodec_flush_buffers(videoCodecCtx);
		return true;
	}

	MediaTime VideoDecoder::lastDecodedImageDisplayTime()
	{
		return _lastDecodedImageDisplayTime;
	}

	MediaTime VideoDecoder::fps()
	{
		if (videoStream)
		{
			MediaTime fps = MediaTime(videoStream->avg_frame_rate);
			return fps;
		}
		return MediaTime(-1.0, 600);
	}

	AVPixelFormat ks::VideoDecoder::getAVPixelFormat(const ks::PixelBuffer::FormatType & formatType) noexcept
	{
		std::unordered_map<ks::PixelBuffer::FormatType, AVPixelFormat> dic;
		dic[ks::PixelBuffer::FormatType::rgba8] = AV_PIX_FMT_RGBA;
		dic[ks::PixelBuffer::FormatType::bgra8] = AV_PIX_FMT_BGRA;
		dic[ks::PixelBuffer::FormatType::rgb8] = AV_PIX_FMT_RGB8;
		dic[ks::PixelBuffer::FormatType::bgr8] = AV_PIX_FMT_BGR8;
		dic[ks::PixelBuffer::FormatType::yuv420p] = AV_PIX_FMT_YUV420P;
		dic[ks::PixelBuffer::FormatType::gray8] = AV_PIX_FMT_GRAY8;
		assert(dic.find(formatType) != dic.end());
		return dic[formatType];
	}
}