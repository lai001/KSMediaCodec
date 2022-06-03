#include "VideoFileEncoder.hpp"
#include <iostream>
#include <assert.h>
#include <functional>
#include "AudioDecoder.hpp"
#include "VideoDecoder.hpp"
#include "Util.hpp"

namespace ks
{
	VideoFileEncoder * VideoFileEncoder::New(const std::string& outputPath,
		const VideoEncodeAttribute& videoEncodeAttribute,
		const ks::AudioFormat& outputAudioFormat,
		VideoFileEncoder::Error* error)
	{
		const AVOutputFormat *outputFormat = nullptr;
		AVFormatContext *outputFormatContext = nullptr;

		const AVCodec *audioCodec = nullptr;
		AVStream *audioStream = nullptr;
		AVCodecContext *audioCodecContext = nullptr;

		const AVCodec *videoCodec = nullptr;
		AVStream *videoStream = nullptr;
		AVCodecContext *videoCodecContext = nullptr;
		VideoFileEncoder::Error _error;
		std::function<void()> cleanClosure = [&]()
		{
			if (error)
			{
				*error = _error;
			}
			if (videoCodecContext)
			{
				avcodec_free_context(&videoCodecContext);
			}
			if (audioCodecContext)
			{
				avcodec_free_context(&audioCodecContext);
			}
			if (outputFormatContext)
			{
				if (!(outputFormat->flags & AVFMT_NOFILE))
				{
					int status = avio_closep(&outputFormatContext->pb);
					assert(status == 0);
				}
				avformat_free_context(outputFormatContext);
			}
		};

		defer
		{
			cleanClosure();
		};

		int status = 0;

		if ((status = avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, outputPath.c_str())) < 0)
		{
			_error = Error::avformat_alloc_output_context2;
			return nullptr;
		}
		else
		{
			outputFormat = outputFormatContext->oformat;
			assert(outputFormat);
		}

		if (videoCodec = avcodec_find_encoder(outputFormat->video_codec))
		{
			videoStream = avformat_new_stream(outputFormatContext, videoCodec);
			videoCodecContext = avcodec_alloc_context3(videoCodec);
			if ((videoStream && videoCodecContext) == false)
			{
				_error = Error::avformat_new_stream_video;
				return nullptr;
			}
		}
		else
		{
			_error = Error::avcodec_find_encoder_video;
			return nullptr;
		}

		if (audioCodec = avcodec_find_encoder(outputFormat->audio_codec))
		{
			audioStream = avformat_new_stream(outputFormatContext, audioCodec);
			audioCodecContext = avcodec_alloc_context3(audioCodec);
			if ((audioStream && audioCodecContext) == false)
			{
				_error = Error::avformat_new_stream_audio;
				return nullptr;
			}
		}
		else
		{
			_error = Error::avcodec_find_encoder_audio;
			return nullptr;
		}

