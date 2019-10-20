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

} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::pressure<_Rep1, _Period1>,
                    metric::pressure<_Rep2, _Period2> >
{
            typedef metric::pressure<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};

} // namespace std


namespace metric {


// pressure_cast

template <class _FromPressure, class _ToPressure,
          class _Period = typename std::ratio_divide<typename _FromPressure::period, typename _ToPressure::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __pressure_cast;

template <class _FromPressure, class _ToPressure, class _Period>
struct __pressure_cast<_FromPressure, _ToPressure, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToPressure operator()(const _FromPressure& __fd) const
    {   
        return _ToPressure(static_cast<typename _ToPressure::rep>(__fd.count()));
    }
};

template <class _FromPressure, class _ToPressure, class _Period>
struct __pressure_cast<_FromPressure, _ToPressure, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToPressure operator()(const _FromPressure& __fd) const
    {   
        typedef typename std::common_type<typename _ToPressure::rep, typename _FromPressure::rep, intmax_t>::type _Ct;
        return _ToPressure(static_cast<typename _ToPressure::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromPressure, class _ToPressure, class _Period>
struct __pressure_cast<_FromPressure, _ToPressure, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToPressure operator()(const _FromPressure& __fd) const
    {   
        typedef typename std::common_type<typename _ToPressure::rep, typename _FromPressure::rep, intmax_t>::type _Ct;
        return _ToPressure(static_cast<typename _ToPressure::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromPressure, class _ToPressure, class _Period>
struct __pressure_cast<_FromPressure, _ToPressure, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToPressure operator()(const _FromPressure& __fd) const
    {
        typedef typename std::common_type<typename _ToPressure::rep, typename _FromPressure::rep, intmax_t>::type _Ct;
        return _ToPressure(static_cast<typename _ToPressure::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


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
    return __pressure_cast<pressure<_Rep, _Period>, _ToPressure>()(__fd);
}


template <class _Rep>
struct pressure_values
{
public:
    inline static METRICCONSTEXPR _Rep zero() {return _Rep(0);}
    inline static METRICCONSTEXPR _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static METRICCONSTEXPR _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class pressure
{
    static_assert(!__is_pressure<_Rep>::value, "A pressure representation can not be a pressure");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of pressure must be a std::ratio");
    static_assert(_Period::num > 0, "pressure period must be positive");

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

    inline static METRICCONSTEXPR pressure zero() {return pressure(pressure_values<rep>::zero());}
    inline static METRICCONSTEXPR pressure min()  {return pressure(pressure_values<rep>::min());}
    inline static METRICCONSTEXPR pressure max()  {return pressure(pressure_values<rep>::max());}
};


typedef pressure<long long, std::ratio<            1,     760> > millimetremercure; // Torr ou mmHg;
typedef pressure<long long, std::ratio<            1,  101325> > pascal;
typedef pressure<long long, std::ratio<          100,  101325> > hectopascal;
typedef pressure<long long, std::ratio<         1000,  101325> > kilopascal;
typedef pressure<long long, std::ratio<      1000000,  101325> > megapascal;
typedef pressure<long long, std::ratio<   1000000000,  101325> > gigapascal;
typedef pressure<long long, std::ratio<1000000000000,  101325> > terapascal;
typedef pressure<long long, std::ratio<      1000000, 1013250> > bar;
typedef pressure<long long, std::ratio<         1000, 1013250> > millibar;
typedef pressure<long long, std::ratio<            1, 1013250> > microbar;


namespace literals {

constexpr millimetremercure operator ""_mmHg(unsigned long long v) { return   millimetremercure(v); }
constexpr            pascal operator ""_Pa(unsigned long long v)   { return   pascal(v); }
constexpr       hectopascal operator ""_hPa(unsigned long long v)  { return   hectopascal(v); }
constexpr        kilopascal operator ""_kPa(unsigned long long v)  { return   kilopascal(v); }
constexpr        megapascal operator ""_MPa(unsigned long long v)  { return   megapascal(v); }
constexpr        gigapascal operator ""_GPa(unsigned long long v)  { return   gigapascal(v); }
constexpr        terapascal operator ""_TPa(unsigned long long v)  { return   terapascal(v); }
constexpr               bar operator ""_bar(unsigned long long v)  { return   bar(v); }
constexpr          millibar operator ""_mbar(unsigned long long v) { return   millibar(v); }
constexpr          microbar operator ""_ubar(unsigned long long v) { return   microbar(v); }

} // namespace literals


// Pressure ==

template <class _LhsPressure, class _RhsPressure>
struct __pressure_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsPressure& __lhs, const _RhsPressure& __rhs) const
        {
            typedef typename std::common_type<_LhsPressure, _RhsPressure>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsPressure>
struct __pressure_eq<_LhsPressure, _LhsPressure>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsPressure& __lhs, const _LhsPressure& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    return __pressure_eq<pressure<_Rep1, _Period1>, pressure<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Pressure !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Pressure <

template <class _LhsPressure, class _RhsPressure>
struct __pressure_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsPressure& __lhs, const _RhsPressure& __rhs) const
        {
            typedef typename std::common_type<_LhsPressure, _RhsPressure>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsPressure>
struct __pressure_lt<_LhsPressure, _LhsPressure>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsPressure& __lhs, const _LhsPressure& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    return __pressure_lt<pressure<_Rep1, _Period1>, pressure<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Pressure >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Pressure <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Pressure >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Pressure +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<pressure<_Rep1, _Period1>, pressure<_Rep2, _Period2> >::type
operator+(const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<pressure<_Rep1, _Period1>, pressure<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Pressure -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<pressure<_Rep1, _Period1>, pressure<_Rep2, _Period2> >::type
operator-(const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<pressure<_Rep1, _Period1>, pressure<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Pressure *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    pressure<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const pressure<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef pressure<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    pressure<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const pressure<_Rep2, _Period>& __d)
{
    return __d * __s;
}

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

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<pressure<_Rep1, _Period1>, pressure<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
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

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<pressure<_Rep1, _Period1>, pressure<_Rep2, _Period2> >::type
operator%(const pressure<_Rep1, _Period1>& __lhs, const pressure<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<pressure<_Rep1, _Period1>, pressure<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_PRESSURE_HPP


