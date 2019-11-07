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

#define LCM std::LCM

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

template <class _R1, class _R2>
struct __no_overflow
{
private:
    static const intmax_t __gcd_n1_n2 = GCD<_R1::num, _R2::num>::value;
    static const intmax_t __gcd_d1_d2 = GCD<_R1::den, _R2::den>::value;
    static const intmax_t __n1 = _R1::num / __gcd_n1_n2;
    static const intmax_t __d1 = _R1::den / __gcd_d1_d2;
    static const intmax_t __n2 = _R2::num / __gcd_n1_n2;
    static const intmax_t __d2 = _R2::den / __gcd_d1_d2;
    static const intmax_t max = -((intmax_t(1) << (sizeof(intmax_t) * CHAR_BIT - 1)) + 1);

    template <intmax_t _Xp, intmax_t _Yp, bool __overflow>
    struct __mul    // __overflow == false
    {
        static const intmax_t value = _Xp * _Yp;
    };

    template <intmax_t _Xp, intmax_t _Yp>
    struct __mul<_Xp, _Yp, true>
    {
        static const intmax_t value = 1;
    };

public:
    static const bool value = (__n1 <= max / __d2) && (__n2 <= max / __d1);
    typedef std::ratio<__mul<__n1, __d2, !value>::value,
                  __mul<__n2, __d1, !value>::value> type;
};

// Cast
template <class _FromMetric, class _ToMetric,
          class _Period = typename std::ratio_divide<typename _FromMetric::period, typename _ToMetric::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __metric_cast;

template <class _FromMetric, class _ToMetric, class _Period>
struct __metric_cast<_FromMetric, _ToMetric, _Period, true, true>
{
    inline METRICCONSTEXPR
    _ToMetric operator()(const _FromMetric& __fd) const
    {
        return _ToMetric(static_cast<typename _ToMetric::rep>(__fd.count()));
    }
};

template <class _FromMetric, class _ToMetric, class _Period>
struct __metric_cast<_FromMetric, _ToMetric, _Period, true, false>
{
    inline METRICCONSTEXPR
    _ToMetric operator()(const _FromMetric& __fd) const
    {
        typedef typename std::common_type<typename _ToMetric::rep, typename _FromMetric::rep, intmax_t>::type _Ct;
        return _ToMetric(static_cast<typename _ToMetric::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromMetric, class _ToMetric, class _Period>
struct __metric_cast<_FromMetric, _ToMetric, _Period, false, true>
{
    inline METRICCONSTEXPR
    _ToMetric operator()(const _FromMetric& __fd) const
    {
        typedef typename std::common_type<typename _ToMetric::rep, typename _FromMetric::rep, intmax_t>::type _Ct;
        return _ToMetric(static_cast<typename _ToMetric::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromMetric, class _ToMetric, class _Period>
struct __metric_cast<_FromMetric, _ToMetric, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToMetric operator()(const _FromMetric& __fd) const
    {
        typedef typename std::common_type<typename _ToMetric::rep, typename _FromMetric::rep, intmax_t>::type _Ct;
        return _ToMetric(static_cast<typename _ToMetric::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};

// Metric ==
template <class _LhsMetric, class _RhsMetric>
struct __metric_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsMetric& __lhs, const _RhsMetric& __rhs) const
        {
            typedef typename std::common_type<_LhsMetric, _RhsMetric>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsMetric>
struct __metric_eq<_LhsMetric, _LhsMetric>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsMetric& __lhs, const _LhsMetric& __rhs) const
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
template <class _LhsMetric, class _RhsMetric>
struct __metric_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsMetric& __lhs, const _RhsMetric& __rhs) const
        {
            typedef typename std::common_type<_LhsMetric, _RhsMetric>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsMetric>
struct __metric_lt<_LhsMetric, _LhsMetric>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsMetric& __lhs, const _LhsMetric& __rhs) const
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

// Metric >
template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Metric <=
template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Metric >=
template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Metric +
template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type
operator+(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Metric -
template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type
operator-(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Metric *
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

template <template <typename...> class _Master, class _Metric, class _Rep, bool = __is_specialization<_Rep, _Master>::value>
struct __metric_divide_result
{
};

template <class _Metric, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Metric::rep, _Rep2>::type>::value>
struct __metric_divide_imp
{
};

template <template <typename...> class _Master, class _Rep1, class _Period, class _Rep2>
struct __metric_divide_imp<_Master<_Rep1, _Period>, _Rep2, true>
{
    typedef _Master<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <template <class, class> class _Master, class _Rep1, class _Period, class _Rep2>
struct __metric_divide_result<_Master, _Master<_Rep1, _Period>, _Rep2, false>
    : __metric_divide_imp<_Master<_Rep1, _Period>, _Rep2>
{
};

// Metric /
template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const _Master<_Rep1, _Period1>& __lhs, const _Master<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Master<_Rep1, _Period1>, _Master<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

template <template <typename...> class _Master, class _Rep1, class _Period, class _Rep2, typename std::enable_if<std::is_arithmetic<_Rep2>::value, int>::type = 0>
inline
METRICCONSTEXPR
typename __metric_divide_result<_Master, _Master<_Rep1, _Period>, _Rep2>::type
operator/(const _Master<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef _Master<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

// Metric %
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

