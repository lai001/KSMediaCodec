#ifndef KSMediaCodec_MediaTime_hpp
#define KSMediaCodec_MediaTime_hpp

#include <string>

#include "defs.hpp"
#include "FFmpeg.h"

namespace ks
{
	struct KSMediaCodec_API MediaTime
	{
	public:
		MediaTime();
		MediaTime(const double seconds, const int timeScale);
		MediaTime(const int timeValue, const int timeScale);
		MediaTime(const AVRational rational);

	public:
		double seconds() const;
		const AVRational getRational() const;
		int timeValue() const;
		int timeScale() const;

		MediaTime add(const MediaTime time) const;
		MediaTime subtract(const MediaTime time) const;
		MediaTime muliply(const MediaTime time) const;
		MediaTime div(const MediaTime time) const;
		MediaTime convertScale(const int timeScale) const;
		MediaTime nearer(const MediaTime time0, const MediaTime time1) const;
		MediaTime invert() const;

		/**
		 * operator
		 */
		MediaTime operator+(const MediaTime &time);
		MediaTime operator-(const MediaTime &time);
		MediaTime operator+(const MediaTime &time) const;
		MediaTime operator-(const MediaTime &time) const;

		MediaTime operator*(const MediaTime &time);
		MediaTime operator/(const MediaTime &time);
		MediaTime operator*(const MediaTime &time) const;
		MediaTime operator/(const MediaTime &time) const;

		bool operator<(const MediaTime &time) const;
		bool operator>(const MediaTime &time) const;
		bool operator==(const MediaTime &time) const;
		bool operator!=(const MediaTime &time) const;
		bool operator<=(const MediaTime &time) const;
		bool operator>=(const MediaTime &time) const;

	public:
		static MediaTime zero;

	private:
		AVRational rational;
	};
}

#endif // KSMediaCodec_MediaTime_hpp