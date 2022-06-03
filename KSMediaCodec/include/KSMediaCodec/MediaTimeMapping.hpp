#ifndef KSMediaCodec_MediaTimeMapping_hpp
#define KSMediaCodec_MediaTimeMapping_hpp
#include "defs.hpp"

#include "MediaTimeRange.hpp"

namespace ks
{
	struct KSMediaCodec_API MediaTimeMapping
	{
	public:
		MediaTimeMapping();
		MediaTimeMapping(const MediaTimeRange source, const MediaTimeRange target);

		//private:
		MediaTimeRange source;
		MediaTimeRange target;
	};
}

#endif // KSMediaCodec_MediaTimeMapping_hpp