// -*- C++ -*-
//
//===---------------------------- energy ------------------------------------===//
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

#ifndef METRICS_ENERGY_HPP
#define METRICS_ENERGY_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class energy;

template <typename A> struct __is_energy: __is_specialization<A, energy> {};


// energy_cast

template <class _FromEnergy, class _ToEnergy,
          class _Period = typename std::ratio_divide<typename _FromEnergy::period, typename _ToEnergy::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __energy_cast;

template <class _FromEnergy, class _ToEnergy, class _Period>
struct __energy_cast<_FromEnergy, _ToEnergy, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToEnergy operator()(const _FromEnergy& __fd) const
    {   
        return _ToEnergy(static_cast<typename _ToEnergy::rep>(__fd.count()));
    }
};

template <class _FromEnergy, class _ToEnergy, class _Period>
struct __energy_cast<_FromEnergy, _ToEnergy, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToEnergy operator()(const _FromEnergy& __fd) const
    {   
        typedef typename std::common_type<typename _ToEnergy::rep, typename _FromEnergy::rep, intmax_t>::type _Ct;
        return _ToEnergy(static_cast<typename _ToEnergy::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromEnergy, class _ToEnergy, class _Period>
struct __energy_cast<_FromEnergy, _ToEnergy, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToEnergy operator()(const _FromEnergy& __fd) const
    {   
        typedef typename std::common_type<typename _ToEnergy::rep, typename _FromEnergy::rep, intmax_t>::type _Ct;
        return _ToEnergy(static_cast<typename _ToEnergy::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromEnergy, class _ToEnergy, class _Period>
struct __energy_cast<_FromEnergy, _ToEnergy, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToEnergy operator()(const _FromEnergy& __fd) const
    {
        typedef typename std::common_type<typename _ToEnergy::rep, typename _FromEnergy::rep, intmax_t>::type _Ct;
        return _ToEnergy(static_cast<typename _ToEnergy::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToEnergy, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_energy<_ToEnergy>::value,
    _ToEnergy
>::type
energy_cast(const energy<_Rep, _Period>& __fd)
{
    return __energy_cast<energy<_Rep, _Period>, _ToEnergy>()(__fd);
}

template <class _Rep, class _Period>
class energy
{
    static_assert(!__is_energy<_Rep>::value, "A energy representation can not be a energy");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of energy must be a std::ratio");
    static_assert(_Period::num > 0, "energy period must be positive");

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
    energy() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit energy(const _Rep2& __r,
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
        energy(const energy<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::energy_cast<energy>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR energy  operator+() const {return *this;}
    inline METRICCONSTEXPR energy  operator-() const {return energy(-__rep_);}
    inline const energy& operator++()      {++__rep_; return *this;}
    inline const energy  operator++(int)   {return energy(__rep_++);}
    inline const energy& operator--()      {--__rep_; return *this;}
    inline const energy  operator--(int)   {return energy(__rep_--);}

    inline const energy& operator+=(const energy& __d) {__rep_ += __d.count(); return *this;}
    inline const energy& operator-=(const energy& __d) {__rep_ -= __d.count(); return *this;}

    inline const energy& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const energy& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const energy& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const energy& operator%=(const energy& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR energy zero() {return energy(limits_values<rep>::zero());}
    inline static METRICCONSTEXPR energy min()  {return energy(limits_values<rep>::min());}
    inline static METRICCONSTEXPR energy max()  {return energy(limits_values<rep>::max());}
};


typedef energy<long long, std::ratio<                  1LL, 10000LL>> microwatthour;
typedef energy<long long, std::ratio<                  1LL,    10LL>> milliwatthour;
typedef energy<long long, std::ratio<               1000LL,    10LL>> watthour;
typedef energy<long long, std::ratio<            1000000LL,    10LL>> kilowatthour;
typedef energy<long long, std::ratio<         1000000000LL,    10LL>> megawatthour;
typedef energy<long long, std::ratio<      1000000000000LL,    10LL>> gigawatthour;
typedef energy<long long, std::ratio<   1000000000000000LL,    10LL>> terawatthour;
typedef energy<long long, std::ratio<1000000000000000000LL,    10LL>> petawatthour;
typedef energy<long long, std::ratio<               1000LL, 36000LL>> joule;
typedef energy<long long, std::ratio<               1000LL,  8598LL>> calorie;


namespace literals {

constexpr microwatthour operator ""_uWh(unsigned long long v) { return microwatthour(v); }
constexpr milliwatthour operator ""_mWh(unsigned long long v) { return milliwatthour(v); }
constexpr      watthour operator ""_Wh( unsigned long long v) { return      watthour(v); }
constexpr  kilowatthour operator ""_kWh(unsigned long long v) { return  kilowatthour(v); }
constexpr  megawatthour operator ""_MWh(unsigned long long v) { return  megawatthour(v); }
constexpr  gigawatthour operator ""_GWh(unsigned long long v) { return  gigawatthour(v); }
constexpr  terawatthour operator ""_TWh(unsigned long long v) { return  terawatthour(v); }
constexpr  petawatthour operator ""_PWh(unsigned long long v) { return  petawatthour(v); }
constexpr         joule operator ""_j(  unsigned long long v) { return         joule(v); }
constexpr       calorie operator ""_c(  unsigned long long v) { return       calorie(v); }

}


// Energy ==

template <class _LhsEnergy, class _RhsEnergy>
struct __energy_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsEnergy& __lhs, const _RhsEnergy& __rhs) const
        {
            typedef typename std::common_type<_LhsEnergy, _RhsEnergy>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsEnergy>
struct __energy_eq<_LhsEnergy, _LhsEnergy>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsEnergy& __lhs, const _LhsEnergy& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    return __energy_eq<energy<_Rep1, _Period1>, energy<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Energy !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Energy <

template <class _LhsEnergy, class _RhsEnergy>
struct __energy_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsEnergy& __lhs, const _RhsEnergy& __rhs) const
        {
            typedef typename std::common_type<_LhsEnergy, _RhsEnergy>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsEnergy>
struct __energy_lt<_LhsEnergy, _LhsEnergy>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsEnergy& __lhs, const _LhsEnergy& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    return __energy_lt<energy<_Rep1, _Period1>, energy<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Energy >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Energy <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Energy >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Energy +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<energy<_Rep1, _Period1>, energy<_Rep2, _Period2> >::type
operator+(const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<energy<_Rep1, _Period1>, energy<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Energy -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<energy<_Rep1, _Period1>, energy<_Rep2, _Period2> >::type
operator-(const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<energy<_Rep1, _Period1>, energy<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Energy *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    energy<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const energy<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef energy<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    energy<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const energy<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// Energy /

template <class _Energy, class _Rep, bool = __is_energy<_Rep>::value>
struct __energy_divide_result
{
};

template <class _Energy, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Energy::rep, _Rep2>::type>::value>
struct __energy_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __energy_divide_imp<energy<_Rep1, _Period>, _Rep2, true>
{
    typedef energy<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __energy_divide_result<energy<_Rep1, _Period>, _Rep2, false>
    : __energy_divide_imp<energy<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __energy_divide_result<energy<_Rep1, _Period>, _Rep2>::type
operator/(const energy<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef energy<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<energy<_Rep1, _Period1>, energy<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// Energy %

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __energy_divide_result<energy<_Rep1, _Period>, _Rep2>::type
operator%(const energy<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef energy<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<energy<_Rep1, _Period1>, energy<_Rep2, _Period2> >::type
operator%(const energy<_Rep1, _Period1>& __lhs, const energy<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<energy<_Rep1, _Period1>, energy<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_ENERGY_HPP


