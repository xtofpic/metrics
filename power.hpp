// -*- C++ -*-
//
//===---------------------------- power ------------------------------------===//
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

#ifndef METRICS_POWER_HPP
#define METRICS_POWER_HPP

#include "metric_config.hpp"

namespace metric {


template <class _Rep, class _Period = std::ratio<1> > class power;

template <class _Tp> 
    struct __is_power: std::false_type {};

template <class _Rep, class _Period>
    struct __is_power<power<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_power<const power<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_power<volatile power<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_power<const volatile power<_Rep, _Period> > : std::true_type  {};


} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::power<_Rep1, _Period1>,
                    metric::power<_Rep2, _Period2> >
{
            typedef metric::power<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};

} // namespace std


namespace metric {


// power_cast

template <class _FromPower, class _ToPower,
          class _Period = typename std::ratio_divide<typename _FromPower::period, typename _ToPower::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __power_cast;

template <class _FromPower, class _ToPower, class _Period>
struct __power_cast<_FromPower, _ToPower, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToPower operator()(const _FromPower& __fd) const
    {   
        return _ToPower(static_cast<typename _ToPower::rep>(__fd.count()));
    }
};

template <class _FromPower, class _ToPower, class _Period>
struct __power_cast<_FromPower, _ToPower, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToPower operator()(const _FromPower& __fd) const
    {   
        typedef typename std::common_type<typename _ToPower::rep, typename _FromPower::rep, intmax_t>::type _Ct;
        return _ToPower(static_cast<typename _ToPower::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromPower, class _ToPower, class _Period>
struct __power_cast<_FromPower, _ToPower, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToPower operator()(const _FromPower& __fd) const
    {   
        typedef typename std::common_type<typename _ToPower::rep, typename _FromPower::rep, intmax_t>::type _Ct;
        return _ToPower(static_cast<typename _ToPower::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromPower, class _ToPower, class _Period>
struct __power_cast<_FromPower, _ToPower, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToPower operator()(const _FromPower& __fd) const
    {
        typedef typename std::common_type<typename _ToPower::rep, typename _FromPower::rep, intmax_t>::type _Ct;
        return _ToPower(static_cast<typename _ToPower::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToPower, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_power<_ToPower>::value,
    _ToPower
>::type
power_cast(const power<_Rep, _Period>& __fd)
{
    return __power_cast<power<_Rep, _Period>, _ToPower>()(__fd);
}


template <class _Rep>
struct power_values
{
public:
    inline static METRICCONSTEXPR _Rep zero() {return _Rep(0);}
    inline static METRICCONSTEXPR _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static METRICCONSTEXPR _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class power
{
    static_assert(!__is_power<_Rep>::value, "A power representation can not be a power");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of power must be a std::ratio");
    static_assert(_Period::num > 0, "power period must be positive");

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
    power() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit power(const _Rep2& __r,
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
        power(const power<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::power_cast<power>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR power  operator+() const {return *this;}
    inline METRICCONSTEXPR power  operator-() const {return power(-__rep_);}
    inline const power& operator++()      {++__rep_; return *this;}
    inline const power  operator++(int)   {return power(__rep_++);}
    inline const power& operator--()      {--__rep_; return *this;}
    inline const power  operator--(int)   {return power(__rep_--);}

    inline const power& operator+=(const power& __d) {__rep_ += __d.count(); return *this;}
    inline const power& operator-=(const power& __d) {__rep_ -= __d.count(); return *this;}

    inline const power& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const power& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const power& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const power& operator%=(const power& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR power zero() {return power(power_values<rep>::zero());}
    inline static METRICCONSTEXPR power min()  {return power(power_values<rep>::min());}
    inline static METRICCONSTEXPR power max()  {return power(power_values<rep>::max());}
};


typedef power<long long, std::nano > nanowatt;
typedef power<long long, std::micro> microwatt;
typedef power<long long, std::milli> milliwatt;
typedef power<long long            > watt;
typedef power<long long, std::kilo > kilowatt;
typedef power<long long, std::mega > megawatt;
typedef power<long long, std::giga > gigawatt;



// Power ==

template <class _LhsPower, class _RhsPower>
struct __power_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsPower& __lhs, const _RhsPower& __rhs) const
        {
            typedef typename std::common_type<_LhsPower, _RhsPower>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsPower>
struct __power_eq<_LhsPower, _LhsPower>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsPower& __lhs, const _LhsPower& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    return __power_eq<power<_Rep1, _Period1>, power<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Power !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Power <

template <class _LhsPower, class _RhsPower>
struct __power_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsPower& __lhs, const _RhsPower& __rhs) const
        {
            typedef typename std::common_type<_LhsPower, _RhsPower>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsPower>
struct __power_lt<_LhsPower, _LhsPower>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsPower& __lhs, const _LhsPower& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    return __power_lt<power<_Rep1, _Period1>, power<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Power >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Power <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Power >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Power +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<power<_Rep1, _Period1>, power<_Rep2, _Period2> >::type
operator+(const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<power<_Rep1, _Period1>, power<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Power -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<power<_Rep1, _Period1>, power<_Rep2, _Period2> >::type
operator-(const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<power<_Rep1, _Period1>, power<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Power *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    power<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const power<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef power<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    power<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const power<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// Power /

template <class _Power, class _Rep, bool = __is_power<_Rep>::value>
struct __power_divide_result
{
};

template <class _Power, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Power::rep, _Rep2>::type>::value>
struct __power_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __power_divide_imp<power<_Rep1, _Period>, _Rep2, true>
{
    typedef power<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __power_divide_result<power<_Rep1, _Period>, _Rep2, false>
    : __power_divide_imp<power<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __power_divide_result<power<_Rep1, _Period>, _Rep2>::type
operator/(const power<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef power<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<power<_Rep1, _Period1>, power<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// Power %

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __power_divide_result<power<_Rep1, _Period>, _Rep2>::type
operator%(const power<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef power<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<power<_Rep1, _Period1>, power<_Rep2, _Period2> >::type
operator%(const power<_Rep1, _Period1>& __lhs, const power<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<power<_Rep1, _Period1>, power<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_POWER_HPP


