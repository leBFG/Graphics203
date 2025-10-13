#pragma once

#include "Skateboard/Utilities/StringConverters.h"

// <summary>
// Informs the compiler not to generate a copy constructor and copy assignment operator.
// </summary>
#ifndef DISABLE_COPY
#define DISABLE_COPY(T)\
	    T(const T&) = delete;\
		auto operator=(const T&) noexcept -> T& = delete;
#endif

// <summary>
// Informs the compiler not to generate a move constructor and move assignment operator.
// </summary>
#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T)\
	    T(T&&) = delete;\
		auto operator=(T&&) noexcept -> T& = delete;
#endif

#ifndef ENABLE_MOVE
#define ENABLE_MOVE(T)\
	    T(T&&) = default;\
		auto operator=(T&&) noexcept -> T& = default;
#endif


#ifndef DISBALE_DEFAULT
#define DISABLE_DEFAULT(T)\
		T() = delete;
#endif

#define BIT(x) 1 << x

//#define ROUND_UP(v, powerOf2Alignment)// (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))
//round up to the closest power of 2
constexpr  size_t ROUND_UP(size_t v, size_t powOf2)
{
	if(powOf2 == 0)
		return v;
	else
		return (((v)+(powOf2)-1) & ~((powOf2)-1));
};

#ifndef GENERATE_DEFAULT_CLASS
#define GENERATE_DEFAULT_CLASS(T)\
	T() = default;\
	T(const T&) = default;\
	T(T&&) noexcept = default;\
	auto operator=(const T&) -> T& = default;\
	auto operator=(T&&) noexcept -> T& = default;
#endif

#ifndef DISABLE_COPY_AND_MOVE
#define DISABLE_COPY_AND_MOVE(T) DISABLE_COPY(T) DISABLE_MOVE(T)
#endif

//copy move default
#ifndef DISABLE_C_M_D
#define DISABLE_C_M_D(T) DISABLE_COPY_AND_MOVE(T) DISABLE_DEFAULT(T)
#endif // !DISABLE_C_M_D

#if SKTBD_DYNAMIC_LINK
	#ifdef SKTBD_BUILD_DLL 
		#define SKTBD_API __declspec(dllexport)		
	#else 
		#define SKTBD_API __declspec(dllimport)
	#endif
#else 
	#define SKTBD_API
#endif

//Enum operands, communismed from winh

template <size_t S>
struct _Enum_Flag_Size;

template <>
struct _Enum_Flag_Size<1>
{
	typedef uint8_t type;
};

template <>
struct _Enum_Flag_Size<2>
{
	typedef uint16_t type;
};

template <>
struct _Enum_Flag_Size<4>
{
	typedef uint32_t type;
};

template <>
struct _Enum_Flag_Size<8>
{
	typedef uint64_t type;
};

// used as an approximation of std::underlying_type<T>
template <class T>
struct _EnumFlag_Sized_integer
{
	typedef typename _Enum_Flag_Size<sizeof(T)>::type type;
};

#define ENUM_FLAG_OPERATORS(ENUMTYPE) \
inline constexpr ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((_EnumFlag_Sized_integer<ENUMTYPE>::type)a) | ((_EnumFlag_Sized_integer<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE &)(((_EnumFlag_Sized_integer<ENUMTYPE>::type &)a) |= ((_EnumFlag_Sized_integer<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((_EnumFlag_Sized_integer<ENUMTYPE>::type)a) & ((_EnumFlag_Sized_integer<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE &)(((_EnumFlag_Sized_integer<ENUMTYPE>::type &)a) &= ((_EnumFlag_Sized_integer<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator ~ (ENUMTYPE a) noexcept { return ENUMTYPE(~((_EnumFlag_Sized_integer<ENUMTYPE>::type)a)); } \
inline constexpr ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((_EnumFlag_Sized_integer<ENUMTYPE>::type)a) ^ ((_EnumFlag_Sized_integer<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE &)(((_EnumFlag_Sized_integer<ENUMTYPE>::type &)a) ^= ((_EnumFlag_Sized_integer<ENUMTYPE>::type)b)); } \

namespace Skateboard::GraphicsConstants
{
	extern size_t DEFAULT_RESOURCE_ALIGNMENT;
	extern size_t SMALL_RESOURCE_ALIGNMENT;
	extern size_t MSAA_RESOURCE_ALIGNMENT;
	extern size_t SMALL_MSAA_RESOURCE_ALIGNMENT;
	extern size_t CONSTANT_BUFFER_ALIGNMENT;
	extern size_t BUFFER_ALIGNMENT;

	extern size_t RAYTRACING_STRUCT_ALIGNMENT;
	extern size_t RAYTRACING_TLAS_INSTANCE_DESC_ALIGNMENT;

	extern size_t RAYTRACING_SHADER_TABLE_ALIGNMENT;
	extern size_t RAYTRACING_SHADER_TABLE_SHADER_ID_ALIGNMENT;
	extern size_t RAYTRACING_SHADER_TABLE_RECORD_ALIGNMENT;
}

namespace Skateboard
{
	template<class T, template<class...> class U>
	inline constexpr bool is_instance_of_v = std::false_type();

	template<template<class...> class U, class... Vs>
	inline constexpr bool is_instance_of_v<U<Vs...>, U> = std::true_type();
}

//SKATEBOARD RESULT basically winerror
typedef uint32_t SKTBDR;

//everything.. seems to be in order
#define OKEYDOKEY 0;

//OOPSIE WOOPSIE
constexpr auto OOPS_OUTOFMEMORY = -1;
constexpr auto OOPS_INVALIDARG = -2;
constexpr auto OOPS_INVALIDPTR = -3;
constexpr auto OOPS_INVALIDHANDLE = -4;
constexpr auto OOPS_FAIL = -5;
constexpr auto OOPS_ACCESSDENIED =-6;
constexpr auto OOPS_UNEXPECTED =-7;
constexpr auto OOPS_RESOURCESIZEMISSMATCH = -8;

#if !defined(SKTBD_SHIP)
#define ASSERTIONS_ENABLED 1
#endif

#if ASSERTIONS_ENABLED
// check the expression and fail if it is false 
#define ASSERT(expr,...) if(expr){}else{ SKTBD_MSG_CRITICAL("Assertion failed: {0} in {1} at {2}", #expr, __FILE__, __LINE__); __VA_ARGS__; __debugbreak();  }
#define ASSERT_SIMPLE(expr, ...) if(expr){}else{ __VA_ARGS__; __debugbreak(); }
#else
// evaluates to nothing
#define ASSERT(expr)
#define ASSERT_SIMPLE(expr, ...)
#endif

#define SKTBD_ASSERT(A,...) ASSERT_SIMPLE(A, __VA_ARGS__)
#define SKTBD_LOG_ASSERT(A,...) ASSERT(A,SKTBD_MSG_CRITICAL(__VA_ARGS__)) 

namespace Skateboard
{
	constexpr bool is_aligned(off_t value, off_t alignment) { return value % alignment == 0; }
}


// Uncomment this definition to log the CPU idles (warning: may flood console as currently the CPU always
// waits on the GPU -> not enough CPU tasks!
//#define SKTBD_LOG_CPU_IDLE


