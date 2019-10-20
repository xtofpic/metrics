// -*- C++ -*-
//
//===---------------------------- volumetricflow ------------------------------------===//
//
// Copyright (c) 2018, 2019, Christophe Pijcke
//
// This file was largely inspired by the chrono library
//  from the LLVM Compiler Infrastructure.
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses.
//
//===----------------------------------------------------------------------===//

#ifndef METRICS_VOLUMETRICFLOW_HPP
#define METRICS_VOLUMETRICFLOW_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class volumetricflow;

template <typename A> struct __is_volumetricflow: __is_specialization<A, volumetricflow> {};

} // namespace metric



namespace std {

/*
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
*/




template <template <typename...> class _Master, class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< _Master<_Rep1, _Period1>,
                    _Master<_Rep2, _Period2> >
{
            typedef _Master<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};



/*
template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::volumetricflow<_Rep1, _Period1>,
                    metric::volumetricflow<_Rep2, _Period2> >
{
            typedef metric::volumetricflow<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
}; */

} // namespace std


namespace metric {


// volumetricflow_cast

template <class _FromVolumetricFlow, class _ToVolumetricFlow,
          class _Period = typename std::ratio_divide<typename _FromVolumetricFlow::period, typename _ToVolumetricFlow::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __volumetricflow_cast;

template <class _FromVolumetricFlow, class _ToVolumetricFlow, class _Period>
struct __volumetricflow_cast<_FromVolumetricFlow, _ToVolumetricFlow, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToVolumetricFlow operator()(const _FromVolumetricFlow& __fd) const
    {   
        return _ToVolumetricFlow(static_cast<typename _ToVolumetricFlow::rep>(__fd.count()));
    }
};

