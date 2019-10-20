// -*- C++ -*-
//
//===---------------------------- frequency ------------------------------------===//
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

#ifndef METRICS_FREQUENCY_HPP
#define METRICS_FREQUENCY_HPP

#include "metric_config.hpp"

namespace metric {

template <class _Rep, class _Period = std::ratio<1> > class frequency;

template <typename A> struct __is_frequency: __is_specialization<A, frequency> {};

} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::frequency<_Rep1, _Period1>,
                    metric::frequency<_Rep2, _Period2> >
{
            typedef metric::frequency<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};

} // namespace std


namespace metric {


// frequency_cast

template <class _FromFrequency, class _ToFrequency,
          class _Period = typename std::ratio_divide<typename _FromFrequency::period, typename _ToFrequency::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __frequency_cast;

template <class _FromFrequency, class _ToFrequency, class _Period>
struct __frequency_cast<_FromFrequency, _ToFrequency, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToFrequency operator()(const _FromFrequency& __fd) const
    {   
        return _ToFrequency(static_cast<typename _ToFrequency::rep>(__fd.count()));
    }
};

template <class _FromFrequency, class _ToFrequency, class _Period>
struct __frequency_cast<_FromFrequency, _ToFrequency, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToFrequency operator()(const _FromFrequency& __fd) const
    {   
        typedef typename std::common_type<typename _ToFrequency::rep, typename _FromFrequency::rep, intmax_t>::type _Ct;
        return _ToFrequency(static_cast<typename _ToFrequency::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromFrequency, class _ToFrequency, class _Period>
struct __frequency_cast<_FromFrequency, _ToFrequency, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToFrequency operator()(const _FromFrequency& __fd) const
    {   
        typedef typename std::common_type<typename _ToFrequency::rep, typename _FromFrequency::rep, intmax_t>::type _Ct;
        return _ToFrequency(static_cast<typename _ToFrequency::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromFrequency, class _ToFrequency, class _Period>
struct __frequency_cast<_FromFrequency, _ToFrequency, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToFrequency operator()(const _FromFrequency& __fd) const
    {
        typedef typename std::common_type<typename _ToFrequency::rep, typename _FromFrequency::rep, intmax_t>::type _Ct;
        return _ToFrequency(static_cast<typename _ToFrequency::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToFrequency, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_frequency<_ToFrequency>::value,
    _ToFrequency
>::type
frequency_cast(const frequency<_Rep, _Period>& __fd)
{
    return __frequency_cast<frequency<_Rep, _Period>, _ToFrequency>()(__fd);
}


template <class _Rep>
struct frequency_values
{
public:
    inline static METRICCONSTEXPR _Rep zero() {return _Rep(0);}
    inline static METRICCONSTEXPR _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static METRICCONSTEXPR _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class frequency
{
    static_assert(!__is_frequency<_Rep>::value, "A frequency representation can not be a frequency");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of frequency must be a std::ratio");
    static_assert(_Period::num > 0, "frequency period must be positive");

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
    frequency() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit frequency(const _Rep2& __r,
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
        frequency(const frequency<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::frequency_cast<frequency>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR frequency  operator+() const {return *this;}
    inline METRICCONSTEXPR frequency  operator-() const {return frequency(-__rep_);}
    inline const frequency& operator++()      {++__rep_; return *this;}
    inline const frequency  operator++(int)   {return frequency(__rep_++);}
    inline const frequency& operator--()      {--__rep_; return *this;}
    inline const frequency  operator--(int)   {return frequency(__rep_--);}

    inline const frequency& operator+=(const frequency& __d) {__rep_ += __d.count(); return *this;}
    inline const frequency& operator-=(const frequency& __d) {__rep_ -= __d.count(); return *this;}

    inline const frequency& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const frequency& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const frequency& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const frequency& operator%=(const frequency& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR frequency zero() {return frequency(frequency_values<rep>::zero());}
    inline static METRICCONSTEXPR frequency min()  {return frequency(frequency_values<rep>::min());}
    inline static METRICCONSTEXPR frequency max()  {return frequency(frequency_values<rep>::max());}
};


typedef frequency<long long, std::milli> millihertz;
typedef frequency<long long            > hertz;
typedef frequency<long long, std::kilo > kilohertz;
typedef frequency<long     , std::mega > megahertz;
typedef frequency<long     , std::giga > gigahertz;

namespace literals {

constexpr millihertz operator ""_mHz(unsigned long long v)  { return millihertz(v); }
constexpr      hertz operator ""_Hz(unsigned long long v)   { return hertz(v);      }
constexpr  kilohertz operator ""_kHz(unsigned long long v)  { return kilohertz(v);  }
constexpr  megahertz operator ""_MHz(unsigned long long v)  { return megahertz(v);  }
constexpr  gigahertz operator ""_GHz(unsigned long long v)  { return gigahertz(v);  }

} // namespace literals

// Frequency ==

template <class _LhsFrequency, class _RhsFrequency>
struct __frequency_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsFrequency& __lhs, const _RhsFrequency& __rhs) const
        {
            typedef typename std::common_type<_LhsFrequency, _RhsFrequency>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsFrequency>
struct __frequency_eq<_LhsFrequency, _LhsFrequency>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsFrequency& __lhs, const _LhsFrequency& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    return __frequency_eq<frequency<_Rep1, _Period1>, frequency<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Frequency !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Frequency <

template <class _LhsFrequency, class _RhsFrequency>
struct __frequency_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsFrequency& __lhs, const _RhsFrequency& __rhs) const
        {
            typedef typename std::common_type<_LhsFrequency, _RhsFrequency>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsFrequency>
struct __frequency_lt<_LhsFrequency, _LhsFrequency>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsFrequency& __lhs, const _LhsFrequency& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    return __frequency_lt<frequency<_Rep1, _Period1>, frequency<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Frequency >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Frequency <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Frequency >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Frequency +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<frequency<_Rep1, _Period1>, frequency<_Rep2, _Period2> >::type
operator+(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<frequency<_Rep1, _Period1>, frequency<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Frequency -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<frequency<_Rep1, _Period1>, frequency<_Rep2, _Period2> >::type
operator-(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<frequency<_Rep1, _Period1>, frequency<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Frequency *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    frequency<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const frequency<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef frequency<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    frequency<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const frequency<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// Frequency /

template <class _Frequency, class _Rep, bool = __is_frequency<_Rep>::value>
struct __frequency_divide_result
{
};

template <class _Frequency, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Frequency::rep, _Rep2>::type>::value>
struct __frequency_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __frequency_divide_imp<frequency<_Rep1, _Period>, _Rep2, true>
{
    typedef frequency<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __frequency_divide_result<frequency<_Rep1, _Period>, _Rep2, false>
    : __frequency_divide_imp<frequency<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __frequency_divide_result<frequency<_Rep1, _Period>, _Rep2>::type
operator/(const frequency<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef frequency<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<frequency<_Rep1, _Period1>, frequency<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// Frequency %

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __frequency_divide_result<frequency<_Rep1, _Period>, _Rep2>::type
operator%(const frequency<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef frequency<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<frequency<_Rep1, _Period1>, frequency<_Rep2, _Period2> >::type
operator%(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<frequency<_Rep1, _Period1>, frequency<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_FREQUENCY_HPP


