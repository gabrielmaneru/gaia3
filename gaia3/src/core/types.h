#pragma once

#include <memory>
#include <utility>

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

template <typename T>
using unique = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr unique<T> new_unique( Args&&... args )
{
	return std::make_unique<T>( std::forward<Args>( args )... );
}

template <typename T>
using shared = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr shared<T> new_shared( Args&&... args )
{
	return std::make_shared<T>( std::forward<Args>( args )... );
}

struct rect
{
	int x;
	int y;
};
