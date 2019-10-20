// -*- C++ -*-
//
//===---------------------------- speed ------------------------------------===//
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

#ifndef METRICS_SPEED_HPP
#define METRICS_SPEED_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class speed;

template <typename A> struct __is_speed: __is_specialization<A, speed> {};


template <class _ToSpeed, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_speed<_ToSpeed>::value,
    _ToSpeed
>::type
speed_cast(const speed<_Rep, _Period>& __fd)
{
    return __metric_cast<speed<_Rep, _Period>, _ToSpeed>()(__fd);
}

template <class _Rep, class _Period>
class speed
{
    static_assert(!__is_speed<_Rep>::value, "A speed representation can not be a speed");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of speed must be a std::ratio");
    static_assert(_Period::num > 0, "speed period must be positive");

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
    speed() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit speed(const _Rep2& __r,
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
        speed(const speed<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::speed_cast<speed>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR speed  operator+() const {return *this;}
    inline METRICCONSTEXPR speed  operator-() const {return speed(-__rep_);}
    inline const speed& operator++()      {++__rep_; return *this;}
    inline const speed  operator++(int)   {return speed(__rep_++);}
    inline const speed& operator--()      {--__rep_; return *this;}
    inline const speed  operator--(int)   {return speed(__rep_--);}

    inline const speed& operator+=(const speed& __d) {__rep_ += __d.count(); return *this;}
    inline const speed& operator-=(const speed& __d) {__rep_ -= __d.count(); return *this;}

    inline const speed& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const speed& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const speed& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const speed& operator%=(const speed& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR speed zero() {return speed(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR speed min()  {return speed(limits_values<rep>::min());}
    inline static METRICCONSTEXPR speed max()  {return speed(limits_values<rep>::max());}
};


// Speed /
template <class _Speed, class _Rep, bool = __is_speed<_Rep>::value>
struct __speed_divide_result
{
};

template <class _Speed, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Speed::rep, _Rep2>::type>::value>
struct __speed_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __speed_divide_imp<speed<_Rep1, _Period>, _Rep2, true>
{
    typedef speed<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __speed_divide_result<speed<_Rep1, _Period>, _Rep2, false>
    : __speed_divide_imp<speed<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __speed_divide_result<speed<_Rep1, _Period>, _Rep2>::type
operator/(const speed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef speed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}


// Speed %
template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __speed_divide_result<speed<_Rep1, _Period>, _Rep2>::type
operator%(const speed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef speed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}


typedef speed<long long, std::ratio<86400, 1000000> > micrometre_second;
typedef speed<long long, std::ratio< 1440, 1000000> > micrometre_minute;
typedef speed<long long, std::ratio<   24, 1000000> > micrometre_hour;
typedef speed<long long, std::ratio<86400,    1000> > millimetre_second;
typedef speed<long long, std::ratio< 1440,    1000> > millimetre_minute;
typedef speed<long long, std::ratio<   24,    1000> > millimetre_hour;
typedef speed<long long, std::ratio<    1,    1000> > millimetre_day;
typedef speed<long long, std::ratio<86400,       1> > metre_second;
typedef speed<long long, std::ratio< 1440,       1> > metre_minute;
typedef speed<long long, std::ratio<   24,       1> > metre_hour;
typedef speed<long long, std::ratio<    1,       1> > metre_day;

namespace literals {
constexpr micrometre_second operator ""_um_sec(unsigned long long v) { return   micrometre_second(v); }
constexpr micrometre_minute operator ""_um_m(unsigned long long v)   { return   micrometre_minute(v); }
constexpr   micrometre_hour operator ""_um_h(unsigned long long v)   { return   micrometre_hour(v);   }
constexpr millimetre_second operator ""_mm_sec(unsigned long long v) { return   millimetre_second(v); }
constexpr millimetre_minute operator ""_mm_m(unsigned long long v)   { return   millimetre_minute(v); }
constexpr   millimetre_hour operator ""_mm_h(unsigned long long v)   { return   millimetre_hour(v);   }
constexpr    millimetre_day operator ""_mm_d(unsigned long long v)   { return   millimetre_day(v);    }
constexpr      metre_second operator ""_m_sec(unsigned long long v)  { return   metre_second(v);      }
constexpr      metre_minute operator ""_m_m(unsigned long long v)    { return   metre_minute(v);      }
constexpr        metre_hour operator ""_m_h(unsigned long long v)    { return   metre_hour(v);        }
constexpr         metre_day operator ""_m_d(unsigned long long v)    { return   metre_day(v);         }
} // namespace literals

} // namespace metric

#endif // METRICS_SPEED_HPP


