#ifndef KSMediaCodec_Util_hpp
#define KSMediaCodec_Util_hpp

#include <string>
#include "defs.hpp"

namespace ks
{
	std::string KSMediaCodec_API ffmpegErrorDescription(int errorCode) noexcept;
}

#endif // !KSMediaCodec_Util_hpp