		videoCodecContext->bit_rate = videoEncodeAttribute.bitRate;
		videoCodecContext->width = videoEncodeAttribute.videoWidth;
		videoCodecContext->height = videoEncodeAttribute.videoHeight;
		videoCodecContext->framerate = videoEncodeAttribute.fps.getRational();
		videoCodecContext->time_base = videoEncodeAttribute.timeBase.getRational();
		videoCodecContext->gop_size = videoEncodeAttribute.gopSize;
		videoCodecContext->pix_fmt = VideoDecoder::getAVPixelFormat(videoEncodeAttribute.pixelBufferFormatType);// AV_PIX_FMT_YUV420P;
		if (videoCodecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO)
		{
			videoCodecContext->max_b_frames = 2;
		}
		if (videoCodecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO)
		{
			videoCodecContext->mb_decision = 2;
		}
		if (outputFormat->flags & AVFMT_GLOBALHEADER)
		{
			videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
		videoStream->time_base = videoCodecContext->time_base;

		if ((status = avcodec_open2(videoCodecContext, videoCodec, nullptr)) != 0)
		{
			_error = Error::avcodec_open2_video;
			return nullptr;
		}
		if (avcodec_parameters_from_context(videoStream->codecpar, videoCodecContext) < 0)
		{
			_error = Error::avcodec_parameters_from_context_video;
			return nullptr;
		}

		audioCodecContext->sample_fmt = AudioDecoder::getAVSampleFormat(outputAudioFormat);
		audioCodecContext->bit_rate = 128000;
		audioCodecContext->sample_rate = outputAudioFormat.sampleRate;
		audioCodecContext->channel_layout = av_get_default_channel_layout(outputAudioFormat.channelsPerFrame);
		audioCodecContext->channels = av_get_channel_layout_nb_channels(audioCodecContext->channel_layout);
		audioCodecContext->time_base = MediaTime(1, audioCodecContext->sample_rate).getRational();
		if (outputFormat->flags & AVFMT_GLOBALHEADER)
		{
			audioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
		audioStream->time_base = audioCodecContext->time_base;
		if ((status = avcodec_open2(audioCodecContext, audioCodec, nullptr)) != 0)
		{
			_error = Error::avcodec_open2_audio;
			return nullptr;
		}
		if ((status = avcodec_parameters_from_context(audioStream->codecpar, audioCodecContext)) < 0)
		{
			_error = Error::avcodec_parameters_from_context_audio;
			return nullptr;
		}

		if (!(outputFormat->flags & AVFMT_NOFILE))
		{
			if ((status = avio_open(&outputFormatContext->pb, outputPath.c_str(), AVIO_FLAG_WRITE)) < 0)
			{
				_error = Error::avio_open;
				return nullptr;
			}
		}
		if ((status = avformat_write_header(outputFormatContext, nullptr)) < 0)
		{
			_error = Error::avformat_write_header;
			return nullptr;
		}

		VideoFileEncoder * videoFileEncoder = new VideoFileEncoder();
		videoFileEncoder->audioCodec = audioCodec;
		videoFileEncoder->audioCodecContext = audioCodecContext;
		videoFileEncoder->audioStream = audioStream;
		videoFileEncoder->videoCodec = videoCodec;
		videoFileEncoder->videoCodecContext = videoCodecContext;
		videoFileEncoder->videoStream = videoStream;
		videoFileEncoder->outputFormatContext = outputFormatContext;
		videoFileEncoder->outputFormat = outputFormat;
		videoFileEncoder->videoEncodeAttribute = videoEncodeAttribute;
		videoFileEncoder->outputPath = outputPath;
		videoFileEncoder->outputAudioFormat = outputAudioFormat;
		cleanClosure = []() {};
		return videoFileEncoder;
	}

	VideoFileEncoder::~VideoFileEncoder()
	{
		assert(videoCodecContext);
		assert(audioCodecContext);
		assert(outputFormatContext);
		avcodec_free_context(&videoCodecContext);
		avcodec_free_context(&audioCodecContext);
		if (!(outputFormat->flags & AVFMT_NOFILE))
		{
			int status = avio_closep(&outputFormatContext->pb);
			assert(status == 0);
		}
		avformat_free_context(outputFormatContext);
	}

	void VideoFileEncoder::encode(const ks::PixelBuffer & pixelBuffer, const ks::MediaTime & pts)
	{
		AVFrame *frame = av_frame_alloc();
		defer{ av_frame_unref(frame); av_frame_free(&frame); };
		assert(frame);
		frame->format = videoCodecContext->pix_fmt;
		frame->width = videoCodecContext->width;
		frame->height = videoCodecContext->height;
		int status = av_frame_get_buffer(frame, 0);
		assert(status == 0);

		struct SwsContext *videoSwsContext = sws_getContext(pixelBuffer.getWidth(), pixelBuffer.getHeight(), VideoDecoder::getAVPixelFormat(pixelBuffer.getType()),
			videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
			SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
		assert(videoSwsContext);
		defer { sws_freeContext(videoSwsContext); };
		int rgblinesizes[4];
		status = av_image_fill_linesizes(rgblinesizes, VideoDecoder::getAVPixelFormat(pixelBuffer.getType()), pixelBuffer.getWidth());
		assert(status >= 0);

		sws_scale(videoSwsContext, pixelBuffer.getImmutableData(),
			rgblinesizes, 0, pixelBuffer.getHeight(),
			frame->data, frame->linesize);

		frame->pts = pts.convertScale(videoCodecContext->time_base.den).timeValue();

		encodeFrame(frame, videoCodecContext, videoStream);
	}

	void VideoFileEncoder::encode(const ks::AudioPCMBuffer & pcmBuffer, const ks::MediaTime & pts)
	{
		struct SwrContext *audioSwrContext = swr_alloc_set_opts(nullptr,
			audioCodecContext->channel_layout, audioCodecContext->sample_fmt, audioCodecContext->sample_rate,
			av_get_default_channel_layout(pcmBuffer.audioFormat().channelsPerFrame), ks::AudioDecoder::getAVSampleFormat(pcmBuffer.audioFormat()), pcmBuffer.audioFormat().sampleRate,
			0, nullptr);
		assert(audioSwrContext);
		int status = swr_init(audioSwrContext);
		assert(status >= 0);
		defer{ swr_close(audioSwrContext); swr_free(&audioSwrContext); };
		AVFrame *frame = av_frame_alloc();
		defer{ av_frame_unref(frame); av_frame_free(&frame); };
		assert(frame);
		frame->format = audioCodecContext->sample_fmt;
		frame->channel_layout = audioCodecContext->channel_layout;
		frame->sample_rate = audioCodecContext->sample_rate;
		frame->nb_samples = getAudioSamples();
		frame->pts = pts.convertScale(audioCodecContext->sample_rate).timeValue();
		assert(pcmBuffer.samplesPerChannel() == frame->nb_samples);
		av_frame_get_buffer(frame, 0);
		const unsigned char * const * inData = pcmBuffer.immutableChannelData();
		status = swr_convert(audioSwrContext,
			frame->data, frame->nb_samples,
			const_cast<const uint8_t**>(inData), pcmBuffer.samplesPerChannel());
		assert(status >= 0);
		encodeFrame(frame, audioCodecContext, audioStream);
	}

	void VideoFileEncoder::encodeTail()
	{
		encodeFrame(nullptr, videoCodecContext, videoStream);
		encodeFrame(nullptr, audioCodecContext, audioStream);
		int status = av_write_trailer(outputFormatContext);
		assert(status == 0);
	}

	unsigned int ks::VideoFileEncoder::getAudioSamples()
	{
		assert(audioCodecContext);
		return audioCodecContext->frame_size == 0 ? 1024 : audioCodecContext->frame_size;
	}

	int VideoFileEncoder::encodeFrame(AVFrame * frame, AVCodecContext * codecContext, AVStream * steam) noexcept
	{
		int ret = 0;
		ret = avcodec_send_frame(codecContext, frame);
		if (ret < 0)
		{
			std::cout << ffmpegErrorDescription(ret) << std::endl;
			assert(false);
		}

		while (ret >= 0)
		{
			AVPacket pkt = { 0 };
			ret = avcodec_receive_packet(codecContext, &pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				break;
			}
			else if (ret < 0)
			{
				assert(false);
			}

			av_packet_rescale_ts(&pkt, codecContext->time_base, steam->time_base);

			pkt.stream_index = steam->index;
			ret = av_interleaved_write_frame(outputFormatContext, &pkt);
			av_packet_unref(&pkt);
		}
		return ret;
	}
}