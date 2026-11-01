#pragma once

#include <core/types.h>

#include <string_view>

// Fowler–Noll–Vo hash function
constexpr u64 HashStringFnv1a( std::string_view value )
{
	u64 hash = 1469598103934665603ull;

	for( const char c : value )
	{
		hash ^= static_cast<u64>( c );
		hash *= 1099511628211ull;
	}
	return hash;
}

template <typename T>
constexpr u64 GetTypeId()
{
#if defined( _MSC_VER )
	constexpr std::string_view signature = __FUNCSIG__;
#elif defined( __clang__ ) || defined( __GNUC__ )
	constexpr std::string_view signature = __PRETTY_FUNCTION__;
#else
	constexpr std::string_view signature = "UnsupportedCompilerTypeId";
#endif

	return HashStringFnv1a( signature );
}
