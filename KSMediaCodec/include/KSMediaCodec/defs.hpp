#ifndef KSMediaCodec_defs_hpp
#define KSMediaCodec_defs_hpp

#ifdef _WIN32
#ifdef KSMediaCodec_BUILD_DLL_EXPORT
#define KSMediaCodec_API __declspec(dllexport)
#elif (defined KSMediaCodec_DLL)
#define KSMediaCodec_API __declspec(dllimport)
#else
#define KSMediaCodec_API
#endif // KSMediaCodec_BUILD_DLL_EXPORT
#else
#define KSMediaCodec_API
#endif // _WIN32

#endif // !KSMediaCodec_defs_hpp