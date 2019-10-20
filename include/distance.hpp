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

} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::distance<_Rep1, _Period1>,
                    metric::distance<_Rep2, _Period2> >
{
            typedef metric::distance<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};

} // namespace std


namespace metric {


// distance_cast

template <class _FromDistance, class _ToDistance,
          class _Period = typename std::ratio_divide<typename _FromDistance::period, typename _ToDistance::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __distance_cast;

template <class _FromDistance, class _ToDistance, class _Period>
struct __distance_cast<_FromDistance, _ToDistance, _Period, true, true>
{
    inline METRICCONSTEXPR
    _ToDistance operator()(const _FromDistance& __fd) const
    {
        return _ToDistance(static_cast<typename _ToDistance::rep>(__fd.count()));
    }
};

template <class _FromDistance, class _ToDistance, class _Period>
struct __distance_cast<_FromDistance, _ToDistance, _Period, true, false>
{
    inline METRICCONSTEXPR
    _ToDistance operator()(const _FromDistance& __fd) const
    {
        typedef typename std::common_type<typename _ToDistance::rep, typename _FromDistance::rep, intmax_t>::type _Ct;
        return _ToDistance(static_cast<typename _ToDistance::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromDistance, class _ToDistance, class _Period>
struct __distance_cast<_FromDistance, _ToDistance, _Period, false, true>
{
    inline METRICCONSTEXPR
    _ToDistance operator()(const _FromDistance& __fd) const
    {
        typedef typename std::common_type<typename _ToDistance::rep, typename _FromDistance::rep, intmax_t>::type _Ct;
        return _ToDistance(static_cast<typename _ToDistance::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromDistance, class _ToDistance, class _Period>
struct __distance_cast<_FromDistance, _ToDistance, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToDistance operator()(const _FromDistance& __fd) const
    {
        typedef typename std::common_type<typename _ToDistance::rep, typename _FromDistance::rep, intmax_t>::type _Ct;
        return _ToDistance(static_cast<typename _ToDistance::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


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
    return __distance_cast<distance<_Rep, _Period>, _ToDistance>()(__fd);
}

template <class _Rep, class _Period>
class distance
{
    static_assert(!__is_distance<_Rep>::value, "A distance representation can not be a distance");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of distance must be a std::ratio");
    static_assert(_Period::num > 0, "distance period must be positive");

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


// typedef distance<long long, metric::yocto> yoctometre;
// typedef distance<long long, metric::zepto> zeptometre;
typedef distance<long long,    std::atto > attometre;
typedef distance<long long,    std::femto> femtometre;
typedef distance<long long,    std::pico > picometre;
typedef distance<long long,    std::nano > nanometre;
typedef distance<long long,    std::micro> micrometre;
typedef distance<long long,    std::milli> millimetre;
typedef distance<long long,    std::centi> centimetre;
typedef distance<long long               > metre;
typedef distance<     long,    std::kilo > kilometre;
typedef distance<     long,    std::mega > megametre;
typedef distance<     long,    std::giga > gigametre;
typedef distance<     long,    std::tera > terametre;
typedef distance<     long,    std::peta > petametre;
typedef distance<     long,    std::exa  > exametre;
// typedef distance<     long, metric::zetta> zettametre;

typedef distance<long long, std::ratio<9144LL, 10000LL>> yard;			// yd
typedef distance<long long, std::ratio< 254LL, 10000LL>> inch;			// in
typedef distance<long long, std::ratio<1852LL,     1LL>> nauticalmile;	// nmi
typedef distance<long long, std::ratio<3048LL, 10000LL>> foot;			// ft



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
constexpr nauticalmile operator ""_nmi(unsigned long long v) { return nauticalmile(v); }
constexpr         foot operator ""_ft(unsigned long long v)  { return         foot(v); }

} // namespace literals

// Distance ==

template <class _LhsDistance, class _RhsDistance>
struct __distance_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsDistance& __lhs, const _RhsDistance& __rhs) const
        {
            typedef typename std::common_type<_LhsDistance, _RhsDistance>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsDistance>
struct __distance_eq<_LhsDistance, _LhsDistance>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsDistance& __lhs, const _LhsDistance& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return __distance_eq<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Distance !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Distance <

template <class _LhsDistance, class _RhsDistance>
struct __distance_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsDistance& __lhs, const _RhsDistance& __rhs) const
        {
            typedef typename std::common_type<_LhsDistance, _RhsDistance>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsDistance>
struct __distance_lt<_LhsDistance, _LhsDistance>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsDistance& __lhs, const _LhsDistance& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return __distance_lt<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Distance >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Distance <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Distance >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Distance +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type
operator+(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Distance -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type
operator-(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Distance *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    distance<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const distance<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef distance<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    distance<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const distance<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// Distance /

template <class _Distance, class _Rep, bool = __is_distance<_Rep>::value>
struct __distance_divide_result
{
};

template <class _Distance, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Distance::rep, _Rep2>::type>::value>
struct __distance_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __distance_divide_imp<distance<_Rep1, _Period>, _Rep2, true>
{
    typedef distance<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __distance_divide_result<distance<_Rep1, _Period>, _Rep2, false>
    : __distance_divide_imp<distance<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __distance_divide_result<distance<_Rep1, _Period>, _Rep2>::type
operator/(const distance<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef distance<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// Distance %

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __distance_divide_result<distance<_Rep1, _Period>, _Rep2>::type
operator%(const distance<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef distance<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type
operator%(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_DISTANCE_HPP