template <class _FromVolumetricFlow, class _ToVolumetricFlow, class _Period>
struct __volumetricflow_cast<_FromVolumetricFlow, _ToVolumetricFlow, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToVolumetricFlow operator()(const _FromVolumetricFlow& __fd) const
    {   
        typedef typename std::common_type<typename _ToVolumetricFlow::rep, typename _FromVolumetricFlow::rep, intmax_t>::type _Ct;
        return _ToVolumetricFlow(static_cast<typename _ToVolumetricFlow::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromVolumetricFlow, class _ToVolumetricFlow, class _Period>
struct __volumetricflow_cast<_FromVolumetricFlow, _ToVolumetricFlow, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToVolumetricFlow operator()(const _FromVolumetricFlow& __fd) const
    {   
        typedef typename std::common_type<typename _ToVolumetricFlow::rep, typename _FromVolumetricFlow::rep, intmax_t>::type _Ct;
        return _ToVolumetricFlow(static_cast<typename _ToVolumetricFlow::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromVolumetricFlow, class _ToVolumetricFlow, class _Period>
struct __volumetricflow_cast<_FromVolumetricFlow, _ToVolumetricFlow, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToVolumetricFlow operator()(const _FromVolumetricFlow& __fd) const
    {
        typedef typename std::common_type<typename _ToVolumetricFlow::rep, typename _FromVolumetricFlow::rep, intmax_t>::type _Ct;
        return _ToVolumetricFlow(static_cast<typename _ToVolumetricFlow::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToVolumetricFlow, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_volumetricflow<_ToVolumetricFlow>::value,
    _ToVolumetricFlow
>::type
volumetricflow_cast(const volumetricflow<_Rep, _Period>& __fd)
{
    return __volumetricflow_cast<volumetricflow<_Rep, _Period>, _ToVolumetricFlow>()(__fd);
}

template <class _Rep, class _Period>
class volumetricflow
{
    static_assert(!__is_volumetricflow<_Rep>::value, "A volumetricflow representation can not be a volumetricflow");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of volumetricflow must be a std::ratio");
    static_assert(_Period::num > 0, "volumetricflow period must be positive");

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

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    volumetricflow() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit volumetricflow(const _Rep2& __r,
            typename std::enable_if
            <
               std::is_convertible<_Rep2, rep>::value &&
               (std::is_floating_point<rep>::value ||
               !std::is_floating_point<_Rep2>::value)
            >::type* = 0)
                : __rep_(__r) {}

    // conversions
    template <class _Rep2, class _Period2>
        inline METRICCONSTEXPR
        volumetricflow(const volumetricflow<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::volumetricflow_cast<volumetricflow>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR volumetricflow  operator+() const {return *this;}
    inline METRICCONSTEXPR volumetricflow  operator-() const {return volumetricflow(-__rep_);}
    inline const volumetricflow& operator++()      {++__rep_; return *this;}
    inline const volumetricflow  operator++(int)   {return volumetricflow(__rep_++);}
    inline const volumetricflow& operator--()      {--__rep_; return *this;}
    inline const volumetricflow  operator--(int)   {return volumetricflow(__rep_--);}

    inline const volumetricflow& operator+=(const volumetricflow& __d) {__rep_ += __d.count(); return *this;}
    inline const volumetricflow& operator-=(const volumetricflow& __d) {__rep_ -= __d.count(); return *this;}

    inline const volumetricflow& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const volumetricflow& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const volumetricflow& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const volumetricflow& operator%=(const volumetricflow& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR volumetricflow zero() {return volumetricflow(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR volumetricflow min()  {return volumetricflow(limits_values<rep>::min());}
    inline static METRICCONSTEXPR volumetricflow max()  {return volumetricflow(limits_values<rep>::max());}
};


typedef volumetricflow<long long, std::ratio<86400, 1000> > microlitre_second;
typedef volumetricflow<long long, std::ratio< 1440, 1000> > microlitre_minute;
typedef volumetricflow<long long, std::ratio<   24, 1000> > microlitre_hour;
typedef volumetricflow<long long, std::ratio<86400,    1> > millilitre_second;
typedef volumetricflow<long long, std::ratio< 1440,    1> > millilitre_minute;
typedef volumetricflow<long long, std::ratio<   24,    1> > millilitre_hour;
typedef volumetricflow<long long                          > millilitre_day;


namespace literals {

constexpr microlitre_second operator ""_ul_sec(unsigned long long v) { return microlitre_second(v); }
constexpr microlitre_minute operator ""_ul_m(unsigned long long v)   { return microlitre_minute(v); }
constexpr microlitre_hour   operator ""_ul_h(unsigned long long v)   { return microlitre_hour(v);   }
constexpr millilitre_second operator ""_ml_sec(unsigned long long v) { return millilitre_second(v); }
constexpr millilitre_minute operator ""_ml_m(unsigned long long v)   { return millilitre_minute(v); }
constexpr millilitre_hour   operator ""_ml_h(unsigned long long v)   { return millilitre_hour(v);   }
constexpr millilitre_day    operator ""_ml_d(unsigned long long v)   { return millilitre_day(v);    }

} // namespace literals


// VolumetricFlow ==

template <class _LhsVolumetricFlow, class _RhsVolumetricFlow>
struct __volumetricflow_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsVolumetricFlow& __lhs, const _RhsVolumetricFlow& __rhs) const
        {
            typedef typename std::common_type<_LhsVolumetricFlow, _RhsVolumetricFlow>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsVolumetricFlow>
struct __volumetricflow_eq<_LhsVolumetricFlow, _LhsVolumetricFlow>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsVolumetricFlow& __lhs, const _LhsVolumetricFlow& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    return __volumetricflow_eq<volumetricflow<_Rep1, _Period1>, volumetricflow<_Rep2, _Period2> >()(__lhs, __rhs);
}

// VolumetricFlow !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// VolumetricFlow <

template <class _LhsVolumetricFlow, class _RhsVolumetricFlow>
struct __volumetricflow_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsVolumetricFlow& __lhs, const _RhsVolumetricFlow& __rhs) const
        {
            typedef typename std::common_type<_LhsVolumetricFlow, _RhsVolumetricFlow>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsVolumetricFlow>
struct __volumetricflow_lt<_LhsVolumetricFlow, _LhsVolumetricFlow>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsVolumetricFlow& __lhs, const _LhsVolumetricFlow& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    return __volumetricflow_lt<volumetricflow<_Rep1, _Period1>, volumetricflow<_Rep2, _Period2> >()(__lhs, __rhs);
}

// VolumetricFlow >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// VolumetricFlow <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// VolumetricFlow >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// VolumetricFlow +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<volumetricflow<_Rep1, _Period1>, volumetricflow<_Rep2, _Period2> >::type
operator+(const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<volumetricflow<_Rep1, _Period1>, volumetricflow<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// VolumetricFlow -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<volumetricflow<_Rep1, _Period1>, volumetricflow<_Rep2, _Period2> >::type
operator-(const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<volumetricflow<_Rep1, _Period1>, volumetricflow<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// VolumetricFlow *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    volumetricflow<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const volumetricflow<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef volumetricflow<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    volumetricflow<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const volumetricflow<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// VolumetricFlow /

template <class _VolumetricFlow, class _Rep, bool = __is_volumetricflow<_Rep>::value>
struct __volumetricflow_divide_result
{
};

template <class _VolumetricFlow, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _VolumetricFlow::rep, _Rep2>::type>::value>
struct __volumetricflow_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __volumetricflow_divide_imp<volumetricflow<_Rep1, _Period>, _Rep2, true>
{
    typedef volumetricflow<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __volumetricflow_divide_result<volumetricflow<_Rep1, _Period>, _Rep2, false>
    : __volumetricflow_divide_imp<volumetricflow<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __volumetricflow_divide_result<volumetricflow<_Rep1, _Period>, _Rep2>::type
operator/(const volumetricflow<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef volumetricflow<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<volumetricflow<_Rep1, _Period1>, volumetricflow<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// VolumetricFlow %

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __volumetricflow_divide_result<volumetricflow<_Rep1, _Period>, _Rep2>::type
operator%(const volumetricflow<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef volumetricflow<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<volumetricflow<_Rep1, _Period1>, volumetricflow<_Rep2, _Period2> >::type
operator%(const volumetricflow<_Rep1, _Period1>& __lhs, const volumetricflow<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<volumetricflow<_Rep1, _Period1>, volumetricflow<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_VOLUMETRICFLOW_HPP


