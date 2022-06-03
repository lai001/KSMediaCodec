#include "MediaTime.hpp"

namespace ks
{
	MediaTime::MediaTime()
		: rational(MediaTime::zero.getRational())
	{
	}

	MediaTime::MediaTime(const double seconds, const int timeScale)
	{
		rational.num = seconds * double(timeScale);
		rational.den = timeScale;
	}

	MediaTime::MediaTime(const int timeValue, const int timeScale)
	{
		rational.num = timeValue;
		rational.den = timeScale;
	}

	MediaTime::MediaTime(const AVRational rational)
		: rational(rational)
	{
	}

	double MediaTime::seconds() const
	{
		return av_q2d(rational);
	}

	const AVRational MediaTime::getRational() const
	{
		return rational;
	}

	int MediaTime::timeValue() const
	{
		return rational.num;
	}

	int MediaTime::timeScale() const
	{
		return rational.den;
	}

	MediaTime MediaTime::add(const MediaTime time) const
	{
		MediaTime newTime = MediaTime(av_add_q(rational, time.rational));
		return newTime;
	}

	MediaTime MediaTime::subtract(const MediaTime time) const
	{
		MediaTime newTime = MediaTime(av_sub_q(rational, time.rational));
		return newTime;
	}

	MediaTime MediaTime::muliply(const MediaTime time) const
	{
		MediaTime newTime = MediaTime(av_mul_q(rational, time.rational));
		return newTime;
	}

	MediaTime MediaTime::div(const MediaTime time) const
	{
		MediaTime newTime = MediaTime(av_div_q(rational, time.rational));
		return newTime;
	}

	MediaTime MediaTime::convertScale(const int timeScale) const
	{
		MediaTime newTime = MediaTime((int)((double)timeValue() / (double)this->timeScale() * (double)timeScale), timeScale);
		return newTime;
	}

	MediaTime MediaTime::nearer(const MediaTime time0, const MediaTime time1) const
	{
		int compareResult = av_nearer_q(getRational(), time0.getRational(), time1.getRational());
		if (compareResult == 1)
		{
			return MediaTime(time0);
		}
		else if (compareResult == -1)
		{
			return MediaTime(time1);
		}
		else
		{
			return *this;
		}
	}

	MediaTime MediaTime::invert() const
	{
		return MediaTime(av_inv_q(this->getRational()));
	}

	MediaTime MediaTime::operator+(const MediaTime &time)
	{
		return this->add(time);
	}

	MediaTime MediaTime::operator-(const MediaTime &time)
	{
		return this->subtract(time);
	}

	MediaTime MediaTime::operator+(const MediaTime & time) const
	{
		return this->add(time);
	}

	MediaTime MediaTime::operator-(const MediaTime & time) const
	{
		return this->subtract(time);
	}

	MediaTime MediaTime::operator*(const MediaTime & time)
	{
		return this->muliply(time);
	}

	MediaTime MediaTime::operator/(const MediaTime & time)
	{
		return this->div(time);
	}

	MediaTime MediaTime::operator*(const MediaTime & time) const
	{
		return this->muliply(time);
	}

	MediaTime MediaTime::operator/(const MediaTime & time) const
	{
		return this->div(time);
	}

	bool MediaTime::operator<(const MediaTime &time) const
	{
		return av_cmp_q(this->getRational(), time.getRational()) == -1;
	}

	bool MediaTime::operator>(const MediaTime &time) const
	{
		return av_cmp_q(this->getRational(), time.getRational()) == 1;
	}

	bool MediaTime::operator==(const MediaTime &time) const
	{
		return av_cmp_q(this->getRational(), time.getRational()) == 0;
	}

	bool MediaTime::operator!=(const MediaTime &time) const
	{
		return (*this == time) == false;
	}

	bool MediaTime::operator<=(const MediaTime &time) const
	{
		return *this < time || *this == time;
	}

	bool MediaTime::operator>=(const MediaTime &time) const
	{
		return *this > time || *this == time;
	}

	MediaTime MediaTime::zero = MediaTime(0, 1);
}