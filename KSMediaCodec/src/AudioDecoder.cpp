#include "AudioDecoder.hpp"
#include <assert.h>
#include <functional>

namespace ks
{
	AudioDecoder * AudioDecoder::New(const std::string& filePath, const ks::AudioFormat& format)
	{
		SwrContext *swrctx = nullptr;
		AVFormatContext *formatContext = nullptr;
		AVStream *audioStream = nullptr;
		AVCodecContext *audioCodecCtx = nullptr;
		AVCodec *codec = nullptr;
		int audioStreamIndex = -1;
		std::function<void()> cleanClosure = [&]()
		{
			if (swrctx)
			{
				swr_close(swrctx);
				swr_free(&swrctx);
			}
			if (audioCodecCtx)
			{
				avcodec_close(audioCodecCtx);
				avcodec_free_context(&audioCodecCtx);
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
			if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				audioStreamIndex = i;
				audioStream = formatContext->streams[i];
				break;
			}
		}
		if (audioStreamIndex == -1)
		{
			return nullptr;
		}

		codec = avcodec_find_decoder(audioStream->codecpar->codec_id);

		if (codec == nullptr)
		{
			return nullptr;
		}

		audioCodecCtx = avcodec_alloc_context3(codec);

		if (audioCodecCtx == nullptr)
		{
			return nullptr;
		}

		avcodec_parameters_to_context(audioCodecCtx, audioStream->codecpar);

		if (avcodec_open2(audioCodecCtx, codec, nullptr) < 0)
		{
			return nullptr;
		}

		AVSampleFormat sampleFormat = AudioDecoder::getAVSampleFormat(format);

		swrctx = swr_alloc_set_opts(swrctx,
			av_get_default_channel_layout(format.channelsPerFrame), sampleFormat, format.sampleRate,
			audioCodecCtx->channel_layout, audioCodecCtx->sample_fmt, audioCodecCtx->sample_rate,
			0, nullptr);
		if (swrctx == nullptr)
		{
			return nullptr;
		}

		if (swr_init(swrctx) < 0)
		{
			return nullptr;
		}

		AudioDecoder* audioDecoder = new AudioDecoder();
		audioDecoder->filePath = filePath;
		audioDecoder->outputAudioFormat = format;
		audioDecoder->swrctx = swrctx;
		audioDecoder->formatContext = formatContext;
		audioDecoder->audioStream = audioStream;
		audioDecoder->audioCodecCtx = audioCodecCtx;
		audioDecoder->codec = codec;
		audioDecoder->audioStreamIndex = audioStreamIndex;
		cleanClosure = []() {};
		return audioDecoder;
	}

	AudioDecoder::~AudioDecoder()
	{
		assert(swrctx);
		assert(audioCodecCtx);
		assert(formatContext);

		swr_close(swrctx);
		swr_free(&swrctx);

		avcodec_close(audioCodecCtx);
		avcodec_free_context(&audioCodecCtx);

		avformat_close_input(&formatContext);
		avformat_free_context(formatContext);
	}

	std::string AudioDecoder::getFilePath() const
	{
		return filePath;
	}

	ks::AudioPCMBuffer* AudioDecoder::newFrame(MediaTimeRange& outTimeRange)
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

			int ret = av_read_frame(formatContext, packet);
			if (ret < 0)
			{
				break;
			}
			else
			{
				if (packet->stream_index == audioStreamIndex)
				{
					ks::AudioPCMBuffer* buffer = newDecodedPCMBuffer(audioCodecCtx, frame, packet, outTimeRange);
					if (buffer)
					{
						_lastDecodedAudioChunkDisplayTime = outTimeRange.start;
					}
					return buffer;
				}
				else
				{
					continue;
				}
			}
		}
		return nullptr;
	}

	int AudioDecoder::seek(MediaTime time)
	{
		MediaTime seekTime = time;
		seekTime = seekTime.convertScale(audioStream->time_base.den);
		int seekResult = av_seek_frame(formatContext, audioStreamIndex, seekTime.timeValue(), AVSEEK_FLAG_BACKWARD);
		if (seekResult < 0)
		{
			return seekResult;
		}
		avcodec_flush_buffers(audioCodecCtx);
		return 1;
	}

	MediaTime AudioDecoder::lastDecodedAudioChunkDisplayTime() const
	{
		return _lastDecodedAudioChunkDisplayTime;
	}

	MediaTime AudioDecoder::fps() const
	{
		if (audioStream)
		{
			MediaTime fps = MediaTime(audioStream->avg_frame_rate);
			return fps;
		}
		return MediaTime(-1.0, 600);
	}

	ks::AudioPCMBuffer * AudioDecoder::newDecodedPCMBuffer(AVCodecContext* audioCodecCtx, AVFrame * frame, const AVPacket * packet, MediaTimeRange & outTimeRange)
	{
		int got_frame_ptr;
		int ret = avcodec_decode_audio4(audioCodecCtx, frame, &got_frame_ptr, packet);
		if (ret >= 0)
		{
			ks::AudioPCMBuffer* outPCMBuffer = new ks::AudioPCMBuffer(outputAudioFormat, frame->nb_samples);
			const uint8_t ** source = const_cast<const uint8_t **>(frame->data);
			ret = swr_convert(swrctx,
				outPCMBuffer->channelData(), frame->nb_samples,
				source, frame->nb_samples);
			outTimeRange = MediaTimeRange(MediaTime((int)frame->pts, frame->sample_rate), MediaTime((int)frame->pts + (int)frame->nb_samples, frame->sample_rate));
			return outPCMBuffer;
		}
		else
		{
			return nullptr;
		}
	}

	AVSampleFormat AudioDecoder::getAVSampleFormat(const ks::AudioFormat & format) noexcept
	{

		bool isSignedInteger = format.isSignedInteger();
		bool isFloat = format.isFloat();
		bool isNonInterleaved = format.isNonInterleaved();
		AVSampleFormat sampleFormat = AV_SAMPLE_FMT_NONE;

		if (isSignedInteger && format.bitsPerChannel == 16 && isNonInterleaved)
		{
			sampleFormat = AV_SAMPLE_FMT_S16P;
		}
		if (isSignedInteger && format.bitsPerChannel == 16 && isNonInterleaved == false)
		{
			sampleFormat = AV_SAMPLE_FMT_S16;
		}
		if (isSignedInteger && format.bitsPerChannel == 32 && isNonInterleaved)
		{
			sampleFormat = AV_SAMPLE_FMT_S32P;
		}
		if (isSignedInteger && format.bitsPerChannel == 32 && isNonInterleaved == false)
		{
			sampleFormat = AV_SAMPLE_FMT_S32;
		}
		if (isFloat && format.bitsPerChannel == 32 && isNonInterleaved)
		{
			sampleFormat = AV_SAMPLE_FMT_FLTP;
		}
		if (isFloat && format.bitsPerChannel == 32 && isNonInterleaved == false)
		{
			sampleFormat = AV_SAMPLE_FMT_FLT;
		}
		if (isFloat && format.bitsPerChannel == 64 && isNonInterleaved)
		{
			sampleFormat = AV_SAMPLE_FMT_DBLP;
		}
		if (isFloat && format.bitsPerChannel == 64 && isNonInterleaved == false)
		{
			sampleFormat = AV_SAMPLE_FMT_DBL;
		}
		assert(sampleFormat != AV_SAMPLE_FMT_NONE);
		return sampleFormat;
	}
}