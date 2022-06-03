#include "MediaTimeMapping.hpp"

namespace ks
{
	MediaTimeMapping::MediaTimeMapping()
	{
	}

	MediaTimeMapping::MediaTimeMapping(const MediaTimeRange source, const MediaTimeRange target)
		: source(source), target(target)
	{
	}
}