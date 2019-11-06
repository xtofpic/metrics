// -*- C++ -*-
//
//===---------------------------- angularspeed ------------------------------------===//
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

#ifndef METRICS_ANGULARSPEED_HPP
#define METRICS_ANGULARSPEED_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class angularspeed;

template <typename A> struct __is_angularspeed: __is_specialization<A, angularspeed> {};


template <class _ToAngularSpeed, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_angularspeed<_ToAngularSpeed>::value,
    _ToAngularSpeed
>::type
angularspeed_cast(const angularspeed<_Rep, _Period>& __fd)
{
    return __metric_cast<angularspeed<_Rep, _Period>, _ToAngularSpeed>()(__fd);
}

template <class _Rep, class _Period>
class angularspeed
{
    static_assert(!__is_angularspeed<_Rep>::value, "A angularspeed representation can not be a angularspeed");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of angularspeed must be a std::ratio");
    static_assert(_Period::num > 0, "angularspeed period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    angularspeed() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit angularspeed(const _Rep2& __r,
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
		angularspeed(const angularspeed<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::angularspeed_cast<angularspeed>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR angularspeed  operator+() const {return *this;}
    inline METRICCONSTEXPR angularspeed  operator-() const {return angularspeed(-__rep_);}
    inline const angularspeed& operator++()      {++__rep_; return *this;}
    inline const angularspeed  operator++(int)   {return angularspeed(__rep_++);}
    inline const angularspeed& operator--()      {--__rep_; return *this;}
    inline const angularspeed  operator--(int)   {return angularspeed(__rep_--);}

    inline const angularspeed& operator+=(const angularspeed& __d) {__rep_ += __d.count(); return *this;}
    inline const angularspeed& operator-=(const angularspeed& __d) {__rep_ -= __d.count(); return *this;}

    inline const angularspeed& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const angularspeed& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const angularspeed& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const angularspeed& operator%=(const angularspeed& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR angularspeed zero() {return angularspeed(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR angularspeed min()  {return angularspeed(limits_values<rep>::min());}
    inline static METRICCONSTEXPR angularspeed max()  {return angularspeed(limits_values<rep>::max());}
};


typedef angularspeed<long long, std::ratio<  10, 1> > degree_second;
typedef angularspeed<long long, std::ratio<3600, 1> > turn_second;
typedef angularspeed<long long, std::ratio<  60, 1> > turn_minute;
typedef angularspeed<long long                      > turn_hour;

namespace literals {
constexpr degree_second operator ""_degsec(unsigned long long v) { return degree_second(v); }
constexpr   turn_second operator ""_rps(unsigned long long v)    { return turn_second(v); }
constexpr   turn_minute operator ""_rpm(unsigned long long v)    { return turn_minute(v); }
constexpr     turn_hour operator ""_rph(unsigned long long v)    { return turn_hour(v); }
}


} // namespace metric

#endif // METRICS_ANGULARSPEED_HPP


