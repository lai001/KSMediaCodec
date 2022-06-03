#ifndef KSMediaCodec_FFmpeg_h
#define KSMediaCodec_FFmpeg_h

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

extern "C"
{
#include <libavcodec/avcodec.h>

#include <libswscale/swscale.h>

#include <libavdevice/avdevice.h>

#include <libswresample/swresample.h>

#include <libavformat/version.h>
#include <libavformat/avformat.h>

#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}

#endif // KSMediaCodec_FFmpeg_h