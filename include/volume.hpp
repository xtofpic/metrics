// -*- C++ -*-
//
//===---------------------------- volume ------------------------------------===//
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

#ifndef METRICS_VOLUME_HPP
#define METRICS_VOLUME_HPP

#include "metric_config.hpp"

namespace metric {


template <class _Rep, class _Period = std::ratio<1> > class volume;

template <class _Tp> 
    struct __is_volume: std::false_type {};

template <class _Rep, class _Period>
    struct __is_volume<volume<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_volume<const volume<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_volume<volatile volume<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_volume<const volatile volume<_Rep, _Period> > : std::true_type  {};


} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::volume<_Rep1, _Period1>,
                    metric::volume<_Rep2, _Period2> >
{
            typedef metric::volume<typename common_type<_Rep1, _Rep2>::type,
                ratio< GCD<_Period1::num, _Period2::num>::value,
                       LCM<_Period1::den, _Period2::den>::value> > type;
};

} // namespace std


namespace metric {


// volume_cast

template <class _FromVolume, class _ToVolume,
          class _Period = typename std::ratio_divide<typename _FromVolume::period, typename _ToVolume::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __volume_cast;

template <class _FromVolume, class _ToVolume, class _Period>
struct __volume_cast<_FromVolume, _ToVolume, _Period, true, true>
{   
    inline METRICCONSTEXPR
    _ToVolume operator()(const _FromVolume& __fd) const
    {   
        return _ToVolume(static_cast<typename _ToVolume::rep>(__fd.count()));
    }
};

template <class _FromVolume, class _ToVolume, class _Period>
struct __volume_cast<_FromVolume, _ToVolume, _Period, true, false>
{   
    inline METRICCONSTEXPR
    _ToVolume operator()(const _FromVolume& __fd) const
    {   
        typedef typename std::common_type<typename _ToVolume::rep, typename _FromVolume::rep, intmax_t>::type _Ct;
        return _ToVolume(static_cast<typename _ToVolume::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromVolume, class _ToVolume, class _Period>
struct __volume_cast<_FromVolume, _ToVolume, _Period, false, true>
{   
    inline METRICCONSTEXPR
    _ToVolume operator()(const _FromVolume& __fd) const
    {   
        typedef typename std::common_type<typename _ToVolume::rep, typename _FromVolume::rep, intmax_t>::type _Ct;
        return _ToVolume(static_cast<typename _ToVolume::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromVolume, class _ToVolume, class _Period>
struct __volume_cast<_FromVolume, _ToVolume, _Period, false, false>
{
    inline METRICCONSTEXPR
    _ToVolume operator()(const _FromVolume& __fd) const
    {
        typedef typename std::common_type<typename _ToVolume::rep, typename _FromVolume::rep, intmax_t>::type _Ct;
        return _ToVolume(static_cast<typename _ToVolume::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToVolume, class _Rep, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_volume<_ToVolume>::value,
    _ToVolume
>::type
volume_cast(const volume<_Rep, _Period>& __fd)
{
    return __volume_cast<volume<_Rep, _Period>, _ToVolume>()(__fd);
}


template <class _Rep>
struct volume_values
{
public:
    inline static METRICCONSTEXPR _Rep zero() {return _Rep(0);}
    inline static METRICCONSTEXPR _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static METRICCONSTEXPR _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class volume
{
    static_assert(!__is_volume<_Rep>::value, "A volume representation can not be a volume");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of volume must be a std::ratio");
    static_assert(_Period::num > 0, "volume period must be positive");

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
    volume() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit volume(const _Rep2& __r,
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
        volume(const volume<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::volume_cast<volume>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR volume  operator+() const {return *this;}
    inline METRICCONSTEXPR volume  operator-() const {return volume(-__rep_);}
    inline const volume& operator++()      {++__rep_; return *this;}
    inline const volume  operator++(int)   {return volume(__rep_++);}
    inline const volume& operator--()      {--__rep_; return *this;}
    inline const volume  operator--(int)   {return volume(__rep_--);}

    inline const volume& operator+=(const volume& __d) {__rep_ += __d.count(); return *this;}
    inline const volume& operator-=(const volume& __d) {__rep_ -= __d.count(); return *this;}

    inline const volume& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const volume& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const volume& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const volume& operator%=(const volume& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR volume zero() {return volume(volume_values<rep>::zero());}
    inline static METRICCONSTEXPR volume min()  {return volume(volume_values<rep>::min());}
    inline static METRICCONSTEXPR volume max()  {return volume(volume_values<rep>::max());}
};


typedef volume<long long, std::nano > nanolitre;
typedef volume<long long, std::micro> microlitre;
typedef volume<long long, std::milli> millilitre;
typedef volume<long long            > litre;
typedef volume<long long, std::kilo > kilolitre;
typedef volume<long long, std::mega > megalitre;


namespace literals {

constexpr   nanolitre operator ""_nl(unsigned long long v) { return   nanolitre(v); }
constexpr  microlitre operator ""_ul(unsigned long long v) { return  microlitre(v); }
constexpr  millilitre operator ""_ml(unsigned long long v) { return  millilitre(v); }
constexpr       litre operator ""_l(unsigned long long v)  { return       litre(v); }
constexpr   kilolitre operator ""_kl(unsigned long long v) { return   kilolitre(v); }
constexpr   megalitre operator ""_Ml(unsigned long long v) { return   megalitre(v); }

} // namespace literals


// Volume ==

template <class _LhsVolume, class _RhsVolume>
struct __volume_eq
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsVolume& __lhs, const _RhsVolume& __rhs) const
        {
            typedef typename std::common_type<_LhsVolume, _RhsVolume>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsVolume>
struct __volume_eq<_LhsVolume, _LhsVolume>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsVolume& __lhs, const _LhsVolume& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator==(const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    return __volume_eq<volume<_Rep1, _Period1>, volume<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Volume !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator!=(const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Volume <

template <class _LhsVolume, class _RhsVolume>
struct __volume_lt
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsVolume& __lhs, const _RhsVolume& __rhs) const
        {
            typedef typename std::common_type<_LhsVolume, _RhsVolume>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsVolume>
struct __volume_lt<_LhsVolume, _LhsVolume>
{
    inline METRICCONSTEXPR
    bool operator()(const _LhsVolume& __lhs, const _LhsVolume& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator< (const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    return __volume_lt<volume<_Rep1, _Period1>, volume<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Volume >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator> (const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Volume <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator<=(const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Volume >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
bool
operator>=(const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Volume +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<volume<_Rep1, _Period1>, volume<_Rep2, _Period2> >::type
operator+(const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<volume<_Rep1, _Period1>, volume<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Volume -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<volume<_Rep1, _Period1>, volume<_Rep2, _Period2> >::type
operator-(const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<volume<_Rep1, _Period1>, volume<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Volume *

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    volume<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const volume<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef volume<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    volume<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const volume<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// Volume /

template <class _Volume, class _Rep, bool = __is_volume<_Rep>::value>
struct __volume_divide_result
{
};

template <class _Volume, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Volume::rep, _Rep2>::type>::value>
struct __volume_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __volume_divide_imp<volume<_Rep1, _Period>, _Rep2, true>
{
    typedef volume<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __volume_divide_result<volume<_Rep1, _Period>, _Rep2, false>
    : __volume_divide_imp<volume<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __volume_divide_result<volume<_Rep1, _Period>, _Rep2>::type
operator/(const volume<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef volume<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<_Rep1, _Rep2>::type
operator/(const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<volume<_Rep1, _Period1>, volume<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// Volume %

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __volume_divide_result<volume<_Rep1, _Period>, _Rep2>::type
operator%(const volume<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef volume<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
METRICCONSTEXPR
typename std::common_type<volume<_Rep1, _Period1>, volume<_Rep2, _Period2> >::type
operator%(const volume<_Rep1, _Period1>& __lhs, const volume<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<volume<_Rep1, _Period1>, volume<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_VOLUME_HPP


