// -*- C++ -*-
//
//===---------------------------- voltage ------------------------------------===//
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

#ifndef METRICS_VOLTAGE_HPP
#define METRICS_VOLTAGE_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class voltage;

template <typename A> struct __is_voltage: __is_specialization<A, voltage> {};


template <class _ToVoltage, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_voltage<_ToVoltage>::value,
    _ToVoltage
>::type
voltage_cast(const voltage<_Rep, _Period>& __fd)
{
    return __metric_cast<voltage<_Rep, _Period>, _ToVoltage>()(__fd);
}

template <class _Rep, class _Period>
class voltage
{
    static_assert(!__is_voltage<_Rep>::value, "A voltage representation can not be a voltage");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of voltage must be a std::ratio");
    static_assert(_Period::num > 0, "voltage period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    voltage() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit voltage(const _Rep2& __r,
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
		voltage(const voltage<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::voltage_cast<voltage>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR voltage  operator+() const {return *this;}
    inline METRICCONSTEXPR voltage  operator-() const {return voltage(-__rep_);}
    inline const voltage& operator++()      {++__rep_; return *this;}
    inline const voltage  operator++(int)   {return voltage(__rep_++);}
    inline const voltage& operator--()      {--__rep_; return *this;}
    inline const voltage  operator--(int)   {return voltage(__rep_--);}

    inline const voltage& operator+=(const voltage& __d) {__rep_ += __d.count(); return *this;}
    inline const voltage& operator-=(const voltage& __d) {__rep_ -= __d.count(); return *this;}

    inline const voltage& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const voltage& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const voltage& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const voltage& operator%=(const voltage& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR voltage zero() {return voltage(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR voltage min()  {return voltage(limits_values<rep>::min());}
    inline static METRICCONSTEXPR voltage max()  {return voltage(limits_values<rep>::max());}
};


typedef voltage<long long, std::nano > nanovolt;
typedef voltage<long long, std::micro> microvolt;
typedef voltage<long long, std::milli> millivolt;
typedef voltage<long long            > volt;
typedef voltage<long long, std::kilo > kilovolt;
typedef voltage<long long, std::mega > megavolt;

namespace literals {
constexpr  nanovolt operator ""_nV(unsigned long long v) { return  nanovolt(v); }
constexpr microvolt operator ""_uV(unsigned long long v) { return microvolt(v); }
constexpr millivolt operator ""_mV(unsigned long long v) { return millivolt(v); }
constexpr      volt operator ""_V(unsigned long long v)  { return      volt(v); }
constexpr  kilovolt operator ""_kV(unsigned long long v) { return  kilovolt(v); }
constexpr  megavolt operator ""_MV(unsigned long long v) { return  megavolt(v); }
} // namespace literals

} // namespace metric

#endif // METRICS_VOLTAGE_HPP


