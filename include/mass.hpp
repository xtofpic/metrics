// -*- C++ -*-
//
//===---------------------------- mass ------------------------------------===//
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

#ifndef METRICS_MASS_HPP
#define METRICS_MASS_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class mass;

template <typename A> struct __is_mass: __is_specialization<A, mass> {};


template <class _ToMass, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_mass<_ToMass>::value,
    _ToMass
>::type
mass_cast(const mass<_Rep, _Period>& __fd)
{
    return __metric_cast<mass<_Rep, _Period>, _ToMass>()(__fd);
}

template <class _Rep, class _Period>
class mass
{
    static_assert(!__is_mass<_Rep>::value, "A mass representation can not be a mass");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of mass must be a std::ratio");
    static_assert(_Period::num > 0, "mass period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    mass() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit mass(const _Rep2& __r,
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
		mass(const mass<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::mass_cast<mass>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR mass  operator+() const {return *this;}
    inline METRICCONSTEXPR mass  operator-() const {return mass(-__rep_);}
    inline const mass& operator++()      {++__rep_; return *this;}
    inline const mass  operator++(int)   {return mass(__rep_++);}
    inline const mass& operator--()      {--__rep_; return *this;}
    inline const mass  operator--(int)   {return mass(__rep_--);}

    inline const mass& operator+=(const mass& __d) {__rep_ += __d.count(); return *this;}
    inline const mass& operator-=(const mass& __d) {__rep_ -= __d.count(); return *this;}

    inline const mass& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const mass& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const mass& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const mass& operator%=(const mass& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR mass zero() {return mass(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR mass min()  {return mass(limits_values<rep>::min());}
    inline static METRICCONSTEXPR mass max()  {return mass(limits_values<rep>::max());}
};


// Mass /
template <class _Mass, class _Rep, bool = __is_mass<_Rep>::value>
struct __mass_divide_result
{
};

template <class _Mass, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Mass::rep, _Rep2>::type>::value>
struct __mass_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __mass_divide_imp<mass<_Rep1, _Period>, _Rep2, true>
{
    typedef mass<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __mass_divide_result<mass<_Rep1, _Period>, _Rep2, false>
    : __mass_divide_imp<mass<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2, typename std::enable_if<std::is_arithmetic<_Rep2>::value, int>::type = 0>
inline
METRICCONSTEXPR
typename __mass_divide_result<mass<_Rep1, _Period>, _Rep2>::type
operator/(const mass<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef mass<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}


// Mass %
template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __mass_divide_result<mass<_Rep1, _Period>, _Rep2>::type
operator%(const mass<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef mass<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}


typedef mass<long long, std::nano > nanogram;
typedef mass<long long, std::micro> microgram;
typedef mass<long long, std::milli> milligram;
typedef mass<long long            > gram;
typedef mass<     long, std::kilo > kilogram;
typedef mass<     long, std::mega > ton;

namespace literals {
constexpr  nanogram operator ""_ng(unsigned long long v)  { return  nanogram(v); }
constexpr microgram operator ""_ug(unsigned long long v)  { return microgram(v); }
constexpr milligram operator ""_mg(unsigned long long v)  { return milligram(v); }
constexpr      gram operator ""_g(unsigned long long v)   { return      gram(v); }
constexpr  kilogram operator ""_kg(unsigned long long v)  { return  kilogram(v); }
constexpr      ton  operator ""_ton(unsigned long long v) { return      ton(v);  }
} // namespace literals

} // namespace metric

#endif // METRICS_MASS_HPP


