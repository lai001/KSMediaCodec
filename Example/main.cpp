#include <iostream>
#include <string>

#include <Foundation/Foundation.hpp>
#include <KSMediaCodec/KSMediaCodec.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char** argv)
{
	ks::Application::Init(argc, argv);
	std::string inputVideoPath = ks::Application::getResourcePath("BigBuckBunny.mp4");
	std::string outputVideoPath = ks::Application::getAppDir() + "/BigBuckBunny.mp4";
	ks::AudioFormat decodeAudioFormat;
	decodeAudioFormat.bitsPerChannel = 32;
	decodeAudioFormat.bytesPerFrame = 4;
	decodeAudioFormat.bytesPerPacket = 4;
	decodeAudioFormat.formatFlags = ks::AudioFormatFlag::isFloat | ks::AudioFormatFlag::isNonInterleaved;
	decodeAudioFormat.formatType = ks::AudioFormatIdentifiersType::pcm;
	decodeAudioFormat.sampleRate = 44100;
	decodeAudioFormat.framesPerPacket = 1;
	decodeAudioFormat.channelsPerFrame = 2;

	ks::AudioFormat encodeAudioFormat;
	encodeAudioFormat.bitsPerChannel = 32;
	encodeAudioFormat.bytesPerFrame = 4;
	encodeAudioFormat.bytesPerPacket = 4;
	encodeAudioFormat.formatFlags = ks::AudioFormatFlag::isFloat | ks::AudioFormatFlag::isNonInterleaved;
	encodeAudioFormat.formatType = ks::AudioFormatIdentifiersType::pcm;
	encodeAudioFormat.sampleRate = 44100;
	encodeAudioFormat.framesPerPacket = 1;
	encodeAudioFormat.channelsPerFrame = 2;

	ks::VideoFileEncoder::VideoEncodeAttribute encodeAttribute;
	encodeAttribute.pixelBufferFormatType = ks::PixelBuffer::FormatType::yuv420p;
	encodeAttribute.videoWidth = 720;
	encodeAttribute.videoHeight = 480;
	encodeAttribute.fps = ks::MediaTime(0, 1);
	encodeAttribute.gopSize = 30;
	encodeAttribute.timeBase = ks::MediaTime(1, 600);
	encodeAttribute.bitRate = 6*1000*1000;

	std::unique_ptr<ks::VideoDecoder> videoDecoder = std::unique_ptr<ks::VideoDecoder>(ks::VideoDecoder::New(inputVideoPath, ks::PixelBuffer::FormatType::rgba8));
	std::unique_ptr<ks::AudioDecoder> audioDecoder = std::unique_ptr<ks::AudioDecoder>(ks::AudioDecoder::New(inputVideoPath, decodeAudioFormat));

	ks::VideoFileEncoder::Error error;
	std::unique_ptr<ks::VideoFileEncoder> videoFileEncoder = std::unique_ptr<ks::VideoFileEncoder>(ks::VideoFileEncoder::New(outputVideoPath, encodeAttribute, encodeAudioFormat, &error));
	assert(videoFileEncoder);
	assert(videoDecoder);
	assert(audioDecoder);

	bool isEncodeVideo = true;
	bool isEncodeAudio = true;

	while (isEncodeVideo || isEncodeAudio)
	{
		ks::MediaTime pts;
		if (std::unique_ptr<ks::PixelBuffer> pixelBuffer = std::unique_ptr<ks::PixelBuffer>(videoDecoder->newFrame(pts)))
		{
			spdlog::info("video: {}", pts.seconds());
			videoFileEncoder->encode(*pixelBuffer, pts);
		}
		else
		{
			isEncodeVideo = false;
		}

		ks::MediaTimeRange timeRange;
		if (std::unique_ptr<ks::AudioPCMBuffer> pcmBuffer = std::unique_ptr<ks::AudioPCMBuffer>(audioDecoder->newFrame(timeRange)))
		{
			spdlog::info("audio: {}", timeRange.start.seconds());
			videoFileEncoder->encode(*pcmBuffer, timeRange.start);
		}
		else
		{
			isEncodeAudio = false;
		}
	}

	videoFileEncoder->encodeTail();

	spdlog::info(outputVideoPath);
	std::cin >> std::string();
    return 0;
}
