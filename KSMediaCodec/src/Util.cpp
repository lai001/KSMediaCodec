#include "Util.hpp"
#include "FFmpeg.h"

namespace ks
{
	std::string KSMediaCodec_API ffmpegErrorDescription(int errorCode) noexcept
	{
		char errStr[AV_ERROR_MAX_STRING_SIZE] = { 0 };
		av_strerror(errorCode, errStr, sizeof(errStr));
		return std::string(errStr);
	}
}