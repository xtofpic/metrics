// -*- C++ -*-
//
//===---------------------------- volume ------------------------------------===//
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

#ifndef METRICS_VOLUME_HPP
#define METRICS_VOLUME_HPP

#include "metric_config.hpp"
#include <type_traits>

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class volume;

template <typename A> struct __is_volume: __is_specialization<A, volume> {};


template <class _ToVolume, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_volume<_ToVolume>::value,
    _ToVolume
>::type
volume_cast(const volume<_Rep, _Period>& __fd)
{
    return __metric_cast<volume<_Rep, _Period>, _ToVolume>()(__fd);
}

template <class _Rep, class _Period>
class volume
{
    static_assert(!__is_volume<_Rep>::value, "A volume representation can not be a volume");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of volume must be a std::ratio");
    static_assert(_Period::num > 0, "volume period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    volume() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit volume(const _Rep2& __r,
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
		volume(const volume<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::volume_cast<volume>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR volume  operator+() const {return *this;}
    inline METRICCONSTEXPR volume  operator-() const {return volume(-__rep_);}
    inline const volume& operator++()      {++__rep_; return *this;}
    inline const volume  operator++(int)   {return volume(__rep_++);}
    inline const volume& operator--()      {--__rep_; return *this;}
    inline const volume  operator--(int)   {return volume(__rep_--);}

    inline const volume& operator+=(const volume& __d) {__rep_ += __d.count(); return *this;}
    inline const volume& operator-=(const volume& __d) {__rep_ -= __d.count(); return *this;}

    inline const volume& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const volume& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const volume& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const volume& operator%=(const volume& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR volume zero() {return volume(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR volume min()  {return volume(limits_values<rep>::min());}
    inline static METRICCONSTEXPR volume max()  {return volume(limits_values<rep>::max());}
};


typedef volume<long long, std::nano > nanolitre;
typedef volume<long long, std::micro> microlitre;
typedef volume<long long, std::milli> millilitre;
typedef volume<long long            > litre;
typedef volume<long long, std::kilo > kilolitre;
typedef volume<long long, std::mega > megalitre;

namespace literals {
constexpr   nanolitre operator ""_nl(unsigned long long v) { return   nanolitre(v); }
constexpr  microlitre operator ""_ul(unsigned long long v) { return  microlitre(v); }
constexpr  millilitre operator ""_ml(unsigned long long v) { return  millilitre(v); }
constexpr       litre operator ""_l(unsigned long long v)  { return       litre(v); }
constexpr   kilolitre operator ""_kl(unsigned long long v) { return   kilolitre(v); }
constexpr   megalitre operator ""_Ml(unsigned long long v) { return   megalitre(v); }
} // namespace literals


} // namespace metric

#endif // METRICS_VOLUME_HPP


