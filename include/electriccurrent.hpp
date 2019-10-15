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

template <class _Tp> 
    struct __is_electriccurrent: std::false_type {};

template <class _Rep, class _Period>
    struct __is_electriccurrent<electriccurrent<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_electriccurrent<const electriccurrent<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_electriccurrent<volatile electriccurrent<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_electriccurrent<const volatile electriccurrent<_Rep, _Period> > : std::true_type  {};


} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::electriccurrent<_Rep1, _Period1>,
                    metric::electriccurrent<_Rep2, _Period2> >
{
            typedef metric::electriccurrent<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};

} // namespace std


namespace metric {


// electriccurrent_cast

template <class _FromElectricCurrent, class _ToElectricCurrent,
          class _Period = typename std::ratio_divide<typename _FromElectricCurrent::period, typename _ToElectricCurrent::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __electriccurrent_cast;

template <class _FromElectricCurrent, class _ToElectricCurrent, class _Period>
struct __electriccurrent_cast<_FromElectricCurrent, _ToElectricCurrent, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToElectricCurrent operator()(const _FromElectricCurrent& __fd) const
    {   
        return _ToElectricCurrent(static_cast<typename _ToElectricCurrent::rep>(__fd.count()));
    }
};

template <class _FromElectricCurrent, class _ToElectricCurrent, class _Period>
struct __electriccurrent_cast<_FromElectricCurrent, _ToElectricCurrent, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToElectricCurrent operator()(const _FromElectricCurrent& __fd) const
    {   
        typedef typename std::common_type<typename _ToElectricCurrent::rep, typename _FromElectricCurrent::rep, intmax_t>::type _Ct;
        return _ToElectricCurrent(static_cast<typename _ToElectricCurrent::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromElectricCurrent, class _ToElectricCurrent, class _Period>
struct __electriccurrent_cast<_FromElectricCurrent, _ToElectricCurrent, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToElectricCurrent operator()(const _FromElectricCurrent& __fd) const
    {   
        typedef typename std::common_type<typename _ToElectricCurrent::rep, typename _FromElectricCurrent::rep, intmax_t>::type _Ct;
        return _ToElectricCurrent(static_cast<typename _ToElectricCurrent::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromElectricCurrent, class _ToElectricCurrent, class _Period>
struct __electriccurrent_cast<_FromElectricCurrent, _ToElectricCurrent, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToElectricCurrent operator()(const _FromElectricCurrent& __fd) const
    {
        typedef typename std::common_type<typename _ToElectricCurrent::rep, typename _FromElectricCurrent::rep, intmax_t>::type _Ct;
        return _ToElectricCurrent(static_cast<typename _ToElectricCurrent::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


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
    return __electriccurrent_cast<electriccurrent<_Rep, _Period>, _ToElectricCurrent>()(__fd);
}


template <class _Rep>
struct electriccurrent_values
{
public:
    inline static METRICCONSTEXPR _Rep zero() {return _Rep(0);}
    inline static METRICCONSTEXPR _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static METRICCONSTEXPR _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class electriccurrent
{
    static_assert(!__is_electriccurrent<_Rep>::value, "A electriccurrent representation can not be a electriccurrent");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of electriccurrent must be a std::ratio");
    static_assert(_Period::num > 0, "electriccurrent period must be positive");

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

    inline static METRICCONSTEXPR electriccurrent zero() {return electriccurrent(electriccurrent_values<rep>::zero());}
    inline static METRICCONSTEXPR electriccurrent min()  {return electriccurrent(electriccurrent_values<rep>::min());}
    inline static METRICCONSTEXPR electriccurrent max()  {return electriccurrent(electriccurrent_values<rep>::max());}
};


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

// ElectricCurrent ==

template <class _LhsElectricCurrent, class _RhsElectricCurrent>
struct __electriccurrent_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsElectricCurrent& __lhs, const _RhsElectricCurrent& __rhs) const
        {
            typedef typename std::common_type<_LhsElectricCurrent, _RhsElectricCurrent>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsElectricCurrent>
struct __electriccurrent_eq<_LhsElectricCurrent, _LhsElectricCurrent>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsElectricCurrent& __lhs, const _LhsElectricCurrent& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    return __electriccurrent_eq<electriccurrent<_Rep1, _Period1>, electriccurrent<_Rep2, _Period2> >()(__lhs, __rhs);
}

// ElectricCurrent !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// ElectricCurrent <

template <class _LhsElectricCurrent, class _RhsElectricCurrent>
struct __electriccurrent_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsElectricCurrent& __lhs, const _RhsElectricCurrent& __rhs) const
        {
            typedef typename std::common_type<_LhsElectricCurrent, _RhsElectricCurrent>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsElectricCurrent>
struct __electriccurrent_lt<_LhsElectricCurrent, _LhsElectricCurrent>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsElectricCurrent& __lhs, const _LhsElectricCurrent& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    return __electriccurrent_lt<electriccurrent<_Rep1, _Period1>, electriccurrent<_Rep2, _Period2> >()(__lhs, __rhs);
}

// ElectricCurrent >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// ElectricCurrent <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// ElectricCurrent >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// ElectricCurrent +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<electriccurrent<_Rep1, _Period1>, electriccurrent<_Rep2, _Period2> >::type
operator+(const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<electriccurrent<_Rep1, _Period1>, electriccurrent<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// ElectricCurrent -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<electriccurrent<_Rep1, _Period1>, electriccurrent<_Rep2, _Period2> >::type
operator-(const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<electriccurrent<_Rep1, _Period1>, electriccurrent<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// ElectricCurrent *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    electriccurrent<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const electriccurrent<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef electriccurrent<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    electriccurrent<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const electriccurrent<_Rep2, _Period>& __d)
{
    return __d * __s;
}

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

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<electriccurrent<_Rep1, _Period1>, electriccurrent<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
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

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<electriccurrent<_Rep1, _Period1>, electriccurrent<_Rep2, _Period2> >::type
operator%(const electriccurrent<_Rep1, _Period1>& __lhs, const electriccurrent<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<electriccurrent<_Rep1, _Period1>, electriccurrent<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_ELECTRICCURRENT_HPP


