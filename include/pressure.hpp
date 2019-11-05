// -*- C++ -*-
//
//===---------------------------- pressure ------------------------------------===//
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

#ifndef METRICS_PRESSURE_HPP
#define METRICS_PRESSURE_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class pressure;

template <typename A> struct __is_pressure: __is_specialization<A, pressure> {};


template <class _ToPressure, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_pressure<_ToPressure>::value,
    _ToPressure
>::type
pressure_cast(const pressure<_Rep, _Period>& __fd)
{
    return __metric_cast<pressure<_Rep, _Period>, _ToPressure>()(__fd);
}

template <class _Rep, class _Period>
class pressure
{
    static_assert(!__is_pressure<_Rep>::value, "A pressure representation can not be a pressure");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of pressure must be a std::ratio");
    static_assert(_Period::num > 0, "pressure period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    pressure() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit pressure(const _Rep2& __r,
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
		pressure(const pressure<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::pressure_cast<pressure>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR pressure  operator+() const {return *this;}
    inline METRICCONSTEXPR pressure  operator-() const {return pressure(-__rep_);}
    inline const pressure& operator++()      {++__rep_; return *this;}
    inline const pressure  operator++(int)   {return pressure(__rep_++);}
    inline const pressure& operator--()      {--__rep_; return *this;}
    inline const pressure  operator--(int)   {return pressure(__rep_--);}

    inline const pressure& operator+=(const pressure& __d) {__rep_ += __d.count(); return *this;}
    inline const pressure& operator-=(const pressure& __d) {__rep_ -= __d.count(); return *this;}

    inline const pressure& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const pressure& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const pressure& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const pressure& operator%=(const pressure& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR pressure zero() {return pressure(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR pressure min()  {return pressure(limits_values<rep>::min());}
    inline static METRICCONSTEXPR pressure max()  {return pressure(limits_values<rep>::max());}
};


// Pressure /
template <class _Pressure, class _Rep, bool = __is_pressure<_Rep>::value>
struct __pressure_divide_result
{
};

template <class _Pressure, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Pressure::rep, _Rep2>::type>::value>
struct __pressure_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __pressure_divide_imp<pressure<_Rep1, _Period>, _Rep2, true>
{
    typedef pressure<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __pressure_divide_result<pressure<_Rep1, _Period>, _Rep2, false>
    : __pressure_divide_imp<pressure<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __pressure_divide_result<pressure<_Rep1, _Period>, _Rep2>::type
operator/(const pressure<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef pressure<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}


// Pressure %
template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __pressure_divide_result<pressure<_Rep1, _Period>, _Rep2>::type
operator%(const pressure<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef pressure<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

typedef pressure<long long, std::ratio<            1,     760> > millimetremercury; // Torr ou mmHg;
#ifdef _WIN32
typedef pressure<long long, std::ratio<            1,  101325> > pascl;
#else
typedef pressure<long long, std::ratio<            1,  101325> > pascal;
#endif
typedef pressure<long long, std::ratio<          100,  101325> > hectopascal;
typedef pressure<long long, std::ratio<         1000,  101325> > kilopascal;
typedef pressure<long long, std::ratio<      1000000,  101325> > megapascal;
typedef pressure<long long, std::ratio<   1000000000,  101325> > gigapascal;
typedef pressure<long long, std::ratio<1000000000000,  101325> > terapascal;
typedef pressure<long long, std::ratio<      1000000, 1013250> > bar;
typedef pressure<long long, std::ratio<         1000, 1013250> > millibar;
typedef pressure<long long, std::ratio<            1, 1013250> > microbar;

namespace literals {
constexpr millimetremercury operator ""_mmHg(unsigned long long v) { return millimetremercury(v); }
#ifdef _WIN32
constexpr            pascl operator ""_Pa(  unsigned long long v) { return            pascl(v); }
#else
constexpr            pascal operator ""_Pa(  unsigned long long v) { return            pascal(v); }
#endif
constexpr       hectopascal operator ""_hPa( unsigned long long v) { return       hectopascal(v); }
constexpr        kilopascal operator ""_kPa( unsigned long long v) { return        kilopascal(v); }
constexpr        megapascal operator ""_MPa( unsigned long long v) { return        megapascal(v); }
constexpr        gigapascal operator ""_GPa( unsigned long long v) { return        gigapascal(v); }
constexpr        terapascal operator ""_TPa( unsigned long long v) { return        terapascal(v); }
constexpr               bar operator ""_bar( unsigned long long v) { return               bar(v); }
constexpr          millibar operator ""_mbar(unsigned long long v) { return          millibar(v); }
constexpr          microbar operator ""_ubar(unsigned long long v) { return          microbar(v); }
} // namespace literals

} // namespace metric

#endif // METRICS_PRESSURE_HPP


