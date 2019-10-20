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


// electricresistance_cast

template <class _FromElectricResistance, class _ToElectricResistance,
          class _Period = typename std::ratio_divide<typename _FromElectricResistance::period, typename _ToElectricResistance::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __electricresistance_cast;

template <class _FromElectricResistance, class _ToElectricResistance, class _Period>
struct __electricresistance_cast<_FromElectricResistance, _ToElectricResistance, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToElectricResistance operator()(const _FromElectricResistance& __fd) const
    {   
        return _ToElectricResistance(static_cast<typename _ToElectricResistance::rep>(__fd.count()));
    }
};

template <class _FromElectricResistance, class _ToElectricResistance, class _Period>
struct __electricresistance_cast<_FromElectricResistance, _ToElectricResistance, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToElectricResistance operator()(const _FromElectricResistance& __fd) const
    {   
        typedef typename std::common_type<typename _ToElectricResistance::rep, typename _FromElectricResistance::rep, intmax_t>::type _Ct;
        return _ToElectricResistance(static_cast<typename _ToElectricResistance::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromElectricResistance, class _ToElectricResistance, class _Period>
struct __electricresistance_cast<_FromElectricResistance, _ToElectricResistance, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToElectricResistance operator()(const _FromElectricResistance& __fd) const
    {   
        typedef typename std::common_type<typename _ToElectricResistance::rep, typename _FromElectricResistance::rep, intmax_t>::type _Ct;
        return _ToElectricResistance(static_cast<typename _ToElectricResistance::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromElectricResistance, class _ToElectricResistance, class _Period>
struct __electricresistance_cast<_FromElectricResistance, _ToElectricResistance, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToElectricResistance operator()(const _FromElectricResistance& __fd) const
    {
        typedef typename std::common_type<typename _ToElectricResistance::rep, typename _FromElectricResistance::rep, intmax_t>::type _Ct;
        return _ToElectricResistance(static_cast<typename _ToElectricResistance::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


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
    return __electricresistance_cast<electricresistance<_Rep, _Period>, _ToElectricResistance>()(__fd);
}


template <class _Rep, class _Period>
class electricresistance
{
    static_assert(!__is_electricresistance<_Rep>::value, "A electricresistance representation can not be a electricresistance");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of electricresistance must be a std::ratio");
    static_assert(_Period::num > 0, "electricresistance period must be positive");

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


// ElectricResistance ==

template <class _LhsElectricResistance, class _RhsElectricResistance>
struct __electricresistance_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsElectricResistance& __lhs, const _RhsElectricResistance& __rhs) const
        {
            typedef typename std::common_type<_LhsElectricResistance, _RhsElectricResistance>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsElectricResistance>
struct __electricresistance_eq<_LhsElectricResistance, _LhsElectricResistance>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsElectricResistance& __lhs, const _LhsElectricResistance& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    return __electricresistance_eq<electricresistance<_Rep1, _Period1>, electricresistance<_Rep2, _Period2> >()(__lhs, __rhs);
}

// ElectricResistance !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// ElectricResistance <

template <class _LhsElectricResistance, class _RhsElectricResistance>
struct __electricresistance_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsElectricResistance& __lhs, const _RhsElectricResistance& __rhs) const
        {
            typedef typename std::common_type<_LhsElectricResistance, _RhsElectricResistance>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsElectricResistance>
struct __electricresistance_lt<_LhsElectricResistance, _LhsElectricResistance>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsElectricResistance& __lhs, const _LhsElectricResistance& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    return __electricresistance_lt<electricresistance<_Rep1, _Period1>, electricresistance<_Rep2, _Period2> >()(__lhs, __rhs);
}

// ElectricResistance >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// ElectricResistance <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// ElectricResistance >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// ElectricResistance +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<electricresistance<_Rep1, _Period1>, electricresistance<_Rep2, _Period2> >::type
operator+(const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<electricresistance<_Rep1, _Period1>, electricresistance<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// ElectricResistance -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<electricresistance<_Rep1, _Period1>, electricresistance<_Rep2, _Period2> >::type
operator-(const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<electricresistance<_Rep1, _Period1>, electricresistance<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// ElectricResistance *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    electricresistance<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const electricresistance<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef electricresistance<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    electricresistance<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const electricresistance<_Rep2, _Period>& __d)
{
    return __d * __s;
}

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

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<electricresistance<_Rep1, _Period1>, electricresistance<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
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

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<electricresistance<_Rep1, _Period1>, electricresistance<_Rep2, _Period2> >::type
operator%(const electricresistance<_Rep1, _Period1>& __lhs, const electricresistance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<electricresistance<_Rep1, _Period1>, electricresistance<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_ELECTRICRESISTANCE_HPP


