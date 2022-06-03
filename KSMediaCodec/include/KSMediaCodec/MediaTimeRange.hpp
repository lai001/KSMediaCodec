#ifndef KSMediaCodec_MediaTimeRange_hpp
#define KSMediaCodec_MediaTimeRange_hpp

#include <string>
#include "defs.hpp"

#include "MediaTime.hpp"

namespace ks
{
	struct KSMediaCodec_API MediaTimeRange
	{
	public:
		MediaTimeRange();
		MediaTimeRange(const MediaTime start, const MediaTime end);

	public:
		static MediaTimeRange zero;

	public:
		MediaTime start;
		MediaTime end;

		MediaTime duration() const;
		bool isEmpty() const;

		MediaTimeRange intersection(const MediaTimeRange otherTimeRange) const;
		bool containsTime(const MediaTime time) const;
		MediaTime clamp(const MediaTime& time) const;

	};
}

#endif // KSMediaCodec_MediaTimeRange_hpp