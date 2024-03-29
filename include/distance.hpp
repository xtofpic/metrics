// -*- C++ -*-
//
//===---------------------------- distance ------------------------------------===//
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

#ifndef METRICS_DISTANCE_HPP
#define METRICS_DISTANCE_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class distance;

template <typename A> struct __is_distance: __is_specialization<A, distance> {};

template <class _ToDistance, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_distance<_ToDistance>::value,
    _ToDistance
>::type
distance_cast(const distance<_Rep, _Period>& __fd)
{
    return __metric_cast<distance<_Rep, _Period>, _ToDistance>()(__fd);
}


template <class _Rep, class _Period>
class distance
{
    static_assert(!__is_distance<_Rep>::value, "A distance representation can not be a distance");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of distance must be a std::ratio");
    static_assert(_Period::num > 0, "distance period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    distance() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit distance(const _Rep2& __r,
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
		distance(const distance<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::distance_cast<distance>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR distance  operator+() const {return *this;}
    inline METRICCONSTEXPR distance  operator-() const {return distance(-__rep_);}
    inline const distance& operator++()      {++__rep_; return *this;}
    inline const distance  operator++(int)   {return distance(__rep_++);}
    inline const distance& operator--()      {--__rep_; return *this;}
    inline const distance  operator--(int)   {return distance(__rep_--);}

    inline const distance& operator+=(const distance& __d) {__rep_ += __d.count(); return *this;}
    inline const distance& operator-=(const distance& __d) {__rep_ -= __d.count(); return *this;}

    inline const distance& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const distance& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const distance& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const distance& operator%=(const distance& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR distance zero() {return distance(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR distance min()  {return distance(limits_values<rep>::min());}
    inline static METRICCONSTEXPR distance max()  {return distance(limits_values<rep>::max());}
};


typedef distance<long long,                  std::atto > attometre;
typedef distance<long long,                  std::femto> femtometre;
typedef distance<long long,                  std::pico > picometre;
typedef distance<long long,                  std::nano > nanometre;
typedef distance<long long,                  std::micro> micrometre;
typedef distance<long long,                  std::milli> millimetre;
typedef distance<long long,                  std::centi> centimetre;
typedef distance<long long                             > metre;
typedef distance<     long,                  std::kilo > kilometre;
typedef distance<     long,                  std::mega > megametre;
typedef distance<     long,                  std::giga > gigametre;
typedef distance<     long,                  std::tera > terametre;
typedef distance<     long,                  std::peta > petametre;
typedef distance<     long,                  std::exa  > exametre;
typedef distance<long long, std::ratio<9144LL, 10000LL>> yard;
typedef distance<long long, std::ratio< 254LL, 10000LL>> inch;
typedef distance<long long, std::ratio<3048LL, 10000LL>> foot;
typedef distance<long long, std::ratio<1609LL,     1LL>> mile;

namespace literals {
constexpr    attometre operator ""_am(unsigned long long v)  { return    attometre(v); }
constexpr   femtometre operator ""_fm(unsigned long long v)  { return   femtometre(v); }
constexpr    picometre operator ""_pm(unsigned long long v)  { return    picometre(v); }
constexpr    nanometre operator ""_nm(unsigned long long v)  { return    nanometre(v); }
constexpr   micrometre operator ""_um(unsigned long long v)  { return   micrometre(v); }
constexpr   millimetre operator ""_mm(unsigned long long v)  { return   millimetre(v); }
constexpr   centimetre operator ""_cm(unsigned long long v)  { return   centimetre(v); }
constexpr        metre operator ""_m( unsigned long long v)  { return        metre(v); }
constexpr    kilometre operator ""_km(unsigned long long v)  { return    kilometre(v); }
constexpr    megametre operator ""_Mm(unsigned long long v)  { return    megametre(v); }
constexpr    gigametre operator ""_Gm(unsigned long long v)  { return    gigametre(v); }
constexpr    terametre operator ""_Tm(unsigned long long v)  { return    terametre(v); }
constexpr    petametre operator ""_Pm(unsigned long long v)  { return    petametre(v); }
constexpr     exametre operator ""_Em(unsigned long long v)  { return     exametre(v); }
constexpr         yard operator ""_yd(unsigned long long v)  { return         yard(v); }
constexpr         inch operator ""_in(unsigned long long v)  { return         inch(v); }
constexpr         mile operator ""_mi(unsigned long long v)  { return         mile(v); }
constexpr         foot operator ""_ft(unsigned long long v)  { return         foot(v); }
}

} // namespace metric

#endif // METRICS_DISTANCE_HPP
