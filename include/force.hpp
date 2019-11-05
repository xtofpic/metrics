// -*- C++ -*-
//
//===---------------------------- force ------------------------------------===//
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

#ifndef METRICS_FORCE_HPP
#define METRICS_FORCE_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class force;

template <typename A> struct __is_force: __is_specialization<A, force> {};


template <class _ToForce, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_force<_ToForce>::value,
    _ToForce
>::type
force_cast(const force<_Rep, _Period>& __fd)
{
    return __metric_cast<force<_Rep, _Period>, _ToForce>()(__fd);
}

template <class _Rep, class _Period>
class force
{
    static_assert(!__is_force<_Rep>::value, "A force representation can not be a force");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of force must be a std::ratio");
    static_assert(_Period::num > 0, "force period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    force() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit force(const _Rep2& __r,
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
		force(const force<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::force_cast<force>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR force  operator+() const {return *this;}
    inline METRICCONSTEXPR force  operator-() const {return force(-__rep_);}
    inline const force& operator++()      {++__rep_; return *this;}
    inline const force  operator++(int)   {return force(__rep_++);}
    inline const force& operator--()      {--__rep_; return *this;}
    inline const force  operator--(int)   {return force(__rep_--);}

    inline const force& operator+=(const force& __d) {__rep_ += __d.count(); return *this;}
    inline const force& operator-=(const force& __d) {__rep_ -= __d.count(); return *this;}

    inline const force& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const force& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const force& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const force& operator%=(const force& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR force zero() {return force(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR force min()  {return force(limits_values<rep>::min());}
    inline static METRICCONSTEXPR force max()  {return force(limits_values<rep>::max());}
};


// Force /
template <class _Force, class _Rep, bool = __is_force<_Rep>::value>
struct __force_divide_result
{
};

template <class _Force, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Force::rep, _Rep2>::type>::value>
struct __force_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __force_divide_imp<force<_Rep1, _Period>, _Rep2, true>
{
    typedef force<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __force_divide_result<force<_Rep1, _Period>, _Rep2, false>
    : __force_divide_imp<force<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __force_divide_result<force<_Rep1, _Period>, _Rep2>::type
operator/(const force<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef force<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}


// Force %
template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __force_divide_result<force<_Rep1, _Period>, _Rep2>::type
operator%(const force<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef force<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

typedef force<long long, std::milli                    > millinewton;
typedef force<long long                                > newton;
typedef force<long long, std::deca                     > decanewton;
typedef force<long long, std::ratio<980665, 100000000> > gramforce;
typedef force<long long, std::ratio<980665,    100000> > kilogramforce;

namespace literals {
constexpr   millinewton operator ""_mN(unsigned long long v)  { return millinewton(v); }
constexpr        newton operator ""_N(unsigned long long v)   { return newton(v); }
constexpr    decanewton operator ""_dN(unsigned long long v)  { return decanewton(v); }
constexpr     gramforce operator ""_gf(unsigned long long v)  { return gramforce(v); }
constexpr kilogramforce operator ""_kgf(unsigned long long v) { return kilogramforce(v); }
}

} // namespace metric

#endif // METRICS_FORCE_HPP


