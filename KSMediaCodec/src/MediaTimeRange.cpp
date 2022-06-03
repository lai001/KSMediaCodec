#include "MediaTimeRange.hpp"
#include <algorithm>

namespace ks
{
	MediaTimeRange::MediaTimeRange()
		: start(MediaTime()), end(MediaTime())
	{
	}

	MediaTimeRange::MediaTimeRange(const MediaTime start, const MediaTime end)
		: start(start), end(end)
	{
	}

	MediaTime MediaTimeRange::duration() const
	{
		MediaTime _end = end;
		return _end - start;
	}

	bool MediaTimeRange::isEmpty() const
	{
		return duration().seconds() <= 0.0;
	}

	MediaTimeRange MediaTimeRange::intersection(const MediaTimeRange otherTimeRange) const
	{
		MediaTime _start = std::max(this->start, otherTimeRange.start);
		MediaTime end = std::min(this->end, otherTimeRange.end);
		if (_start < end)
		{
			return MediaTimeRange(_start, end);
		}
		else
		{
			return MediaTimeRange();
		}
	}

	bool MediaTimeRange::containsTime(const MediaTime time) const
	{
		if (time <= end && time >= start)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	MediaTime MediaTimeRange::clamp(const MediaTime & time) const
	{
		return std::max(std::min(time, (*this).end), (*this).start);
	}

	MediaTimeRange MediaTimeRange::zero = MediaTimeRange(MediaTime::zero, MediaTime::zero);

}