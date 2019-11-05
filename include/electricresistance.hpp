// -*- C++ -*-
//
//===---------------------------- electricresistance ------------------------------------===//
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

#ifndef METRICS_ELECTRICRESISTANCE_HPP
#define METRICS_ELECTRICRESISTANCE_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class electricresistance;

template <typename A> struct __is_electricresistance: __is_specialization<A, electricresistance> {};


template <class _ToElectricResistance, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_electricresistance<_ToElectricResistance>::value,
    _ToElectricResistance
>::type
electricresistance_cast(const electricresistance<_Rep, _Period>& __fd)
{
    return __metric_cast<electricresistance<_Rep, _Period>, _ToElectricResistance>()(__fd);
}

template <class _Rep, class _Period>
class electricresistance
{
    static_assert(!__is_electricresistance<_Rep>::value, "A electricresistance representation can not be a electricresistance");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of electricresistance must be a std::ratio");
    static_assert(_Period::num > 0, "electricresistance period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    electricresistance() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit electricresistance(const _Rep2& __r,
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
		electricresistance(const electricresistance<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::electricresistance_cast<electricresistance>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR electricresistance  operator+() const {return *this;}
    inline METRICCONSTEXPR electricresistance  operator-() const {return electricresistance(-__rep_);}
    inline const electricresistance& operator++()      {++__rep_; return *this;}
    inline const electricresistance  operator++(int)   {return electricresistance(__rep_++);}
    inline const electricresistance& operator--()      {--__rep_; return *this;}
    inline const electricresistance  operator--(int)   {return electricresistance(__rep_--);}

    inline const electricresistance& operator+=(const electricresistance& __d) {__rep_ += __d.count(); return *this;}
    inline const electricresistance& operator-=(const electricresistance& __d) {__rep_ -= __d.count(); return *this;}

    inline const electricresistance& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const electricresistance& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const electricresistance& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const electricresistance& operator%=(const electricresistance& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR electricresistance zero() {return electricresistance(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR electricresistance min()  {return electricresistance(limits_values<rep>::min());}
    inline static METRICCONSTEXPR electricresistance max()  {return electricresistance(limits_values<rep>::max());}
};


// ElectricResistance /
template <class _ElectricResistance, class _Rep, bool = __is_electricresistance<_Rep>::value>
struct __electricresistance_divide_result
{
};

template <class _ElectricResistance, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _ElectricResistance::rep, _Rep2>::type>::value>
struct __electricresistance_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __electricresistance_divide_imp<electricresistance<_Rep1, _Period>, _Rep2, true>
{
    typedef electricresistance<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __electricresistance_divide_result<electricresistance<_Rep1, _Period>, _Rep2, false>
    : __electricresistance_divide_imp<electricresistance<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __electricresistance_divide_result<electricresistance<_Rep1, _Period>, _Rep2>::type
operator/(const electricresistance<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef electricresistance<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}


// ElectricResistance %
template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __electricresistance_divide_result<electricresistance<_Rep1, _Period>, _Rep2>::type
operator%(const electricresistance<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef electricresistance<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

typedef electricresistance<long long, std::nano > abohm;
typedef electricresistance<long long, std::micro> microohm;
typedef electricresistance<long long, std::milli> milliohm;
typedef electricresistance<long long            > ohm;
typedef electricresistance<     long, std::kilo > kiloohm;
typedef electricresistance<     long, std::mega > megaohm;
typedef electricresistance<     long, std::giga > gigaohm;

namespace literals {
constexpr    abohm operator ""_ao(unsigned long long v) { return    abohm(v); }
constexpr microohm operator ""_uo(unsigned long long v) { return microohm(v); }
constexpr milliohm operator ""_mo(unsigned long long v) { return milliohm(v); }
constexpr      ohm operator ""_o( unsigned long long v) { return      ohm(v); }
constexpr  kiloohm operator ""_ko(unsigned long long v) { return  kiloohm(v); }
constexpr  megaohm operator ""_Mo(unsigned long long v) { return  megaohm(v); }
constexpr  gigaohm operator ""_Go(unsigned long long v) { return  gigaohm(v); }
} // namespace literals

} // namespace metric

#endif // METRICS_ELECTRICRESISTANCE_HPP


