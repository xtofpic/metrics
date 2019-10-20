// -*- C++ -*-
//
//===---------------------------- config ----------------------------------===//
//
// Copyright (c) 2018, 2019, Christophe Pijcke
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses.
//
//===----------------------------------------------------------------------===//

#ifndef METRICS_CONFIG_HPP
#define METRICS_CONFIG_HPP

#include <ratio>
#include <limits>


#ifdef _WIN32
	#if _MSC_VER > 1800
    	#define METRICCONSTEXPR constexpr
	#else
    	#define METRICCONSTEXPR
	#endif
	#define GCD std::_Gcd
#else
	#define METRICCONSTEXPR constexpr
	#define GCD std::__static_gcd
#endif


#ifdef __GNUC__
	#define CHAR_BIT __CHAR_BIT__
#endif


#ifdef __APPLE__
	#define LCM std::__static_lcm
#else

// _Lcm is defined in chrono on Windows.  But we don't want to necessary include chrono when we are using metrics.
namespace std
{

template <intmax_t _Xp, intmax_t _Yp>
struct LCM
{
    static const intmax_t value = _Xp / GCD<_Xp, _Yp>::value * _Yp;
};

template <class _Tp>                    struct __is_ratio                          : std::false_type {};
template <intmax_t _Num, intmax_t _Den> struct __is_ratio<std::ratio<_Num, _Den> > : std::true_type  {};

} // namespace std

#endif


namespace metric
{


template<typename _Type,
    template <typename...> class _Template>
struct __is_specialization
    : std::false_type
{};

template<template <typename...> class _Template,
    typename... _Types>
    struct __is_specialization<_Template<_Types...>, _Template>
    : std::true_type
{};


template <class _Rep>
struct limits_values
{
public:
    inline static METRICCONSTEXPR _Rep zero() {return _Rep(0);}
    inline static METRICCONSTEXPR _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static METRICCONSTEXPR _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};




}


namespace std
{

template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< _Master<_Rep1, _Period1>,
                    _Master<_Rep2, _Period2> >
{
            typedef _Master<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};


}

#endif // METRICS_CONFIG_HPP

