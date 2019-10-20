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


namespace metric
{



template <class _FromAngularSpeed, class _ToAngularSpeed,
          class _Period = typename std::ratio_divide<typename _FromAngularSpeed::period, typename _ToAngularSpeed::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __metric_cast;




template <class _FromAngularSpeed, class _ToAngularSpeed, class _Period>
struct __metric_cast<_FromAngularSpeed, _ToAngularSpeed, _Period, true, true>
{
    inline METRICCONSTEXPR
    _ToAngularSpeed operator()(const _FromAngularSpeed& __fd) const
    {
        return _ToAngularSpeed(static_cast<typename _ToAngularSpeed::rep>(__fd.count()));
    }
};

template <class _FromAngularSpeed, class _ToAngularSpeed, class _Period>
struct __metric_cast<_FromAngularSpeed, _ToAngularSpeed, _Period, true, false>
{
    inline METRICCONSTEXPR
    _ToAngularSpeed operator()(const _FromAngularSpeed& __fd) const
    {
        typedef typename std::common_type<typename _ToAngularSpeed::rep, typename _FromAngularSpeed::rep, intmax_t>::type _Ct;
        return _ToAngularSpeed(static_cast<typename _ToAngularSpeed::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromAngularSpeed, class _ToAngularSpeed, class _Period>
struct __metric_cast<_FromAngularSpeed, _ToAngularSpeed, _Period, false, true>
{
    inline METRICCONSTEXPR
    _ToAngularSpeed operator()(const _FromAngularSpeed& __fd) const
    {
        typedef typename std::common_type<typename _ToAngularSpeed::rep, typename _FromAngularSpeed::rep, intmax_t>::type _Ct;
        return _ToAngularSpeed(static_cast<typename _ToAngularSpeed::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromAngularSpeed, class _ToAngularSpeed, class _Period>
struct __metric_cast<_FromAngularSpeed, _ToAngularSpeed, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToAngularSpeed operator()(const _FromAngularSpeed& __fd) const
    {
        typedef typename std::common_type<typename _ToAngularSpeed::rep, typename _FromAngularSpeed::rep, intmax_t>::type _Ct;
        return _ToAngularSpeed(static_cast<typename _ToAngularSpeed::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};







// Metric ==
template <class _LhsAngularSpeed, class _RhsAngularSpeed>
struct __metric_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsAngularSpeed& __lhs, const _RhsAngularSpeed& __rhs) const
        {
            typedef typename std::common_type<_LhsAngularSpeed, _RhsAngularSpeed>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsAngularSpeed>
struct __metric_eq<_LhsAngularSpeed, _LhsAngularSpeed>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsAngularSpeed& __lhs, const _LhsAngularSpeed& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    return __metric_eq<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Metric !=
template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}


// Metric <
template <class _LhsAngularSpeed, class _RhsAngularSpeed>
struct __metric_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsAngularSpeed& __lhs, const _RhsAngularSpeed& __rhs) const
        {
            typedef typename std::common_type<_LhsAngularSpeed, _RhsAngularSpeed>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsAngularSpeed>
struct __metric_lt<_LhsAngularSpeed, _LhsAngularSpeed>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsAngularSpeed& __lhs, const _LhsAngularSpeed& __rhs) const
        {return __lhs.count() < __rhs.count();}
};


template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    return __metric_lt<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >()(__lhs, __rhs);
}

// AngularSpeed >
template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// AngularSpeed <=

template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// AngularSpeed >=

template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// AngularSpeed +

template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type
operator+(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// AngularSpeed -

template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type
operator-(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// AngularSpeed *

template <template <typename...> class _Master, class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
	_Master<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Master<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef _Master<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <template <typename...> class _Master, class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
	_Master<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const _Master<_Rep2, _Period>& __d)
{
    return __d * __s;
}



/*
// AngularSpeed /

template <class _AngularSpeed, class _Rep, bool = __is_angularspeed<_Rep>::value>
struct __angularspeed_divide_result
{
};

template <class _AngularSpeed, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _AngularSpeed::rep, _Rep2>::type>::value>
struct __angularspeed_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __angularspeed_divide_imp<angularspeed<_Rep1, _Period>, _Rep2, true>
{
    typedef angularspeed<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __angularspeed_divide_result<angularspeed<_Rep1, _Period>, _Rep2, false>
    : __angularspeed_divide_imp<angularspeed<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __angularspeed_divide_result<angularspeed<_Rep1, _Period>, _Rep2>::type
operator/(const angularspeed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef angularspeed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}


// AngularSpeed %

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __angularspeed_divide_result<angularspeed<_Rep1, _Period>, _Rep2>::type
operator%(const angularspeed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef angularspeed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}


*/


template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}



template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type
operator%(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}




}

#endif // METRICS_CONFIG_HPP

