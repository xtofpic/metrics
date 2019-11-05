// -*- C++ -*-
//
//===---------------------------- electriccurrent ------------------------------------===//
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

#ifndef METRICS_ELECTRICCURRENT_HPP
#define METRICS_ELECTRICCURRENT_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class electriccurrent;

template <typename A> struct __is_electriccurrent: __is_specialization<A, electriccurrent> {};


template <class _ToElectricCurrent, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_electriccurrent<_ToElectricCurrent>::value,
    _ToElectricCurrent
>::type
electriccurrent_cast(const electriccurrent<_Rep, _Period>& __fd)
{
    return __metric_cast<electriccurrent<_Rep, _Period>, _ToElectricCurrent>()(__fd);
}

template <class _Rep, class _Period>
class electriccurrent
{
    static_assert(!__is_electriccurrent<_Rep>::value, "A electriccurrent representation can not be a electriccurrent");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of electriccurrent must be a std::ratio");
    static_assert(_Period::num > 0, "electriccurrent period must be positive");

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline METRICCONSTEXPR
    electriccurrent() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit electriccurrent(const _Rep2& __r,
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
		electriccurrent(const electriccurrent<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::electriccurrent_cast<electriccurrent>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR electriccurrent  operator+() const {return *this;}
    inline METRICCONSTEXPR electriccurrent  operator-() const {return electriccurrent(-__rep_);}
    inline const electriccurrent& operator++()      {++__rep_; return *this;}
    inline const electriccurrent  operator++(int)   {return electriccurrent(__rep_++);}
    inline const electriccurrent& operator--()      {--__rep_; return *this;}
    inline const electriccurrent  operator--(int)   {return electriccurrent(__rep_--);}

    inline const electriccurrent& operator+=(const electriccurrent& __d) {__rep_ += __d.count(); return *this;}
    inline const electriccurrent& operator-=(const electriccurrent& __d) {__rep_ -= __d.count(); return *this;}

    inline const electriccurrent& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const electriccurrent& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const electriccurrent& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const electriccurrent& operator%=(const electriccurrent& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR electriccurrent zero() {return electriccurrent(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR electriccurrent min()  {return electriccurrent(limits_values<rep>::min());}
    inline static METRICCONSTEXPR electriccurrent max()  {return electriccurrent(limits_values<rep>::max());}
};


// ElectricCurrent /
template <class _ElectricCurrent, class _Rep, bool = __is_electriccurrent<_Rep>::value>
struct __electriccurrent_divide_result
{
};

template <class _ElectricCurrent, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _ElectricCurrent::rep, _Rep2>::type>::value>
struct __electriccurrent_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __electriccurrent_divide_imp<electriccurrent<_Rep1, _Period>, _Rep2, true>
{
    typedef electriccurrent<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __electriccurrent_divide_result<electriccurrent<_Rep1, _Period>, _Rep2, false>
    : __electriccurrent_divide_imp<electriccurrent<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __electriccurrent_divide_result<electriccurrent<_Rep1, _Period>, _Rep2>::type
operator/(const electriccurrent<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef electriccurrent<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}


// ElectricCurrent %
template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __electriccurrent_divide_result<electriccurrent<_Rep1, _Period>, _Rep2>::type
operator%(const electriccurrent<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef electriccurrent<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

typedef electriccurrent<long long, std::femto> femtoampere;
typedef electriccurrent<long long, std::pico > picoampere;
typedef electriccurrent<long long, std::nano > nanoampere;
typedef electriccurrent<long long, std::micro> microampere;
typedef electriccurrent<long long, std::milli> milliampere;
typedef electriccurrent<long long            > ampere;
typedef electriccurrent<     long, std::kilo > kiloampere;
typedef electriccurrent<     long, std::mega > megaampere;

namespace literals {
constexpr femtoampere operator ""_fA(unsigned long long v) { return femtoampere(v); }
constexpr picoampere  operator ""_pA(unsigned long long v) { return picoampere(v);  }
constexpr nanoampere  operator ""_nA(unsigned long long v) { return nanoampere(v);  }
constexpr microampere operator ""_uA(unsigned long long v) { return microampere(v); }
constexpr milliampere operator ""_mA(unsigned long long v) { return milliampere(v); }
constexpr ampere      operator ""_A( unsigned long long v) { return ampere(v);      }
constexpr kiloampere  operator ""_kA(unsigned long long v) { return kiloampere(v);  }
constexpr megaampere  operator ""_MA(unsigned long long v) { return megaampere(v);  }
} // literals

} // namespace metric

#endif // METRICS_ELECTRICCURRENT_HPP


