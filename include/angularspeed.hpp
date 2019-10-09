// -*- C++ -*-
//
//===---------------------------- angularspeed ------------------------------------===//
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

#ifndef METRICS_ANGULARSPEED_HPP
#define METRICS_ANGULARSPEED_HPP

#include "metric_config.hpp"

namespace metric {


template <class _Rep, class _Period = std::ratio<1> > class angularspeed;

template <class _Tp> 
    struct __is_angularspeed: std::false_type {};

template <class _Rep, class _Period>
    struct __is_angularspeed<angularspeed<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_angularspeed<const angularspeed<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_angularspeed<volatile angularspeed<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_angularspeed<const volatile angularspeed<_Rep, _Period> > : std::true_type  {};


} // namespace metric


namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::angularspeed<_Rep1, _Period1>,
                    metric::angularspeed<_Rep2, _Period2> >
{
            typedef metric::angularspeed<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};

} // namespace std


namespace metric {


// angularspeed_cast

template <class _FromAngularSpeed, class _ToAngularSpeed,
          class _Period = typename std::ratio_divide<typename _FromAngularSpeed::period, typename _ToAngularSpeed::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __angularspeed_cast;

template <class _FromAngularSpeed, class _ToAngularSpeed, class _Period>
struct __angularspeed_cast<_FromAngularSpeed, _ToAngularSpeed, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToAngularSpeed operator()(const _FromAngularSpeed& __fd) const
    {   
        return _ToAngularSpeed(static_cast<typename _ToAngularSpeed::rep>(__fd.count()));
    }
};

