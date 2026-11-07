#pragma once

#include <spdlog/spdlog.h>

#include <cstdlib>

class Log
{
public:
	static spdlog::logger& GetLogger();
};

// Note: Messages use fmt syntax, for example GAIA_INFO( "Loaded {} in {}ms", name, time )
// Note: INFO reports normal progress and details of the running environment
#define GAIA_INFO( ... ) ::Log::GetLogger().info( __VA_ARGS__ )
// Note: WARN reports something wrong where the returned result is still acceptable to use
#define GAIA_WARN( ... ) ::Log::GetLogger().warn( __VA_ARGS__ )
// Note: ERROR reports a failed operation, the caller gets false and execution continues without the result
#define GAIA_ERROR( ... ) ::Log::GetLogger().error( __VA_ARGS__ )

#if defined( NDEBUG )
#define GAIA_DEBUG_BREAK() ( (void)0 )
#elif defined( _WIN32 )
#include <intrin.h>
#define GAIA_DEBUG_BREAK() __debugbreak()
#else
#include <csignal>
#define GAIA_DEBUG_BREAK() raise( SIGTRAP )
#endif

// Note: ASSERT stops the program in every build, Debug hits a breakpoint first to allow inspecting the state
#define GAIA_ASSERT( condition )                                                                       \
	do                                                                                                 \
	{                                                                                                  \
		if( !( condition ) )                                                                           \
		{                                                                                              \
			::Log::GetLogger().critical( "Assertion failed: {} ({}:{})", #condition, __FILE__, __LINE__ ); \
			GAIA_DEBUG_BREAK();                                                                        \
			std::abort();                                                                              \
		}                                                                                              \
	} while( false )
