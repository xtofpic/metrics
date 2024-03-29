// -*- C++ -*-
//
//===---------------------------- frequency ------------------------------------===//
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

#ifndef METRICS_FREQUENCY_HPP
#define METRICS_FREQUENCY_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class frequency;

template <typename A> struct __is_frequency: __is_specialization<A, frequency> {};


template <class _ToFrequency, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_frequency<_ToFrequency>::value,
    _ToFrequency
>::type
frequency_cast(const frequency<_Rep, _Period>& __fd)
{
    return __metric_cast<frequency<_Rep, _Period>, _ToFrequency>()(__fd);
}

template <class _Rep, class _Period>
class frequency
{
    static_assert(!__is_frequency<_Rep>::value, "A frequency representation can not be a frequency");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of frequency must be a std::ratio");
    static_assert(_Period::num > 0, "frequency period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    frequency() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit frequency(const _Rep2& __r,
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
		frequency(const frequency<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::frequency_cast<frequency>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR frequency  operator+() const {return *this;}
    inline METRICCONSTEXPR frequency  operator-() const {return frequency(-__rep_);}
    inline const frequency& operator++()      {++__rep_; return *this;}
    inline const frequency  operator++(int)   {return frequency(__rep_++);}
    inline const frequency& operator--()      {--__rep_; return *this;}
    inline const frequency  operator--(int)   {return frequency(__rep_--);}

    inline const frequency& operator+=(const frequency& __d) {__rep_ += __d.count(); return *this;}
    inline const frequency& operator-=(const frequency& __d) {__rep_ -= __d.count(); return *this;}

    inline const frequency& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const frequency& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const frequency& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const frequency& operator%=(const frequency& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR frequency zero() {return frequency(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR frequency min()  {return frequency(limits_values<rep>::min());}
    inline static METRICCONSTEXPR frequency max()  {return frequency(limits_values<rep>::max());}
};


typedef frequency<long long, std::milli> millihertz;
typedef frequency<long long            > hertz;
typedef frequency<long long, std::kilo > kilohertz;
typedef frequency<long     , std::mega > megahertz;
typedef frequency<long     , std::giga > gigahertz;

namespace literals {
constexpr millihertz operator ""_mHz(unsigned long long v)  { return millihertz(v); }
constexpr      hertz operator ""_Hz( unsigned long long v)  { return hertz(v);      }
constexpr  kilohertz operator ""_kHz(unsigned long long v)  { return kilohertz(v);  }
constexpr  megahertz operator ""_MHz(unsigned long long v)  { return megahertz(v);  }
constexpr  gigahertz operator ""_GHz(unsigned long long v)  { return gigahertz(v);  }
} // namespace literals

} // namespace metric

#endif // METRICS_FREQUENCY_HPP