template <class _FromAngularSpeed, class _ToAngularSpeed, class _Period>
struct __angularspeed_cast<_FromAngularSpeed, _ToAngularSpeed, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToAngularSpeed operator()(const _FromAngularSpeed& __fd) const
    {   
        typedef typename std::common_type<typename _ToAngularSpeed::rep, typename _FromAngularSpeed::rep, intmax_t>::type _Ct;
        return _ToAngularSpeed(static_cast<typename _ToAngularSpeed::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromAngularSpeed, class _ToAngularSpeed, class _Period>
struct __angularspeed_cast<_FromAngularSpeed, _ToAngularSpeed, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToAngularSpeed operator()(const _FromAngularSpeed& __fd) const
    {   
        typedef typename std::common_type<typename _ToAngularSpeed::rep, typename _FromAngularSpeed::rep, intmax_t>::type _Ct;
        return _ToAngularSpeed(static_cast<typename _ToAngularSpeed::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromAngularSpeed, class _ToAngularSpeed, class _Period>
struct __angularspeed_cast<_FromAngularSpeed, _ToAngularSpeed, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToAngularSpeed operator()(const _FromAngularSpeed& __fd) const
    {
        typedef typename std::common_type<typename _ToAngularSpeed::rep, typename _FromAngularSpeed::rep, intmax_t>::type _Ct;
        return _ToAngularSpeed(static_cast<typename _ToAngularSpeed::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToAngularSpeed, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_angularspeed<_ToAngularSpeed>::value,
    _ToAngularSpeed
>::type
angularspeed_cast(const angularspeed<_Rep, _Period>& __fd)
{
    return __angularspeed_cast<angularspeed<_Rep, _Period>, _ToAngularSpeed>()(__fd);
}


template <class _Rep>
struct angularspeed_values
{
public:
    inline static METRICCONSTEXPR _Rep zero() {return _Rep(0);}
    inline static METRICCONSTEXPR _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static METRICCONSTEXPR _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class angularspeed
{
    static_assert(!__is_angularspeed<_Rep>::value, "A angularspeed representation can not be a angularspeed");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of angularspeed must be a std::ratio");
    static_assert(_Period::num > 0, "angularspeed period must be positive");

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
    angularspeed() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit angularspeed(const _Rep2& __r,
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
        angularspeed(const angularspeed<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::angularspeed_cast<angularspeed>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR angularspeed  operator+() const {return *this;}
    inline METRICCONSTEXPR angularspeed  operator-() const {return angularspeed(-__rep_);}
    inline const angularspeed& operator++()      {++__rep_; return *this;}
    inline const angularspeed  operator++(int)   {return angularspeed(__rep_++);}
    inline const angularspeed& operator--()      {--__rep_; return *this;}
    inline const angularspeed  operator--(int)   {return angularspeed(__rep_--);}

    inline const angularspeed& operator+=(const angularspeed& __d) {__rep_ += __d.count(); return *this;}
    inline const angularspeed& operator-=(const angularspeed& __d) {__rep_ -= __d.count(); return *this;}

    inline const angularspeed& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const angularspeed& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const angularspeed& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const angularspeed& operator%=(const angularspeed& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR angularspeed zero() {return angularspeed(angularspeed_values<rep>::zero());}
    inline static METRICCONSTEXPR angularspeed min()  {return angularspeed(angularspeed_values<rep>::min());}
    inline static METRICCONSTEXPR angularspeed max()  {return angularspeed(angularspeed_values<rep>::max());}
};


typedef angularspeed<long long, std::ratio<3600, 360> > degree_second;  // Not fan.
typedef angularspeed<long long, std::ratio<3600,   1> > turn_second;
typedef angularspeed<long long, std::ratio<  60,   1> > turn_minute;
typedef angularspeed<long long                        > turn_hour;


namespace literals {

constexpr degree_second operator ""_degsec(unsigned long long v) { return degree_second(v); }
constexpr   turn_second operator ""_rps(unsigned long long v)    { return turn_second(v); }
constexpr   turn_minute operator ""_rpm(unsigned long long v)    { return turn_minute(v); }
constexpr     turn_hour operator ""_rph(unsigned long long v)    { return turn_hour(v); }

}



// AngularSpeed ==

template <class _LhsAngularSpeed, class _RhsAngularSpeed>
struct __angularspeed_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsAngularSpeed& __lhs, const _RhsAngularSpeed& __rhs) const
        {
            typedef typename std::common_type<_LhsAngularSpeed, _RhsAngularSpeed>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsAngularSpeed>
struct __angularspeed_eq<_LhsAngularSpeed, _LhsAngularSpeed>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsAngularSpeed& __lhs, const _LhsAngularSpeed& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    return __angularspeed_eq<angularspeed<_Rep1, _Period1>, angularspeed<_Rep2, _Period2> >()(__lhs, __rhs);
}

// AngularSpeed !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// AngularSpeed <

template <class _LhsAngularSpeed, class _RhsAngularSpeed>
struct __angularspeed_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsAngularSpeed& __lhs, const _RhsAngularSpeed& __rhs) const
        {
            typedef typename std::common_type<_LhsAngularSpeed, _RhsAngularSpeed>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsAngularSpeed>
struct __angularspeed_lt<_LhsAngularSpeed, _LhsAngularSpeed>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsAngularSpeed& __lhs, const _LhsAngularSpeed& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    return __angularspeed_lt<angularspeed<_Rep1, _Period1>, angularspeed<_Rep2, _Period2> >()(__lhs, __rhs);
}

// AngularSpeed >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// AngularSpeed <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// AngularSpeed >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// AngularSpeed +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<angularspeed<_Rep1, _Period1>, angularspeed<_Rep2, _Period2> >::type
operator+(const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<angularspeed<_Rep1, _Period1>, angularspeed<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// AngularSpeed -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<angularspeed<_Rep1, _Period1>, angularspeed<_Rep2, _Period2> >::type
operator-(const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<angularspeed<_Rep1, _Period1>, angularspeed<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// AngularSpeed *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    angularspeed<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const angularspeed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef angularspeed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    angularspeed<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const angularspeed<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// AngularSpeed /

template <class _AngularSpeed, class _Rep, bool = __is_angularspeed<_Rep>::value>
struct __angularspeed_divide_result
{
};

template <class _AngularSpeed, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _AngularSpeed::rep, _Rep2>::type>::value>
struct __angularspeed_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __angularspeed_divide_imp<angularspeed<_Rep1, _Period>, _Rep2, true>
{
    typedef angularspeed<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __angularspeed_divide_result<angularspeed<_Rep1, _Period>, _Rep2, false>
    : __angularspeed_divide_imp<angularspeed<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __angularspeed_divide_result<angularspeed<_Rep1, _Period>, _Rep2>::type
operator/(const angularspeed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef angularspeed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<angularspeed<_Rep1, _Period1>, angularspeed<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// AngularSpeed %

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __angularspeed_divide_result<angularspeed<_Rep1, _Period>, _Rep2>::type
operator%(const angularspeed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef angularspeed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<angularspeed<_Rep1, _Period1>, angularspeed<_Rep2, _Period2> >::type
operator%(const angularspeed<_Rep1, _Period1>& __lhs, const angularspeed<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<angularspeed<_Rep1, _Period1>, angularspeed<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_ANGULARSPEED_HPP


