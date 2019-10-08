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

template <class _Tp> 
    struct __is_mass: std::false_type {};

template <class _Rep, class _Period>
    struct __is_mass<mass<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_mass<const mass<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_mass<volatile mass<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_mass<const volatile mass<_Rep, _Period> > : std::true_type  {};


} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::mass<_Rep1, _Period1>,
                    metric::mass<_Rep2, _Period2> >
{
            typedef metric::mass<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};

} // namespace std


namespace metric {


// mass_cast

template <class _FromMass, class _ToMass,
          class _Period = typename std::ratio_divide<typename _FromMass::period, typename _ToMass::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __mass_cast;

template <class _FromMass, class _ToMass, class _Period>
struct __mass_cast<_FromMass, _ToMass, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToMass operator()(const _FromMass& __fd) const
    {   
        return _ToMass(static_cast<typename _ToMass::rep>(__fd.count()));
    }
};

template <class _FromMass, class _ToMass, class _Period>
struct __mass_cast<_FromMass, _ToMass, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToMass operator()(const _FromMass& __fd) const
    {   
        typedef typename std::common_type<typename _ToMass::rep, typename _FromMass::rep, intmax_t>::type _Ct;
        return _ToMass(static_cast<typename _ToMass::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromMass, class _ToMass, class _Period>
struct __mass_cast<_FromMass, _ToMass, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToMass operator()(const _FromMass& __fd) const
    {   
        typedef typename std::common_type<typename _ToMass::rep, typename _FromMass::rep, intmax_t>::type _Ct;
        return _ToMass(static_cast<typename _ToMass::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromMass, class _ToMass, class _Period>
struct __mass_cast<_FromMass, _ToMass, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToMass operator()(const _FromMass& __fd) const
    {
        typedef typename std::common_type<typename _ToMass::rep, typename _FromMass::rep, intmax_t>::type _Ct;
        return _ToMass(static_cast<typename _ToMass::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


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
    return __mass_cast<mass<_Rep, _Period>, _ToMass>()(__fd);
}


template <class _Rep>
struct mass_values
{
public:
    inline static METRICCONSTEXPR _Rep zero() {return _Rep(0);}
    inline static METRICCONSTEXPR _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static METRICCONSTEXPR _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class mass
{
    static_assert(!__is_mass<_Rep>::value, "A mass representation can not be a mass");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of mass must be a std::ratio");
    static_assert(_Period::num > 0, "mass period must be positive");

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

    inline static METRICCONSTEXPR mass zero() {return mass(mass_values<rep>::zero());}
    inline static METRICCONSTEXPR mass min()  {return mass(mass_values<rep>::min());}
    inline static METRICCONSTEXPR mass max()  {return mass(mass_values<rep>::max());}
};

typedef mass<long long, std::nano > nanogram;
typedef mass<long long, std::micro> microgram;
typedef mass<long long, std::milli> milligram;
typedef mass<long long            > gram;
typedef mass<     long, std::kilo > kilogram;
typedef mass<     long, std::mega > ton;


// Mass ==

template <class _LhsMass, class _RhsMass>
struct __mass_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsMass& __lhs, const _RhsMass& __rhs) const
        {
            typedef typename std::common_type<_LhsMass, _RhsMass>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsMass>
struct __mass_eq<_LhsMass, _LhsMass>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsMass& __lhs, const _LhsMass& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    return __mass_eq<mass<_Rep1, _Period1>, mass<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Mass !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Mass <

template <class _LhsMass, class _RhsMass>
struct __mass_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsMass& __lhs, const _RhsMass& __rhs) const
        {
            typedef typename std::common_type<_LhsMass, _RhsMass>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsMass>
struct __mass_lt<_LhsMass, _LhsMass>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsMass& __lhs, const _LhsMass& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    return __mass_lt<mass<_Rep1, _Period1>, mass<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Mass >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Mass <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Mass >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Mass +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<mass<_Rep1, _Period1>, mass<_Rep2, _Period2> >::type
operator+(const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<mass<_Rep1, _Period1>, mass<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Mass -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<mass<_Rep1, _Period1>, mass<_Rep2, _Period2> >::type
operator-(const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<mass<_Rep1, _Period1>, mass<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Mass *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    mass<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const mass<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef mass<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    mass<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const mass<_Rep2, _Period>& __d)
{
    return __d * __s;
}

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

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __mass_divide_result<mass<_Rep1, _Period>, _Rep2>::type
operator/(const mass<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef mass<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<mass<_Rep1, _Period1>, mass<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
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

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<mass<_Rep1, _Period1>, mass<_Rep2, _Period2> >::type
operator%(const mass<_Rep1, _Period1>& __lhs, const mass<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<mass<_Rep1, _Period1>, mass<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_MASS_HPP


