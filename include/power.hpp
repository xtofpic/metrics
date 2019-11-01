// -*- C++ -*-
//
//===---------------------------- power ------------------------------------===//
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

#ifndef METRICS_POWER_HPP
#define METRICS_POWER_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class power;

template <typename A> struct __is_power: __is_specialization<A, power> {};


template <class _ToPower, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_power<_ToPower>::value,
    _ToPower
>::type
power_cast(const power<_Rep, _Period>& __fd)
{
    return __metric_cast<power<_Rep, _Period>, _ToPower>()(__fd);
}

template <class _Rep, class _Period>
class power
{
    static_assert(!__is_power<_Rep>::value, "A power representation can not be a power");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of power must be a std::ratio");
    static_assert(_Period::num > 0, "power period must be positive");

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
    power() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit power(const _Rep2& __r,
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
        power(const power<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::power_cast<power>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR power  operator+() const {return *this;}
    inline METRICCONSTEXPR power  operator-() const {return power(-__rep_);}
    inline const power& operator++()      {++__rep_; return *this;}
    inline const power  operator++(int)   {return power(__rep_++);}
    inline const power& operator--()      {--__rep_; return *this;}
    inline const power  operator--(int)   {return power(__rep_--);}

    inline const power& operator+=(const power& __d) {__rep_ += __d.count(); return *this;}
    inline const power& operator-=(const power& __d) {__rep_ -= __d.count(); return *this;}

    inline const power& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const power& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const power& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const power& operator%=(const power& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR power zero() {return power(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR power min()  {return power(limits_values<rep>::min());}
    inline static METRICCONSTEXPR power max()  {return power(limits_values<rep>::max());}
};


// Power /
template <class _Power, class _Rep, bool = __is_power<_Rep>::value>
struct __power_divide_result
{
};

template <class _Power, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Power::rep, _Rep2>::type>::value>
struct __power_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __power_divide_imp<power<_Rep1, _Period>, _Rep2, true>
{
    typedef power<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __power_divide_result<power<_Rep1, _Period>, _Rep2, false>
    : __power_divide_imp<power<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __power_divide_result<power<_Rep1, _Period>, _Rep2>::type
operator/(const power<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef power<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}


// Power %
template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __power_divide_result<power<_Rep1, _Period>, _Rep2>::type
operator%(const power<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef power<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

typedef power<long long, std::nano > nanowatt;
typedef power<long long, std::micro> microwatt;
typedef power<long long, std::milli> milliwatt;
typedef power<long long            > watt;
typedef power<long long, std::kilo > kilowatt;
typedef power<long long, std::mega > megawatt;
typedef power<long long, std::giga > gigawatt;
typedef power<long long, std::tera > terawatt;
typedef power<long long, std::peta > petawatt;

namespace literals {
constexpr  nanowatt operator ""_nW(unsigned long long v) { return   nanowatt(v); }
constexpr microwatt operator ""_uW(unsigned long long v) { return  microwatt(v); }
constexpr milliwatt operator ""_mW(unsigned long long v) { return  milliwatt(v); }
constexpr      watt operator ""_W( unsigned long long v) { return       watt(v); }
constexpr  kilowatt operator ""_kW(unsigned long long v) { return   kilowatt(v); }
constexpr  megawatt operator ""_MW(unsigned long long v) { return   megawatt(v); }
constexpr  gigawatt operator ""_GW(unsigned long long v) { return   gigawatt(v); }
constexpr  terawatt operator ""_TW(unsigned long long v) { return   terawatt(v); }
constexpr  petawatt operator ""_PW(unsigned long long v) { return   petawatt(v); }
} // namespace literals

} // namespace metric

#endif // METRICS_POWER_HPP


